# 为 `run_cpp_tests.py` 增加默认合并 `reference_modules` 的设计

## 背景

当前 `run_cpp_tests.py` 在处理开启了 `-fdump-vtable-layouts` 的 `cpp_tests` 项时，会对 `reference_modules` 中的每个模块分别执行一次 `compare_compiler_vtable_with_yaml()`。

这会带来三个问题：

- 同一个测试项会重复输出多段高度相似的 compare 日志。
- `VTable compares run` 统计的是“模块展开后的 compare 次数”，而不是“测试项的实际 compare 次数”。
- 对于像 `INetworkMessages_MSVC` 这种引用 `networksystem`、`engine`、`server`、`client` 多个模块的场景，当前实现只能逐模块独立比较，不能表达“多模块共同补全同一张虚函数表 reference”的语义。

以 `config.yaml` 中的 `INetworkMessages_MSVC` 为例，当前运行：

```bash
uv run run_cpp_tests.py -gamever 14141b -debug
```

会对四个目录分别 compare 一次：

- `bin/{gamever}/networksystem/CNetworkMessages_*.{platform}.yaml`
- `bin/{gamever}/engine/CNetworkMessages_*.{platform}.yaml`
- `bin/{gamever}/server/CNetworkMessages_*.{platform}.yaml`
- `bin/{gamever}/client/CNetworkMessages_*.{platform}.yaml`

其中 `networksystem`、`engine`、`server` 可能只分别提供部分 `vfunc_index`，但合起来才构成完整的 reference 视图。当前逐模块 compare 既放大了日志，也不能直接表达“补全合并，但冲突显式报错”的需求。

## 目标

- 将多模块 `reference_modules` 的默认行为改为“先合并 reference，再执行一次 compare”。
- 默认启用合并模式，不要求现有配置额外打开。
- 保留显式回退开关，允许单个测试项恢复旧式逐模块 compare。
- 支持多个模块对同一类的 reference 数据做补全合并。
- 若多个模块对同一字段给出冲突值，必须显式报差异，不能静默覆盖。
- compare 报告中保留来源模块与来源 YAML 文件信息，便于定位每个索引来自哪里。
- 让 `VTable compares run` 更贴近“测试项 compare 次数”的直觉语义。

## 非目标

- 不修改 `reference_modules` 的配置结构与含义。
- 不移除旧式逐模块 compare 能力；该能力保留为兼容回退路径。
- 不改变 `alias_symbols` 的现有语义。
- 不修改 clang 编译命令、目标平台映射或 vtable 解析算法。
- 不为没有 `reference_modules` 的测试项引入额外复杂配置。

## 用户接口

### `cpp_tests` 新增可选字段

在 `cpp_tests[]` 下新增：

```yaml
merge_reference_modules: false
```

字段语义如下：

- 未配置时，默认视为 `true`
- `true`：先合并所有 `reference_modules` 的 YAML，再只执行 1 次 compare
- `false`：恢复当前旧行为，对 `reference_modules` 逐模块分别 compare

### 示例

默认启用合并，无需新增配置：

```yaml
- name: INetworkMessages_MSVC
  symbol: INetworkMessages
  alias_symbols:
    - CNetworkMessages
  cpp: cpp_tests/inetworkmessages.cpp
  target: x86_64-pc-windows-msvc
  additional_compiler_options:
    - Xclang
    - fdump-vtable-layouts
  reference_modules:
    - networksystem
    - engine
    - server
    - client
```

显式回退到旧行为：

```yaml
- name: INetworkMessages_MSVC
  symbol: INetworkMessages
  alias_symbols:
    - CNetworkMessages
  cpp: cpp_tests/inetworkmessages.cpp
  target: x86_64-pc-windows-msvc
  merge_reference_modules: false
  additional_compiler_options:
    - Xclang
    - fdump-vtable-layouts
  reference_modules:
    - networksystem
    - engine
    - server
    - client
```

## 总体方案

采用“默认合并 reference + 显式回退旧行为”的最短路径方案。

### 入口层：`run_cpp_tests.py`

`run_one_test()` 负责根据 `merge_reference_modules` 的值选择 compare 模式：

- 若 `reference_modules` 为空，保持当前单次 compare 行为不变
- 若 `reference_modules` 非空且 `merge_reference_modules` 为 `true`，调用一次 compare，并将全部模块作为待合并输入传入
- 若 `reference_modules` 非空且 `merge_reference_modules` 为 `false`，保留当前逐模块循环 compare 的兼容逻辑

这样可以把新语义集中在“reference 如何加载”，而不改动 compile 流程与最终汇总流程的总体结构。

### 底层实现：`cpp_tests_util.py`

对比入口仍保留 `compare_compiler_vtable_with_yaml()`，但扩展其能力：

