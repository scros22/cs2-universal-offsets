# 跨模块 artifact 引用与 inherit_vfuncs 扩展设计

## 背景

当前 `ida_analyze_bin.py` 允许通过 `expected_input` 与 `expected_output` 描述 skill 之间的文件依赖，但默认假设这些 artifact 都位于当前模块目录下。新增的 `networksystem` 二次执行段需要引用 `server` 模块已经生成的 YAML，例如：

```yaml
expected_input:
  - CFlattenedSerializers_vtable.{platform}.yaml
  - ../server/CFlattenedSerializers_CreateFieldChangedEventQueue.{platform}.yaml
```

同时，`find-CFlattenedSerializers_CreateFieldChangedEventQueue-impl` 的预处理希望复用现有的 `inherit_vfuncs` / `preprocess_index_based_vfunc_via_mcp()` 能力：从其他模块已有 YAML 中读取 `vfunc_index` 或 `vfunc_offset`，再结合当前模块的 vtable 还原真实函数地址并生成完整函数 YAML。

## 目标

- 支持 `expected_input` 使用 `../<module>/...` 形式引用同一 `gamever` 下其他模块目录内的 YAML。
- 保持模块执行顺序仍由 `config.yaml` 决定，不引入跨模块自动调度。
- 允许同一模块在 `config.yaml` 中出现多次，并按书写顺序再次执行新增 skills。
- 扩展 `inherit_vfuncs` 的第三个参数，使其既兼容当前模块内的 YAML stem，也支持 `../<module>/<yaml-stem>` 形式的跨模块引用。
- 让 `find-CFlattenedSerializers_CreateFieldChangedEventQueue-impl` 通过预处理直接产出完整函数 YAML：
  - `func_name`
  - `func_va`
  - `func_rva`
  - `func_size`
  - `func_sig`
  - `vtable_name`
  - `vfunc_offset`
  - `vfunc_index`

## 非目标

- 不引入模块级全局拓扑排序。
- 不支持跳出 `bin/{gamever}` 根目录的任意路径访问。
- 不重构现有 `process_binary()` 主流程。
- 不为所有跨模块场景建立新的 artifact graph 框架。

## 选定方案

采用最小增量方案：

1. 在 `ida_analyze_bin.py` 中引入统一的 artifact 路径解析 helper。
2. 将 `expected_input` 的存在性检查与相关依赖匹配统一到这一解析语义。
3. 不改变 `main()` 的模块遍历方式；第二个 `networksystem` 仍作为一次新的模块遍历执行。
4. 新增一个很薄的预处理脚本 `ida_preprocessor_scripts/find-CFlattenedSerializers_CreateFieldChangedEventQueue-impl.py`，仅声明 `inherit_vfuncs` 并调用 `preprocess_common_skill(...)`。
5. 扩展 `preprocess_index_based_vfunc_via_mcp()`，让其自动支持从 `../server/...` 这类引用读取 base YAML 并提取 `vfunc_index` / `vfunc_offset`。

## 配置语义

### `expected_input`

- 旧语义保持不变：
  - `Foo.{platform}.yaml` 表示当前模块目录下的 artifact。
- 新增语义：
  - `../server/Foo.{platform}.yaml` 表示同一 `gamever` 下兄弟模块 `server` 目录内的 artifact。

### `inherit_vfuncs`

继续沿用四元组：

```python
(target_func_name, inherit_vtable_class, base_vfunc_name, generate_func_sig)
```

其中第三项新增两种合法形式：

- 当前模块内引用：`CBaseEntity_Touch`
- 跨模块引用：`../server/CFlattenedSerializers_CreateFieldChangedEventQueue`

本次目标 skill 的预处理配置形如：

```python
INHERIT_VFUNCS = [
    (
        "CFlattenedSerializers_CreateFieldChangedEventQueue",
        "CFlattenedSerializers",
        "../server/CFlattenedSerializers_CreateFieldChangedEventQueue",
        True,
    ),
]
```

## 详细设计

### 1. artifact 路径解析

在 `ida_analyze_bin.py` 中新增统一 helper，输入：

- 当前 `binary_dir`
- artifact 相对描述
- `platform`

输出：

- 展开 `{platform}` 后的规范化绝对路径

规则：

- `Foo.{platform}.yaml` -> `bin/{gamever}/{current_module}/Foo.{platform}.yaml`
- `../server/Foo.{platform}.yaml` -> `bin/{gamever}/server/Foo.{platform}.yaml`

安全约束：

- 解析结果必须仍位于 `bin/{gamever}` 根目录下。
- 任何越界路径均视为非法 artifact 配置。

该 helper 将用于：

- `expand_expected_paths()` 的统一实现
- `expected_input` 文件存在性检查
- `topological_sort_skills()` 对同模块内部 artifact 名称的匹配规范化

说明：

- 本次不尝试让 `topological_sort_skills()` 在不同模块之间建立全局依赖边。
- 跨模块执行顺序仍由 `config.yaml` 中模块出现顺序保证。

### 2. `process_binary()` 中的输入检查

`process_binary()` 当前直接用：

