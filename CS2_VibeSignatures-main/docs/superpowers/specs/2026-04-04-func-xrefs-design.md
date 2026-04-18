# 为 `preprocess_common_skill` 增加 `func_xrefs` 统一函数交叉引用定位设计

## 背景

当前 `ida_analyze_util.py` 中的 `preprocess_common_skill()` 已支持 `func_xref_strings`，用于在 `func_sig` 失效时，通过“目标函数同时引用若干字符串”这一特征反向定位函数。

这一能力已经能覆盖一部分场景，但表达能力仍然不足：

- 它只能利用字符串引用，不能利用“目标函数同时引用另一个已知函数”这一更强约束。
- 调用接口名称是 `func_xref_strings`，语义被绑定在字符串上，不适合作为统一的交叉引用匹配入口继续扩展。
- 当目标函数只靠字符串仍有多个候选时，缺少一个与现有 YAML 产物联动的、稳定的二次约束来源。

本次设计将 `func_xref_strings` 升级为统一的 `func_xrefs` 接口，使预处理既能依赖字符串交叉引用，也能依赖函数交叉引用，并通过二者联合求交得到唯一目标函数。

## 目标

- 为 `preprocess_common_skill()` 新增统一参数 `func_xrefs`。
- 让 `func_xrefs` 完整取代 `func_xref_strings`，代码库中不再保留旧参数与旧标识。
- 支持三种使用形态：
  - 仅字符串引用约束
  - 仅函数引用约束
  - 字符串引用与函数引用联合约束
- 约束中的依赖函数地址只从当前版本 YAML 读取，不读取 `old_yaml`，也不依赖 IDA 中已有名字。
- 继续支持“仅出现在 xref 列表、不出现在 `func_names` 中”的函数目标走统一 func 处理流水线。
- 为新能力补充 Serena memory，记录契约、依赖条件与迁移后的使用方式。

## 非目标

- 不保留 `func_xref_strings` 的兼容层、别名或双分支逻辑。
- 不从 `old_yaml_map` 中读取任何 `xref_funcs_list` 依赖函数地址。
- 不依赖 IDA 当前数据库里的函数名来解析 `xref_funcs_list`。
- 不引入模糊匹配、正则匹配或“交集不唯一时自动选择最优候选”的启发式规则。
- 不改变 `preprocess_func_sig_via_mcp()` 作为首选快路径的既有策略。

## 用户接口

### 新参数

`preprocess_common_skill()` 删除：

- `func_xref_strings=None`

新增：

- `func_xrefs=None`

### `func_xrefs` 数据结构

`func_xrefs` 为列表，每个元素是固定三元组：

```python
(func_name, xref_strings_list, xref_funcs_list)
```

含义如下：

- `func_name`
  - 目标函数名，也是输出 YAML 文件名 stem 和 IDA 重命名目标名。
- `xref_strings_list`
  - 目标函数必须引用的字符串列表，可为空列表。
- `xref_funcs_list`
  - 目标函数必须引用的已知函数列表，可为空列表。

约束规则：

- `xref_strings_list` 与 `xref_funcs_list` 不能同时为空。
- `func_name` 在同一次 `preprocess_common_skill()` 调用中必须唯一。
- 任意依赖函数名都必须能在当前版本 YAML 中解析到合法 `func_va`。

### 调用示例

纯字符串约束：

```python
FUNC_XREFS = [
    (
        "CNetChan_ProcessMessages",
        [
            "NetChan %s ProcessMessages has taken more than %dms to process %d messages.",
        ],
        [],
    ),
]
```

字符串与函数联合约束：

```python
FUNC_XREFS = [
    (
        "CNetworkGameClient_ProcessPacketEntities",
        [
            "CNetworkGameClientBase::OnReceivedUncompressedPacket(), received full update",
        ],
        [
            "CNetworkGameClient_ProcessPacketEntitiesInternal",
        ],
    ),
]
```

### 非法配置

以下情况直接视为配置错误并失败：

- 某个条目不是三元组。
- 同一 `func_name` 在 `func_xrefs` 中重复出现。
- `xref_strings_list` 与 `xref_funcs_list` 同时为空。
- `xref_funcs_list` 中任一依赖函数的当前版本 YAML 缺失、不可读或没有合法 `func_va`。

## 总体方案

采用“统一接口 + 统一求交实现 + 单步破坏式迁移”方案。

### 入口层

`preprocess_common_skill()`：