- 新增参数 `merge_reference_modules: bool = True`
- `merge_reference_modules=True` 时，调用新的合并式加载逻辑
- `merge_reference_modules=False` 时，调用当前按优先级首个命中即返回的旧逻辑

推荐新增辅助函数：

```python
load_merged_reference_vtable_data(
    bindir,
    gamever,
    class_name,
    platform,
    reference_modules,
    alias_class_names=(),
)
```

职责为：

- 扫描所有候选模块目录
- 读取所有匹配 `class_name` 或 alias 的 YAML
- 按顺序聚合 `vtable_size`、`vtable_numvfunc` 与 `functions_by_index`
- 在聚合过程中记录来源与冲突

## 合并算法

### 遍历顺序

遍历顺序固定为：

1. `reference_modules` 中定义的模块顺序
2. 单个模块目录内按文件名排序后的 YAML 顺序
3. 对每个模块，先尝试主类名，再按 `alias_class_names` 顺序尝试别名

该顺序只用于：

- 补全时的“先到先占位”
- 冲突报告中的来源展示顺序

它不再意味着需要输出多份 compare 报告。

### 顶层字段合并

#### `vtable_size`

- 第一个有效值先占位
- 后续相同值视为一致
- 后续不同值记为 `reference_conflict_vtable_size`

#### `vtable_numvfunc`

- 规则与 `vtable_size` 相同
- 后续不同值记为 `reference_conflict_vtable_numvfunc`

### `functions_by_index` 合并

对每个 `vfunc_index`：

- 第一个命中的条目先成为该 index 的选中条目
- 后续条目若仅补充来源信息，不改变已选中值
- 若后续条目的 `member_name` 与已选中值一致，视为一致来源补充
- 若后续条目缺失有效 `member_name`，允许保留现有值并只追加来源
- 若后续条目给出不同的有效 `member_name`，记为 `reference_conflict_vfunc_name`

冲突策略采用“允许补全，不允许静默冲突”的规则：

- 允许补全：只要后来的 YAML 没有推翻已选中的有效值，就可以追加来源信息
- 明确冲突：一旦不同模块对同一 index 给出不同有效名值，必须在 compare 报告中体现

## 来源信息设计

### merged reference 顶层结构

合并函数返回的数据建议包含：

```python
{
    "mode": "merged",
    "modules": [...],
    "files": [...],
    "vtable_size": ...,
    "vtable_numvfunc": ...,
    "functions_by_index": {...},
    "conflicts": [...],
}
```

建议字段说明：

- `mode`
  - 固定为 `merged`
- `modules`
  - 实际命中过 YAML 的模块名列表
- `files`
  - 实际读取并参与合并的 YAML 文件列表
- `conflicts`
  - reference 合并阶段检测到的冲突项

### `functions_by_index[index]` 条目结构

建议扩展为：

```python
{
    "func_name": "...",
    "member_name": "...",
    "path": "...",
    "module": "...",
    "sources": [
        {
            "module": "...",
            "path": "...",
            "func_name": "...",
            "member_name": "...",
        }
    ],
}
```

语义如下：

- `func_name` / `member_name`
  - 当前被 compare 采用的主值
- `path` / `module`
  - 当前主值的主来源
- `sources`
  - 该 index 所有命中来源，用于调试与报告展示

## compare 报告结构调整

### `compare_compiler_vtable_with_yaml()` 返回值

建议为 report 增加以下字段：

- `reference_mode`
  - `merged` 或 `single`
- `reference_modules_merged`
  - merged 模式下实际命中的模块
- `reference_files_merged`
  - merged 模式下实际命中的文件
- `reference_conflicts`
  - merged 阶段记录的 reference 冲突

现有字段保持兼容：

- `reference_found`
- `reference_module`
- `reference_functions_count`
- `differences`
- `notes`

其中：

- single 模式仍使用 `reference_module`
- merged 模式可保留 `reference_module=None`，改由新的 merged 字段提供信息

### 冲突进入差异模型

reference 合并阶段发现的冲突必须进入 `differences`，从而影响最终统计与 `-fixheader` 输入。

建议新增差异类型：

- `reference_conflict_vtable_size`
- `reference_conflict_vtable_numvfunc`
- `reference_conflict_vfunc_name`

示例消息：

- `Reference vtable_size conflict: networksystem=0x130, engine=0x128.`
- `Reference vtable_numvfunc conflict: networksystem=38, server=37.`
- `Reference index 17 conflict: networksystem expects 'SerializeInternal', engine expects 'Serialize'.`

## 日志格式调整

### merged 模式

单次 compare 输出建议包含：

- `Class 'INetworkMessages' compare target platform: windows`
- `Compiler vtable entries: parsed=38, declared=38`
- `Reference mode: merged`
- `Reference modules: networksystem, engine, server`
- `Reference files merged: 21`
- `Reference functions: 21`