```python
os.path.join(binary_dir, f.replace("{platform}", platform))
```

构造 `expected_input` 路径。该逻辑改为统一调用 artifact resolver。

行为：

- 解析成功且文件存在：继续执行。
- 解析成功但文件缺失：保持现有行为，skill 直接失败，不进入 Agent fallback。
- 解析失败或越界：输出明确错误并计为失败。

### 3. `preprocess_index_based_vfunc_via_mcp()` 扩展

该函数当前假设 `base_vfunc_name` 一定位于当前模块目录下：

```python
{new_binary_dir}/{base_vfunc_name}.{platform}.yaml
```

扩展后行为：

1. 新增 base artifact 解析逻辑，支持：
   - 当前模块 stem
   - `../<module>/<stem>` 跨模块 stem
2. 读取解析后的 base YAML。
3. 槽位提取规则：
   - 优先取 `vfunc_index`
   - 若不存在 `vfunc_index`，则从 `vfunc_offset` 反推 `index = offset / 8`
   - 若两者同时存在，则校验一致
4. 读取当前模块 `inherit_vtable_class` 对应的 vtable YAML。
5. 使用槽位索引在当前模块 vtable 中查得真实函数地址。
6. 用 IDA 查询该地址的函数边界，生成：
   - `func_va`
   - `func_size`
   - `func_rva`
7. `func_sig` 处理：
   - 若目标输出对应旧版本 YAML 存在且含 `func_sig`，优先复用
   - 否则在 `generate_func_sig=True` 时自动调用 `preprocess_gen_func_sig_via_mcp()`
8. 输出 payload 保持与 `write_func_yaml()` 的字段顺序和兼容性一致。

### 4. 新增预处理脚本

新增：

- `ida_preprocessor_scripts/find-CFlattenedSerializers_CreateFieldChangedEventQueue-impl.py`

职责：

- 声明 `INHERIT_VFUNCS`
- 调用 `preprocess_common_skill(...)`

不在脚本内手写额外的跨模块 YAML 读取逻辑，避免重复实现。

### 5. 对重复模块的语义确认

`main()` 继续按 `config.yaml` 顺序逐项遍历 `modules`。因此：

- 第一个 `networksystem` 块生成基础 artifact
- `server` 块生成 `../server/...` 依赖
- 第二个 `networksystem` 块消费这些依赖并执行新增 skills

这不是新增能力，而是对现有行为的保留和利用。

## 失败处理

### artifact 解析失败

- 路径越界或格式非法时，当前 skill 记为失败。

### `expected_input` 缺失

- 保持现有行为，当前 skill 记为失败，不进入 Agent fallback。

### 预处理失败

`preprocess_index_based_vfunc_via_mcp()` 在以下场景返回 `None`：

- base YAML 读取失败
- `vfunc_index` / `vfunc_offset` 缺失
- `vfunc_offset` 不是 8 字节对齐
- `vfunc_index` 与 `vfunc_offset` 不一致
- 当前模块 vtable YAML 缺失或格式错误
- 目标索引在当前 vtable 中不存在
- IDA 无法返回函数边界

上层继续沿用现有策略：

- preprocess 失败后可进入 Agent skill fallback
- preprocess 成功但目标 YAML 未写出时，仍记为失败

## 兼容性

- 现有 `inherit_vfuncs` 配置无需改写。
- 现有只在当前模块目录内查找 base YAML 的脚本继续可用。
- `base_vfunc_name` 的名称推导逻辑保持现状；对于 `../server/...` 这种新写法，最终仍会自然回退到 `target_func_name`。
- 不影响 `preprocess_common_skill()` 的其他目标类型：`func_names`、`gv_names`、`patch_names`、`struct_member_names`、`vtable_class_names`、`func_xrefs`。

## 验证策略

本次为轻量实现，采用定向验证：

1. 路径解析验证
   - 当前模块 artifact
   - `../server/...` artifact
   - 越界路径拒绝
2. `preprocess_index_based_vfunc_via_mcp()` 验证
   - 旧式当前模块 stem
   - 新式跨模块 stem
   - 仅有 `vfunc_offset` 时能正确反推 index
3. 新预处理脚本验证
   - `inherit_vfuncs` 配置正确
   - `preprocess_common_skill(...)` 调用参数正确

除非实现阶段发现现有测试框架非常适合覆盖该场景，否则不主动扩大到全量运行。

## 影响文件

- `ida_analyze_bin.py`
- `ida_analyze_util.py`
- `ida_preprocessor_scripts/find-CFlattenedSerializers_CreateFieldChangedEventQueue-impl.py`

## 实施后预期结果

- `config.yaml` 中 `../server/CFlattenedSerializers_CreateFieldChangedEventQueue.{platform}.yaml` 可作为合法 `expected_input`。
- 第二个 `networksystem` 模块块在 `server` 之后执行时，可正确消费跨模块输入。
- `find-CFlattenedSerializers_CreateFieldChangedEventQueue-impl` 可通过 `inherit_vfuncs` 从 `server` 侧 YAML 提取槽位信息，并在当前模块中恢复真实实现函数，生成完整函数 YAML。