- 参数列表从 `func_xref_strings` 切换为 `func_xrefs`
- 文档字符串同步改写为 `func_xrefs`
- 默认值处理、映射表构建、fallback 调用点、xref-only 目标合并逻辑全部迁移到新参数

### 底层实现

现有 `preprocess_func_xref_strings_via_mcp()` 升级为统一函数，建议命名：

- `preprocess_func_xrefs_via_mcp()`

新函数职责：

1. 校验 `xref_strings_list` 与 `xref_funcs_list`
2. 从字符串引用收集候选函数集合
3. 从依赖函数引用收集候选函数集合
4. 对所有非空候选集合做交集
5. 要求最终候选恰好为 1 个函数
6. 调用既有 `preprocess_gen_func_sig_via_mcp()` 生成 `func_sig`
7. 返回与现有 `write_func_yaml()` 兼容的函数 YAML 数据

## 当前版本 YAML 依赖解析

`xref_funcs_list` 的地址解析只依赖当前版本 YAML，不依赖旧版本 YAML，也不依赖 IDA 当前名字。

### YAML 路径解析规则

依赖函数 `dep_func_name` 的 YAML 路径按现有 `inherit_vfuncs` 逻辑对齐：

```python
os.path.join(new_binary_dir, f"{dep_func_name}.{platform}.yaml")
```

说明：

- `new_binary_dir` 已由主流程传入，表示当前模块、当前版本、当前二进制对应的 YAML 输出目录。
- `platform` 使用当前正在处理的平台名。
- 这意味着 `func_xrefs` 依赖的函数必须已经在同一模块/平台的当前版本目录下落盘。
- 依赖顺序由 `config.yaml` 中的 `skills` 排序保证；当执行包含 `func_xrefs` 的脚本时，其依赖函数 YAML 应已存在。

### YAML 读取规则

- 只接受字典型 YAML 顶层。
- 只读取 `func_va` 字段。
- `func_va` 必须可按 `int(value, 0)` 语义解析。
- 读取失败、字段缺失或地址非法时，立即失败并输出明确错误。

推荐错误语义：

- `failed to read dependency func YAML`
- `missing func_va in dependency YAML`
- `invalid func_va in dependency YAML`

## 联合求交算法

### 1. 字符串候选集

沿用现有 `func_xref_strings` 的做法：

1. 对每个 `xref_string` 遍历 IDA 字符串表。
2. 找到包含该字符串的条目。
3. 对字符串地址执行 `XrefsTo(str_ea, 0)`。
4. 对每个 xref 的来源地址执行 `idaapi.get_func(xref.frm)`。
5. 收集所属函数起始地址，得到该字符串的候选集合。
6. 对所有字符串候选集合求交。

任何一个字符串候选集合为空，直接失败。

### 2. 函数候选集

新增函数交叉引用候选集合，流程如下：

1. 对每个 `dep_func_name`，从当前版本 YAML 读取 `func_va`。
2. 在 IDA 中对该 `func_va` 执行 `XrefsTo(dep_func_ea, 0)`。
3. 对每个 xref 的来源地址执行 `idaapi.get_func(xref.frm)`。
4. 仅保留能归属到函数的来源地址，收集其函数起始地址。
5. 得到“引用该依赖函数的所有函数”的候选集合。
6. 对所有依赖函数候选集合求交。

说明：

- 这里不依赖 IDA 中的函数名解析依赖函数，只依赖 YAML 中的地址。
- 这里不要求 xref 来源必须是 call 指令；只要 xref 来源地址属于函数体，就可作为候选。
- 纯数据引用不会进入结果，因为 `idaapi.get_func(xref.frm)` 返回空时会被过滤掉。

任何一个依赖函数候选集合为空，直接失败。

### 3. 总交集

最终候选集合由所有非空类别候选集合共同决定：

- 仅提供字符串约束：最终候选 = 字符串候选交集
- 仅提供函数约束：最终候选 = 函数候选交集
- 同时提供二者：最终候选 = 字符串候选交集 ∩ 函数候选交集

最终结果要求：

- 交集大小必须恰好为 1

否则直接失败并打印明确错误：

- `xref intersection yielded 0 function(s)`
- `xref intersection yielded N function(s), need exactly 1`

### 4. 成功后的 YAML 生成

定位成功后：

1. 生成或补充 `func_sig`
2. 输出 `func_name`、`func_va`、`func_rva`、`func_size`、`func_sig`
3. 继续执行 best-effort IDA rename
4. 继续沿用既有 `func_vtable_relations` 补充逻辑