若存在 alias 命中，继续沿用当前 note：

- `Reference YAML matched via alias symbol 'CNetworkMessages' (primary symbol 'INetworkMessages' not found).`

若存在冲突，增加：

- `Reference conflicts found: N`
- `- ...`

若不存在差异，继续输出：

- `No differences detected for vtable_size/vtable_numvfunc/vfunc_index mapping.`

### single 模式

保留当前输出风格：

- `Reference module: networksystem, reference functions: 16`

避免影响你在回退模式下对旧日志的预期。

## 汇总计数语义

### `VTable compares run`

该计数继续表示“打印出的 compare report 条数”。

因此：

- 在 merged 模式下，一个测试项即使引用多个模块，也只记 1 次
- 在 single 回退模式下，仍按逐模块 compare 数量累加

这会让默认行为下的统计更接近用户直觉，也更符合“测试项只比较了一次”的事实。

### `VTable compares with differences`

该计数继续表示“存在差异的 compare report 条数”。

reference 冲突属于差异，因此应计入。

## 兼容策略

### 默认升级

所有现有配置，只要包含 `reference_modules`，在未显式关闭时都自动使用 merged 模式。

这意味着像 `INetworkMessages_MSVC` 这样的现有测试项，不需要修改配置即可获得：

- 更少的 compare 次数
- 更精炼的日志
- 更完整的 reference 视图

### 显式回退

当需要排查历史行为、做 A/B 对照或怀疑合并逻辑时，可为单个测试项设置：

```yaml
merge_reference_modules: false
```

以恢复旧式逐模块 compare。

## 对 `-fixheader` 的影响

`-fixheader` 的触发条件保持不变：

- 只要某份 compare report 的 `differences` 非空，就会进入 header fix 流程

变化仅在于：

- merged 模式下，传给 agent 的差异文本来自单份聚合报告
- 模块间 reference 冲突也会进入差异文本

这样可以避免以前那种“模块 A 无差异、模块 B 无差异、但整体 reference 其实互相打架”的情况被隐藏掉。

## 验证方案

本变更优先采用 Level 0 定向验证。

### 场景 1：单模块 reference

目标：

- merged 模式与当前行为等价
- compare 次数不变
- 报告不出现多余 merged 冗余信息

### 场景 2：多模块互补

目标：

- 一个测试项只输出 1 份 compare 报告
- `Reference modules` 列出全部命中模块
- `reference functions` 大于任一单模块结果

### 场景 3：多模块冲突

目标：

- 报告中出现 `Reference conflicts found`
- `differences` 中包含对应 `reference_conflict_*`
- 差异消息可定位到模块与文件来源

### 场景 4：显式关闭合并

目标：

- 恢复旧式逐模块 compare
- compare 次数再次按模块数累加
- 日志格式与当前实现保持一致

### 推荐命令

主验证命令：

```bash
uv run run_cpp_tests.py -gamever 14141b -debug
```

回退模式对照验证：

- 在一个测试项上临时设置 `merge_reference_modules: false`
- 再运行同一条命令，确认 compare 次数与日志恢复旧行为

## 风险与权衡

### 风险 1：默认行为变化

由于本设计选择默认启用 merged 模式，已有测试日志与 compare 次数会变化。

应对方式：

- 保留显式回退开关
- 在 spec、plan 与最终变更说明中明确这一点

### 风险 2：冲突定义过严或过松

如果 `member_name` 归一化规则不够稳定，可能导致本应等价的名称被识别为冲突，或本应冲突的条目被误判为一致。

应对方式：

- 复用现有 `_normalize_reference_member_name()` 的结果作为比较基准
- 仅在双方都有有效 `member_name` 且不相等时判定冲突

### 风险 3：报告结构扩展影响旧 formatter 假设

应对方式：

- 对 single 模式保留现有字段与输出格式
- 只在 merged 模式下新增展示行

## 实施建议

推荐按以下顺序实施：

1. 在 `cpp_tests_util.py` 中新增 merged reference 加载函数
2. 扩展 `compare_compiler_vtable_with_yaml()` 的 report 结构与差异模型
3. 更新 `format_vtable_compare_report()` 以展示 merged 信息与冲突
4. 在 `run_cpp_tests.py` 中引入 `merge_reference_modules` 默认逻辑
5. 用 `INetworkMessages_MSVC` 作为主验证样例
6. 补一个显式关闭合并的配置样例做回退验证

## 验收标准

- `INetworkMessages_MSVC` 在默认配置下只输出 1 份 compare 报告
- 报告能展示 merged 模块列表与文件来源信息
- 多模块补全后，compare 使用的是合并后的完整 reference
- 多模块冲突会转化为显式差异
- `merge_reference_modules: false` 时可恢复旧式逐模块 compare

