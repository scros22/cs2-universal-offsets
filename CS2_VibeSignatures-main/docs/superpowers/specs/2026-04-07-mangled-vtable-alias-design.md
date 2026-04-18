# `mangled_class_names` 与 `CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_vtable` 设计

## 背景

当前 `ida_preprocessor_scripts/find-*_vtable.py` 这类预处理脚本通过 `preprocess_common_skill(...)` 传入 `vtable_class_names`，再由 `preprocess_vtable_via_mcp(...)` 根据类名在 IDA 中定位 vtable。

现有实现仅支持两类“自动推导”的直接符号名：

- Windows：`??_7<Class>@@6B@`
- Linux：`_ZTV<len><Class>`

当目标 vtable 的真实符号不是简单的“类名直接拼接”形式，而是模板实例化后的 mangled 符号时，当前逻辑无法优先命中。例如：

- Windows：`??_7?$CGameSystemReallocatingFactory@VCSpawnGroupMgrGameSystem@@V1@@@6B@`
- Linux：`_ZTV30CGameSystemReallocatingFactoryI24CSpawnGroupMgrGameSystemS0_E`

本次需要在公共预处理链路中增加 `mangled_class_names` 支持，并新增 `find-CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_vtable` 作为首个使用者。

## 目标

- 在 `preprocess_common_skill(...)` 中支持传入 `mangled_class_names`。
- 保持 `vtable_class_names` 继续使用规范类名作为主键，不把 mangled 名混入输出命名。
- 让 `preprocess_vtable_via_mcp(...)` 在查找 vtable 时优先尝试显式 mangled alias。
- 尽量将这套 alias 机制复用到所有可透传的 vtable 查找路径，而不是只覆盖“直接 vtable skill”。
- 新增 `find-CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_vtable` skill 与对应 symbol 配置。

## 非目标

- 不改动 vtable YAML 的 schema。
- 不把 mangled 名暴露到 YAML 文件名或 symbol 名中。
- 不重构整个 vtable 发现框架为“任意符号列表驱动”模式。
- 不默认执行测试或 build。

## 选定方案

采用“规范类名 + alias 映射”的最小增量方案。

### 配置形状

在 skill 脚本中新增：

```python
MANGLED_CLASS_NAMES = {
    "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem": [
        "??_7?$CGameSystemReallocatingFactory@VCSpawnGroupMgrGameSystem@@V1@@@6B@",
        "_ZTV30CGameSystemReallocatingFactoryI24CSpawnGroupMgrGameSystemS0_E",
    ],
}
```

并继续使用：

```python
TARGET_CLASS_NAMES = [
    "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem",
]
```

其中：

- `TARGET_CLASS_NAMES` 决定输出 YAML 的规范命名。
- `MANGLED_CLASS_NAMES` 仅作为查找 alias，不参与产物命名。

## 详细设计

### 1. `preprocess_common_skill(...)` 接口扩展

在 `ida_analyze_util.py` 的 `preprocess_common_skill(...)` 新增可选参数：

```python
mangled_class_names=None
```

其语义为：

- 类型：`dict[str, list[str]]`
- key：规范类名
- value：该类对应的 mangled alias 列表

行为：

1. 对参数做轻量校验：
   - 必须是字典
   - key 必须是非空字符串
   - value 必须是字符串列表
2. 当处理每个 `vtable_class` 时，通过 `mangled_class_names.get(vtable_class, [])` 取得 alias 列表。
3. 将 alias 列表透传给 `preprocess_vtable_via_mcp(...)`。

这样所有 skill 仍然围绕规范类名组织，只是多了一层可选查找增强。

### 2. `preprocess_vtable_via_mcp(...)` 接口扩展

在 `ida_analyze_util.py` 的 `preprocess_vtable_via_mcp(...)` 新增可选参数：

```python
symbol_aliases=None
```

其职责仅为：

- 接收显式候选符号名列表
- 优先尝试这些符号名定位 vtable
- 若全部失败，再回退到现有自动推导与 RTTI fallback

返回结构保持不变，继续产出：

- `vtable_class`
- `vtable_symbol`
- `vtable_va`
- `vtable_rva`
- `vtable_size`
- `vtable_numvfunc`
- `vtable_entries`

因此 YAML schema 与下游读取逻辑无需变更。

### 3. vtable 查找顺序

`preprocess_vtable_via_mcp(...)` 的底层 py_eval 查找顺序调整为：

1. 先遍历 `symbol_aliases`
2. 再尝试现有自动推导的直接符号名
   - Windows：`??_7<Class>@@6B@`
   - Linux：`_ZTV<len><Class>`
3. 最后保留现有 RTTI / typeinfo fallback

具体规则：

- 当 alias 为 Linux `_ZTV...` 形式时，继续沿用现有 `+0x10` 规则，将 vtable 起始位置移动到函数表首项。
- 当 alias 为 Windows `??_7...` 形式时，保持当前起始位置逻辑不变。
- 一旦命中任一 alias，`vtable_symbol` 记录实际命中的符号名。
- 若所有 alias 都未命中，不直接失败，而是继续走当前 fallback。

### 4. py_eval 模板改造方式

当前 `_build_vtable_py_eval(...)` 仅把单个 `class_name` 填入模板。扩展后改为同时向模板传递：

- `class_name`
- `candidate_symbols`

IDA 侧模板优先遍历 `candidate_symbols` 并调用 `ida_name.get_name_ea(...)`：