因此 `func_xrefs` 只是函数定位方式的升级，不改变最终 YAML 契约。

## 与 `preprocess_common_skill()` 的集成

### 保持首选快路径

对每个目标函数的处理顺序保持为：

1. 先尝试 `preprocess_func_sig_via_mcp()`
2. 若失败且该函数在 `func_xrefs` 中有条目，再尝试 `preprocess_func_xrefs_via_mcp()`
3. 若仍失败，则整个 `preprocess_skill` 返回失败

这样可以继续复用旧有 `func_sig` 的快速定位能力，只有在签名失效时才进入更重的 xref 求交流程。

### 保持 xref-only 目标支持

沿用当前 `func_xref_strings` 的策略：

- 若某个函数只出现在 `func_xrefs` 中、但不在 `func_names` 中，也要自动并入统一的 func 处理流水线。

这样做的好处是：

- 保持现有脚本编写习惯
- `func_xrefs` 可独立承担目标函数发现职责
- 调用方不必为了进入处理流水线而重复维护 `func_names`

## 迁移策略

本次迁移采用单步破坏式迁移，不保留兼容层。

### 代码迁移范围

- `ida_analyze_util.py`
  - 删除 `func_xref_strings` 参数、注释、文档与内部变量
  - 新增 `func_xrefs` 参数、注释、文档与内部变量
  - 将旧的字符串专用 helper 升级为统一 helper
- `ida_preprocessor_scripts/`
  - 所有调用 `func_xref_strings=` 的脚本统一改为 `func_xrefs=`
  - 所有旧二元组统一改为三元组，第三项填 `[]`
- 新增或更新实际使用“字符串 + 函数联合约束”的脚本
  - 例如 `find-CNetworkGameClient_ProcessPacketEntities.py`

### 收尾检查

迁移完成后，代码库中不应再出现以下标识：

- `func_xref_strings`

这是一个明确的静态收尾条件。

## 失败语义

以下任一情况都应让当前 `preprocess_skill` 明确失败，而不是静默降级：

- `func_xrefs` 配置非法
- 依赖函数当前版本 YAML 缺失
- 依赖函数 YAML 中 `func_va` 缺失或非法
- 某个字符串没有任何候选函数
- 某个依赖函数没有任何引用它的候选函数
- 最终交集不是唯一函数
- 唯一候选定位成功但 `func_sig` 与基础函数信息都无法生成

失败信息应尽量带上：

- 当前目标函数名
- 出错的字符串或依赖函数名
- 出错的 YAML 文件名

这样便于快速判断是依赖顺序问题、输入配置问题还是 IDA 数据问题。

## 验证方案

本次设计对应的实现完成后，至少做以下定向验证：

1. 静态迁移检查
   - 全库搜索不再出现 `func_xref_strings`
2. 接口一致性检查
   - 至少一个旧字符串场景脚本已改成 `func_xrefs=(..., [...], [])`
3. 新功能场景检查
   - `CNetworkGameClient_ProcessPacketEntities` 使用“字符串 + 函数”联合约束
4. 失败语义检查
   - 当依赖函数 YAML 缺失时，日志明确指出缺失的是哪个依赖函数

本设计不要求额外引入新的测试框架；实现阶段以定向静态检查和最小运行验证为主。

## Serena Memory 更新

实现完成后，需要补充一条项目级 Serena memory，记录本次能力升级。

建议 memory 内容至少覆盖：

- `preprocess_common_skill` 已由 `func_xref_strings` 迁移为 `func_xrefs`
- `func_xrefs` 的三元组契约
- `xref_funcs_list` 只从当前版本 YAML 读取 `func_va`
- 依赖顺序由 `config.yaml` 中 skill 排序保证
- `func_xrefs` 支持纯字符串、纯函数、联合约束三种模式
- 迁移后不再保留 `func_xref_strings`

建议 memory 名称可采用：

- `preprocess_common_skill_func_xrefs`

或按后续项目 memory 命名规范调整，但应确保后续阅读者能直接发现该能力变更。

## 实施摘要

本次设计的核心是把“字符串交叉引用定位”升级为“统一的多证据交叉引用定位”：

- 对外统一接口：`func_xrefs`
- 对内统一算法：字符串集合与函数集合联合求交
- 对依赖地址统一约束：仅信任当前版本 YAML
- 对迁移统一要求：全量替换、不留兼容层

这样既保留了现有 `func_xref_strings` 的低成本用法，也为更复杂、更稳健的函数定位场景提供了直接支持。