- 命中后直接确定 `vtable_start` 与 `vtable_symbol`
- 未命中则继续现有逻辑

这能把“显式 mangled 名查找”集中在底层，避免在 Python 侧复制多套平台分支。

### 5. 复用到其他 vtable 查找路径

本次不应只改“直接 vtable skill”路径，还应尽量复用到以下调用链：

#### `preprocess_common_skill(...)` 的 vtable 目标分支

这是本次新增功能的主入口，必须支持。

#### `func_vtable_relations` enrichment

当函数已经定位成功，但还需要反查其在某个 vtable 中的 index / offset 时，也会再次调用 `preprocess_vtable_via_mcp(...)`。这条路径应尽量透传同一份 alias 映射，以避免某些模板类只能生成 vtable YAML，却无法补充 `vfunc_index`。

#### `preprocess_func_sig_via_mcp()` 内部按需生成 vtable YAML 的路径

该函数内部 `_load_vtable_data(...)` 在本地缺失 `*_vtable.yaml` 时，会调用 `preprocess_vtable_via_mcp(...)` 临时生成。因此这里也应支持基于规范类名查询 alias 映射后透传，保证缺失 YAML 的情况下仍能找到模板类 vtable。

#### `inherit_vfuncs`

`preprocess_index_based_vfunc_via_mcp()` 本身读取的是已存在的 vtable YAML，不直接调用 IDA 做类名查找。因此它不需要新增独立逻辑，但会间接受益于前述 vtable 生成路径的增强。

### 6. alias 获取辅助逻辑

为了避免 `preprocess_common_skill(...)`、`func_vtable_relations` 和 `_load_vtable_data(...)` 各自重复处理映射，建议增加一个轻量 helper，例如：

- 负责校验 `mangled_class_names`
- 负责根据规范类名返回 alias 列表
- 缺失时统一返回空列表

这样可以减少重复类型判断，并让调试输出更一致。

## 新增 skill

新增文件：

- `ida_preprocessor_scripts/find-CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_vtable.py`

建议脚本结构：

```python
#!/usr/bin/env python3
"""Preprocess script for find-CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_vtable skill."""

from ida_analyze_util import preprocess_common_skill

TARGET_CLASS_NAMES = [
    "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem",
]

MANGLED_CLASS_NAMES = {
    "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem": [
        "??_7?$CGameSystemReallocatingFactory@VCSpawnGroupMgrGameSystem@@V1@@@6B@",
        "_ZTV30CGameSystemReallocatingFactoryI24CSpawnGroupMgrGameSystemS0_E",
    ],
}

async def preprocess_skill(
    session, skill_name, expected_outputs, old_yaml_map,
    new_binary_dir, platform, image_base, debug=False,
):
    return await preprocess_common_skill(
        session=session,
        expected_outputs=expected_outputs,
        vtable_class_names=TARGET_CLASS_NAMES,
        mangled_class_names=MANGLED_CLASS_NAMES,
        platform=platform,
        image_base=image_base,
        debug=debug,
    )
```

## `config.yaml` 变更

需要在对应模块中新增 skill 配置：

```yaml
- name: find-CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_vtable
  expected_output:
    - CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_vtable.{platform}.yaml
```

并新增 symbol：

```yaml
- name: CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_vtable
  category: vtable
```

文件名和 symbol 名始终使用规范类名，不使用 mangled 名。

## 失败处理

### 配置非法

当 `mangled_class_names` 结构不合法时：

- 在 `debug=True` 下输出明确错误
- 当前 `preprocess_common_skill(...)` 返回 `False`

### alias 未命中

当显式 alias 全部未命中时：

- 不直接失败
- 继续尝试现有自动推导名与 RTTI fallback

### 完全查找失败

只有当以下三类手段全部失败时，`preprocess_vtable_via_mcp(...)` 才返回 `None`：

- 显式 alias
- 自动推导的直接符号名
- RTTI / typeinfo fallback

这与当前“底层查找失败即返回 `None`”的语义保持一致。

## 兼容性

- 未传 `mangled_class_names` 的旧 skill 行为保持不变。
- 旧的 vtable YAML 格式、字段顺序与下游消费者保持兼容。
- 旧的 `vtable_class_names` 仍只接受规范类名，无需迁移。
- 新功能只增强查找能力，不改变输出命名语义。

## 风险与约束

- Linux `_ZTV...` alias 的起始地址仍需保留 `+0x10` 修正，不能把 alias 命中路径与自动推导路径分裂成不同规则。
- 某些 alias 可能指向非标准符号或错误地址，因此候选命中后仍要经过现有 vtable 解析逻辑验证。
- 如果后续有更多模板类需要支持，应继续沿用“规范类名 + alias 映射”模式，避免回退到平铺列表或特判脚本。

## 验证策略

本次按 Level 0 定向验证设计：

- 确认新接口不破坏旧调用方的默认参数行为。
- 确认新 skill 脚本只声明常量并复用 `preprocess_common_skill(...)`。
- 确认 `config.yaml` 中新增的 skill 与 symbol 命名一致。
- 不在本次设计阶段默认运行测试或 build；若进入实现阶段并需要额外验证，再单独执行。

## 实施范围

预计改动文件：

- `ida_analyze_util.py`
- `ida_preprocessor_scripts/find-CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_vtable.py`
- `config.yaml`

本次设计刻意保持最小修改面：只增强公共 vtable 查找入口与新增一个使用示例，不扩展到更重的全局重构。
