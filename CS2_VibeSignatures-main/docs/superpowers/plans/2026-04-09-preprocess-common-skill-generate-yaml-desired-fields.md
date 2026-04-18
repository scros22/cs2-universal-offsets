# Preprocess Common Skill Desired Fields Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 让 `preprocess_common_skill(...)` 强制以 `GENERATE_YAML_DESIRED_FIELDS` 作为唯一 YAML 输出契约，移除 `generate_vfunc_offset` 语义，并完成 `ida_preprocessor_scripts/` 下所有 `preprocess_common_skill(...)` 脚本的全量迁移。

**Architecture:** 先用 `unittest` 锁定三类行为：字段契约必传与严格失败、writer/公共层只输出声明字段、脚本层必须显式转发 `generate_yaml_desired_fields` 且 `FUNC_VTABLE_RELATIONS` 改为二元组。随后在 `ida_analyze_util.py` 中加入目标类型字段顺序、字段契约规范化、payload 组装与严格校验逻辑，并把 `preprocess_common_skill(...)` 改为“候选字段收集 -> 按契约组装 -> 写 YAML”的两阶段模型。最后用一次性的源码迁移脚本批量补齐绝大多数脚本，再手工修正 `INetworkMessages_FindNetworkGroup` 这类特殊 vfunc 场景并跑定向回归。

**Tech Stack:** Python 3、`unittest`、`unittest.mock`、`pathlib`、`ast`、`yaml.safe_dump`、`uv`

---

## File Structure

- Modify: `ida_analyze_util.py`
  - 新增字段顺序常量、symbol 类型合法字段集合
  - 新增 `generate_yaml_desired_fields` 规范化/校验 helper
  - 新增按字段契约组装最终 payload 的 helper
  - 调整 `preprocess_common_skill(...)` 签名、文档字符串和 `FUNC_VTABLE_RELATIONS` 解析逻辑
  - 让 `write_vtable_yaml(...)` 与其他 writer 一样支持“仅写已提供字段”
- Modify: `tests/test_ida_analyze_util.py`
  - 新增字段契约必传、字段缺失严格失败、vtable relation 二元组、非函数类型 payload 过滤测试
- Modify: `tests/test_ida_preprocessor_scripts.py`
  - 更新 `find-CNetworkMessages_FindNetworkGroup.py` 与 `find-INetworkMessages_FindNetworkGroup.py` 的转发断言
  - 新增普通函数脚本必须转发 `GENERATE_YAML_DESIRED_FIELDS` 的断言
- Modify: `ida_preprocessor_scripts/*.py`
  - 全量补齐 `GENERATE_YAML_DESIRED_FIELDS`
  - 所有 `FUNC_VTABLE_RELATIONS` 从三元组迁移到二元组
  - 所有 `preprocess_common_skill(...)` 调用都显式传入 `generate_yaml_desired_fields=GENERATE_YAML_DESIRED_FIELDS`
- Create: `docs/superpowers/plans/2026-04-09-preprocess-common-skill-generate-yaml-desired-fields.md`
  - 当前实现计划文档

**仓库约束：**

- 在独立 worktree 中执行本计划，例如 `.worktrees/preprocess-common-skill-generate-yaml-desired-fields`
- 实施阶段优先跑定向 `unittest`，不要先跑全仓 build
- `git commit` 消息遵循仓库约定：`<type>(scope): <中文动词开头摘要>`

## 迁移模板

后续批量迁移脚本时统一使用以下字段模板：

```python
FUNC_FIELDS = ["func_name", "func_va", "func_rva", "func_size", "func_sig"]
VFUNC_FIELDS = FUNC_FIELDS + ["vtable_name", "vfunc_offset", "vfunc_index"]
INHERIT_VFUNC_FIELDS = ["func_name", "func_va", "func_rva", "func_size", "vtable_name", "vfunc_offset", "vfunc_index"]
GV_FIELDS = [
    "gv_name",
    "gv_va",
    "gv_rva",
    "gv_sig",
    "gv_sig_va",
    "gv_inst_offset",
    "gv_inst_length",
    "gv_inst_disp",
]
VTABLE_FIELDS = [
    "vtable_class",
    "vtable_symbol",
    "vtable_va",
    "vtable_rva",
    "vtable_size",
    "vtable_numvfunc",
    "vtable_entries",
]
PATCH_FIELDS = ["patch_name", "patch_sig", "patch_bytes"]
STRUCT_MEMBER_FIELDS = [
    "struct_name",
    "member_name",
    "offset",
    "size",
    "offset_sig",
    "offset_sig_disp",
]
```

`find-INetworkMessages_FindNetworkGroup.py` 不用通用 `VFUNC_FIELDS`，而是显式保留：

```python
[
    "func_name",
    "vfunc_sig",
    "vfunc_offset",
    "vfunc_index",
    "vtable_name",
]
```

### Task 1: 先补 `ida_analyze_util.py` 的 failing tests

**Files:**
- Modify: `tests/test_ida_analyze_util.py`

- [ ] **Step 1: 新增字段契约必传的回归测试**

在 `TestVtableAliasSupport` 后面追加这个测试类和第一个测试：

```python
class TestGenerateYamlDesiredFieldsContract(unittest.IsolatedAsyncioTestCase):
    async def test_preprocess_common_skill_rejects_missing_generate_yaml_desired_fields(
        self,
    ) -> None:
        result = await ida_analyze_util.preprocess_common_skill(
            session="session",
            expected_outputs=["/tmp/Foo.windows.yaml"],
            old_yaml_map={},
            new_binary_dir="/tmp",
            platform="windows",
            image_base=0x180000000,
            func_names=["Foo"],
            debug=True,
        )

        self.assertFalse(result)
```

- [ ] **Step 2: 新增函数 payload 只输出声明字段且使用二元组 relation 的测试**

继续在同一个测试类里加入：

```python
    async def test_preprocess_common_skill_filters_func_payload_by_desired_fields(
        self,
    ) -> None:
        with patch.object(
            ida_analyze_util,
            "preprocess_func_sig_via_mcp",
            AsyncMock(
                return_value={
                    "func_name": "Foo",
                    "func_va": "0x180004000",
                    "func_rva": "0x4000",
                    "func_size": "0x40",
                    "func_sig": "AA BB",
                }
            ),
        ), patch.object(
            ida_analyze_util,
            "preprocess_vtable_via_mcp",
            AsyncMock(
                return_value={
                    "vtable_class": "Bar",
                    "vtable_symbol": "??_7Bar@@6B@",
                    "vtable_va": "0x180001000",
                    "vtable_rva": "0x1000",
                    "vtable_size": "0x20",
                    "vtable_numvfunc": 2,
                    "vtable_entries": {
                        0: "0x180003000",
                        1: "0x180004000",
                    },
                }
            ),
        ) as mock_preprocess_vtable, patch.object(
            ida_analyze_util,
            "write_func_yaml",
        ) as mock_write_func_yaml, patch.object(
            ida_analyze_util,
            "_rename_func_in_ida",
            AsyncMock(return_value=None),
        ):
            result = await ida_analyze_util.preprocess_common_skill(
                session="session",
                expected_outputs=["/tmp/Foo.windows.yaml"],
                old_yaml_map={},
                new_binary_dir="/tmp",
                platform="windows",
                image_base=0x180000000,
                func_names=["Foo"],
                func_vtable_relations=[("Foo", "Bar")],
                generate_yaml_desired_fields=[
                    ("Foo", ["func_name", "vtable_name", "vfunc_offset", "vfunc_index"])
                ],
                debug=True,
            )

        self.assertTrue(result)
        mock_preprocess_vtable.assert_awaited_once()
        mock_write_func_yaml.assert_called_once()
        self.assertEqual(
            {
                "func_name": "Foo",
                "vtable_name": "Bar",
                "vfunc_offset": "0x8",
                "vfunc_index": 1,
            },
            mock_write_func_yaml.call_args.args[1],
        )
```

- [ ] **Step 3: 新增声明了缺失字段就失败的测试**

继续追加：

```python
    async def test_preprocess_common_skill_rejects_missing_requested_func_field(
        self,
    ) -> None:
        with patch.object(
            ida_analyze_util,
            "preprocess_func_sig_via_mcp",
            AsyncMock(return_value={"func_name": "Foo", "func_sig": "AA BB"}),
        ), patch.object(
            ida_analyze_util,
            "write_func_yaml",
        ) as mock_write_func_yaml:
            result = await ida_analyze_util.preprocess_common_skill(
                session="session",
                expected_outputs=["/tmp/Foo.windows.yaml"],
                old_yaml_map={},
                new_binary_dir="/tmp",
                platform="windows",
                image_base=0x180000000,
                func_names=["Foo"],
                generate_yaml_desired_fields=[("Foo", ["func_name", "func_va"])],
                debug=True,
            )

        self.assertFalse(result)
        mock_write_func_yaml.assert_not_called()
```

- [ ] **Step 4: 新增 vtable payload 也必须按声明字段过滤的测试**

继续追加：

```python
    async def test_preprocess_common_skill_filters_vtable_payload_by_desired_fields(
        self,
    ) -> None:
        with patch.object(
            ida_analyze_util,
            "preprocess_vtable_via_mcp",
            AsyncMock(
                return_value={
                    "vtable_class": "Foo",
                    "vtable_symbol": "??_7Foo@@6B@",
                    "vtable_va": "0x180001000",
                    "vtable_rva": "0x1000",
                    "vtable_size": "0x20",
                    "vtable_numvfunc": 4,
                    "vtable_entries": {0: "0x180010000"},
                }
            ),
        ), patch.object(
            ida_analyze_util,
            "write_vtable_yaml",
        ) as mock_write_vtable_yaml:
            result = await ida_analyze_util.preprocess_common_skill(
                session="session",
                expected_outputs=["/tmp/Foo_vtable.windows.yaml"],
                vtable_class_names=["Foo"],
                platform="windows",
                image_base=0x180000000,
                generate_yaml_desired_fields=[
                    ("Foo", ["vtable_class", "vtable_entries"])
                ],
                debug=True,
            )

        self.assertTrue(result)
        mock_write_vtable_yaml.assert_called_once_with(
            "/tmp/Foo_vtable.windows.yaml",
            {
                "vtable_class": "Foo",
                "vtable_entries": {0: "0x180010000"},
            },
        )
```

- [ ] **Step 5: 运行新增 util 测试，确认先失败**

Run:

```bash
uv run python -m unittest \
  tests.test_ida_analyze_util.TestGenerateYamlDesiredFieldsContract -v
```

Expected:

```text
FAIL: test_preprocess_common_skill_rejects_missing_generate_yaml_desired_fields
AssertionError: True is not false
```

- [ ] **Step 6: 提交测试基线**

```bash
git add tests/test_ida_analyze_util.py
git commit -m "test(preprocess): 补充字段契约回归测试"
```

### Task 2: 先补脚本层 forwarding 的 failing tests

**Files:**
- Modify: `tests/test_ida_preprocessor_scripts.py`

- [ ] **Step 1: 更新 `find-CNetworkMessages_FindNetworkGroup.py` 的预期参数**

把 `TestFindCNetworkMessagesFindNetworkGroup` 中的断言改成下面这样：

```python
        expected_func_vtable_relations = [
            ("CNetworkMessages_FindNetworkGroup", "CNetworkMessages")
        ]
        expected_generate_yaml_desired_fields = [
            (
                "CNetworkMessages_FindNetworkGroup",
                [
                    "func_name",
                    "func_va",
                    "func_rva",
                    "func_size",
                    "func_sig",
                    "vtable_name",
                    "vfunc_offset",
                    "vfunc_index",
                ],
            )
        ]
```

并把最终 `assert_awaited_once_with(...)` 改成：

```python
        mock_preprocess_common_skill.assert_awaited_once_with(
            session="session",
            expected_outputs=["out.yaml"],
            old_yaml_map={"k": "v"},
            new_binary_dir="bin_dir",
            platform="windows",
            image_base=0x180000000,
            func_names=["CNetworkMessages_FindNetworkGroup"],
            func_vtable_relations=expected_func_vtable_relations,
            inherit_vfuncs=expected_inherit_vfuncs,
            generate_yaml_desired_fields=expected_generate_yaml_desired_fields,
            llm_config=llm_config,
            debug=True,
        )
```

- [ ] **Step 2: 更新 `find-INetworkMessages_FindNetworkGroup.py` 的预期参数**

把 `TestFindINetworkMessagesFindNetworkGroup` 中的预期常量替换为：

```python
        expected_func_vtable_relations = [
            ("INetworkMessages_FindNetworkGroup", "INetworkMessages")
        ]
        expected_generate_yaml_desired_fields = [
            (
                "INetworkMessages_FindNetworkGroup",
                [
                    "func_name",
                    "vfunc_sig",
                    "vfunc_offset",
                    "vfunc_index",
                    "vtable_name",
                ],
            )
        ]
```

并把断言改成：

```python
        mock_preprocess_common_skill.assert_awaited_once_with(
            session="session",
            expected_outputs=["out.yaml"],
            old_yaml_map={"k": "v"},
            new_binary_dir="bin_dir",
            platform="windows",
            image_base=0x180000000,
            func_names=["INetworkMessages_FindNetworkGroup"],
            func_vtable_relations=expected_func_vtable_relations,
            llm_decompile_specs=expected_llm_decompile_specs,
            generate_yaml_desired_fields=expected_generate_yaml_desired_fields,
            llm_config=llm_config,
            debug=True,
        )
```

- [ ] **Step 3: 新增普通函数脚本必须转发字段契约的测试**

在文件底部追加：

```python
class TestFindCBaseEntityCollisionRulesChanged(unittest.IsolatedAsyncioTestCase):
    async def test_preprocess_skill_forwards_generate_yaml_desired_fields(self) -> None:
        module = _load_module(
            "ida_preprocessor_scripts/find-CBaseEntity_CollisionRulesChanged.py",
            "find_CBaseEntity_CollisionRulesChanged",
        )
        mock_preprocess_common_skill = AsyncMock(return_value=True)
        expected_generate_yaml_desired_fields = [
            (
                "CBaseEntity_CollisionRulesChanged",
                ["func_name", "func_va", "func_rva", "func_size", "func_sig"],
            )
        ]

        with patch.object(
            module,
            "preprocess_common_skill",
            mock_preprocess_common_skill,
        ):
            result = await module.preprocess_skill(
                session="session",
                skill_name="skill",
                expected_outputs=["out.yaml"],
                old_yaml_map={"k": "v"},
                new_binary_dir="bin_dir",
                platform="windows",
                image_base=0x180000000,
                debug=True,
            )

        self.assertTrue(result)
        mock_preprocess_common_skill.assert_awaited_once_with(
            session="session",
            expected_outputs=["out.yaml"],
            old_yaml_map={"k": "v"},
            new_binary_dir="bin_dir",
            platform="windows",
            image_base=0x180000000,
            func_names=["CBaseEntity_CollisionRulesChanged"],
            generate_yaml_desired_fields=expected_generate_yaml_desired_fields,
            debug=True,
        )
```

- [ ] **Step 4: 运行脚本 forwarding 测试，确认先失败**

Run:

```bash
uv run python -m unittest \
  tests.test_ida_preprocessor_scripts.TestFindCNetworkMessagesFindNetworkGroup \
  tests.test_ida_preprocessor_scripts.TestFindINetworkMessagesFindNetworkGroup \
  tests.test_ida_preprocessor_scripts.TestFindCBaseEntityCollisionRulesChanged -v
```

Expected:

```text
FAIL: test_preprocess_skill_forwards_generate_yaml_desired_fields
AssertionError: expected await not found
```

- [ ] **Step 5: 提交脚本层测试基线**

```bash
git add tests/test_ida_preprocessor_scripts.py
git commit -m "test(preprocess): 补充脚本字段转发断言"
```

### Task 3: 在 `ida_analyze_util.py` 引入字段顺序与 payload 组装 helper

**Files:**
- Modify: `ida_analyze_util.py`

- [ ] **Step 1: 把 writer 固定字段顺序提升为公共常量**

在现有 writer 定义上方加入：

```python
FUNC_YAML_ORDER = [
    "func_name",
    "func_va",
    "func_rva",
    "func_size",
    "func_sig",
    "vtable_name",
    "vfunc_offset",
    "vfunc_index",
    "vfunc_sig",
]
GV_YAML_ORDER = [
    "gv_name",
    "gv_va",
    "gv_rva",
    "gv_sig",
    "gv_sig_va",
    "gv_inst_offset",
    "gv_inst_length",
    "gv_inst_disp",
]
VTABLE_YAML_ORDER = [
    "vtable_class",
    "vtable_symbol",
    "vtable_va",
    "vtable_rva",
    "vtable_size",
    "vtable_numvfunc",
    "vtable_entries",
]
PATCH_YAML_ORDER = ["patch_name", "patch_sig", "patch_bytes"]
STRUCT_MEMBER_YAML_ORDER = [
    "struct_name",
    "member_name",
    "offset",
    "size",
    "offset_sig",
    "offset_sig_disp",
]
TARGET_KIND_TO_FIELD_ORDER = {
    "func": FUNC_YAML_ORDER,
    "gv": GV_YAML_ORDER,
    "vtable": VTABLE_YAML_ORDER,
    "patch": PATCH_YAML_ORDER,
    "struct_member": STRUCT_MEMBER_YAML_ORDER,
}
TARGET_KIND_TO_FIELD_SET = {
    kind: set(field_order)
    for kind, field_order in TARGET_KIND_TO_FIELD_ORDER.items()
}
```

- [ ] **Step 2: 新增公共 payload 组装 helper**

在 writer 上方加入：

```python
def _build_ordered_yaml_payload(data, ordered_keys):
    payload = {}
    for key in ordered_keys:
        if key not in data:
            continue
        value = data[key]
        if key == "vtable_entries":
            normalized_entries = {
                int(entry_index): str(entry_value)
                for entry_index, entry_value in value.items()
            }
            payload[key] = dict(sorted(normalized_entries.items()))
            continue
        if key.endswith("_va") or key.endswith("_rva") or key.endswith("_size"):
            payload[key] = str(value)
            continue
        payload[key] = value
    return payload
```

并把 writer 改成统一调用该 helper，例如：

```python
def write_vtable_yaml(path, data):
    """Write vtable YAML matching the format produced by write-vtable-as-yaml skill."""
    if yaml is None:
        raise RuntimeError("PyYAML is required to write vtable YAML")

    payload = _build_ordered_yaml_payload(data, VTABLE_YAML_ORDER)

    with open(path, "w", encoding="utf-8") as f:
        yaml.safe_dump(
            payload,
            f,
            sort_keys=False,
            default_flow_style=False,
            allow_unicode=False,
        )
```

`write_func_yaml(...)`、`write_gv_yaml(...)`、`write_patch_yaml(...)`、`write_struct_offset_yaml(...)` 也改成同样的调用方式。

- [ ] **Step 3: 新增字段契约规范化 helper**

继续加入：

```python
def _normalize_generate_yaml_desired_fields(generate_yaml_desired_fields, debug=False):
    if not generate_yaml_desired_fields:
        if debug:
            print("    Preprocess: missing generate_yaml_desired_fields")
        return None

    normalized = {}
    for spec in generate_yaml_desired_fields:
        if not isinstance(spec, (tuple, list)) or len(spec) != 2:
            if debug:
                print(f"    Preprocess: invalid desired-fields spec: {spec}")
            return None

        symbol_name, desired_fields = spec
        if not isinstance(symbol_name, str) or not symbol_name:
            if debug:
                print(f"    Preprocess: invalid desired-fields symbol: {symbol_name}")
            return None
        if symbol_name in normalized:
            if debug:
                print(f"    Preprocess: duplicated desired-fields symbol: {symbol_name}")
            return None
        if not isinstance(desired_fields, (tuple, list)) or not desired_fields:
            if debug:
                print(f"    Preprocess: empty desired-fields for {symbol_name}")
            return None

        desired_field_list = list(desired_fields)
        if any(not isinstance(field_name, str) or not field_name for field_name in desired_field_list):
            if debug:
                print(f"    Preprocess: invalid desired field list for {symbol_name}")
            return None
        normalized[symbol_name] = desired_field_list

    return normalized
```

- [ ] **Step 4: 运行当前 util 测试，确认 helper 层已编译通过但主流程测试仍未全部通过**

Run:

```bash
uv run python -m unittest \
  tests.test_ida_analyze_util.TestGenerateYamlDesiredFieldsContract.test_preprocess_common_skill_rejects_missing_generate_yaml_desired_fields -v
```

Expected:

```text
FAIL: test_preprocess_common_skill_rejects_missing_generate_yaml_desired_fields
AssertionError: True is not false
```

- [ ] **Step 5: 提交 helper 基础设施**

```bash
git add ida_analyze_util.py
git commit -m "refactor(preprocess): 抽取 yaml 字段顺序与组装工具"
```

### Task 4: 把 `preprocess_common_skill(...)` 改成严格字段契约模型

**Files:**
- Modify: `ida_analyze_util.py`

- [ ] **Step 1: 更新函数签名与文档字符串**

把函数签名改为：

```python
async def preprocess_common_skill(
    session,
    expected_outputs,
    old_yaml_map=None,
    new_binary_dir=None,
    platform="windows",
    image_base=0,
    func_names=None,
    gv_names=None,
    patch_names=None,
    struct_member_names=None,
    vtable_class_names=None,
    inherit_vfuncs=None,
    func_xrefs=None,
    func_vtable_relations=None,
    generate_yaml_desired_fields=None,
    llm_decompile_specs=None,
    llm_config=None,
    mangled_class_names=None,
    debug=False,
):
```

并在文档字符串里把 `func_vtable_relations` 参数说明改成：

```python
- ``func_vtable_relations``: enrich located function YAML with vtable metadata.
  Each element is a tuple of ``(func_name, vtable_class)``.
- ``generate_yaml_desired_fields``: required list of
  ``(symbol_name, desired_field_names)`` tuples that defines the exact YAML
  payload fields to emit per symbol.
```

- [ ] **Step 2: 在入口处规范化并校验字段契约**

在现有 `llm_decompile_specs_map` 构建前后加入：

```python
    desired_fields_map = _normalize_generate_yaml_desired_fields(
        generate_yaml_desired_fields,
        debug=debug,
    )
    if desired_fields_map is None:
        return False
```

接着加入 symbol 类型映射 helper：

```python
def _build_target_kind_map(
    func_names,
    gv_names,
    patch_names,
    struct_member_names,
    vtable_class_names,
    inherit_vfuncs,
    func_xrefs_map,
    debug=False,
):
    target_kind_map = {}

    def _register(symbol_name, target_kind):
        existing_kind = target_kind_map.get(symbol_name)
        if existing_kind is not None and existing_kind != target_kind:
            if debug:
                print(
                    f"    Preprocess: symbol kind conflict for {symbol_name}: "
                    f"{existing_kind} vs {target_kind}"
                )
            return False
        target_kind_map[symbol_name] = target_kind
        return True

    for func_name in list(func_names) + list(func_xrefs_map):
        if not _register(func_name, "func"):
            return None
    for inherit_spec in inherit_vfuncs:
        if not _register(inherit_spec[0], "func"):
            return None
    for gv_name in gv_names:
        if not _register(gv_name, "gv"):
            return None
    for patch_name in patch_names:
        if not _register(patch_name, "patch"):
            return None
    for struct_member_name in struct_member_names:
        if not _register(struct_member_name, "struct_member"):
            return None
    for class_name in vtable_class_names:
        if not _register(class_name, "vtable"):
            return None

    return target_kind_map
```

再在 `preprocess_common_skill(...)` 中调用它，并校验每个 symbol 的字段名都属于合法集合。

- [ ] **Step 3: 把 `FUNC_VTABLE_RELATIONS` 改成二元组并按需计算 vfunc slot**

把当前：

```python
    # Build vtable-relation lookup: func_name -> (vtable_class, generate_vfunc_offset)
    vtable_relations_map = {}
    for spec in func_vtable_relations:
        vtable_relations_map[spec[0]] = (spec[1], spec[2])
```

改成：

```python
    vtable_relations_map = {}
    for spec in func_vtable_relations:
        if not isinstance(spec, (tuple, list)) or len(spec) != 2:
            if debug:
                print(f"    Preprocess: invalid func_vtable_relations spec: {spec}")
            return False
        func_name, vtable_class = spec
        if not isinstance(func_name, str) or not func_name:
            if debug:
                print(f"    Preprocess: invalid func_vtable_relations target: {func_name}")
            return False
        if not isinstance(vtable_class, str) or not vtable_class:
            if debug:
                print(f"    Preprocess: invalid func_vtable_relations class: {vtable_class}")
            return False
        vtable_relations_map[func_name] = vtable_class
```

然后把 enrichment 分支改为“只在字段契约需要时计算”：

```python
        desired_fields = desired_fields_map[func_name]
        desired_field_set = set(desired_fields)

        if func_name in vtable_relations_map and "vtable_name" in desired_field_set:
            func_data["vtable_name"] = vtable_relations_map[func_name]

        need_vfunc_slot = bool({"vfunc_offset", "vfunc_index"} & desired_field_set)
        if need_vfunc_slot:
            vtable_class = vtable_relations_map.get(func_name) or func_data.get("vtable_name")
            if not vtable_class:
                if debug:
                    print(f"    Preprocess: missing vtable relation for {func_name}")
                return False
            func_data["vtable_name"] = vtable_class
            if "vfunc_offset" not in func_data or "vfunc_index" not in func_data:
                func_va_hex = func_data.get("func_va")
                if not func_va_hex:
                    if debug:
                        print(f"    Preprocess: missing func_va for vfunc slot lookup: {func_name}")
                    return False
                try:
                    func_va_int = int(str(func_va_hex), 16)
                except (TypeError, ValueError):
                    if debug:
                        print(f"    Preprocess: invalid func_va for {func_name}: {func_va_hex}")
                    return False

                vtable_data = await preprocess_vtable_via_mcp(
                    session=session,
                    class_name=vtable_class,
                    image_base=image_base,
                    platform=platform,
                    debug=debug,
                    symbol_aliases=_get_mangled_class_aliases(
                        normalized_mangled_class_names,
                        vtable_class,
                    ),
                )
                if vtable_data is None:
                    if debug:
                        print(f"    Preprocess: failed to look up {vtable_class} vtable for {func_name}")
                    return False

                matched_index = None
                for entry_index, entry_va in vtable_data.get("vtable_entries", {}).items():
                    try:
                        if int(str(entry_va), 16) == func_va_int:
                            matched_index = int(entry_index)
                            break
                    except (TypeError, ValueError):
                        continue

                if matched_index is None:
                    if debug:
                        print(f"    Preprocess: {func_name} at {func_va_hex} not found in {vtable_class} vtable entries")
                    return False

                func_data["vfunc_index"] = matched_index
                func_data["vfunc_offset"] = hex(matched_index * 8)
```

- [ ] **Step 4: 在写 YAML 之前统一按字段契约组装 payload**

在 `ida_analyze_util.py` 中加入 helper：

```python
def _assemble_symbol_payload(symbol_name, target_kind, candidate_data, desired_fields_map, debug=False):
    desired_fields = desired_fields_map.get(symbol_name)
    if desired_fields is None:
        if debug:
            print(f"    Preprocess: missing desired-fields entry for {symbol_name}")
        return None

    payload = {}
    for field_name in desired_fields:
        if field_name not in candidate_data:
            if debug:
                print(
                    f"    Preprocess: missing desired field {field_name} "
                    f"for {symbol_name}"
                )
            return None
        payload[field_name] = candidate_data[field_name]

    ordered_keys = TARGET_KIND_TO_FIELD_ORDER[target_kind]
    return _build_ordered_yaml_payload(payload, ordered_keys)
```

然后把每个 writer 调用改成先组装 payload，例如函数分支：

```python
        payload = _assemble_symbol_payload(
            func_name,
            "func",
            func_data,
            desired_fields_map,
            debug=debug,
        )
        if payload is None:
            return False

        await _rename_func_in_ida(session, func_data.get("func_va"), func_name, debug)
        write_func_yaml(target_output, payload)
```

vtable、gv、patch、struct-member 分支也统一按这个模式处理。

- [ ] **Step 5: 运行 util 定向测试，确认新模型通过**

Run:

```bash
uv run python -m unittest \
  tests.test_ida_analyze_util.TestGenerateYamlDesiredFieldsContract \
  tests.test_ida_analyze_util.TestVtableAliasSupport.test_func_vtable_relations_use_aliases_for_index_enrichment \
  tests.test_ida_analyze_util.TestLlmDecompileSupport.test_preprocess_common_skill_uses_slot_only_fallback_when_vtable_unavailable -v
```

Expected:

```text
OK
```

- [ ] **Step 6: 提交公共层行为变更**

```bash
git add ida_analyze_util.py tests/test_ida_analyze_util.py
git commit -m "refactor(preprocess): 收敛字段契约与 vtable 关系"
```

### Task 5: 批量迁移 `ida_preprocessor_scripts/` 到显式字段契约

**Files:**
- Modify: `ida_preprocessor_scripts/*.py`
- Modify: `tests/test_ida_preprocessor_scripts.py`

- [ ] **Step 1: 先生成迁移清单，确认脚本分组**

Run:

```bash
rg -l 'preprocess_common_skill\(' ida_preprocessor_scripts | sort > /tmp/preprocess_common_skill_files.txt
rg -l '^FUNC_VTABLE_RELATIONS\s*=' ida_preprocessor_scripts | sort > /tmp/func_vtable_relation_files.txt
rg -l '^INHERIT_VFUNCS\s*=' ida_preprocessor_scripts | sort > /tmp/inherit_vfunc_files.txt
rg -l '^FUNC_XREFS\s*=' ida_preprocessor_scripts | sort > /tmp/func_xref_files.txt
wc -l /tmp/preprocess_common_skill_files.txt /tmp/func_vtable_relation_files.txt /tmp/inherit_vfunc_files.txt /tmp/func_xref_files.txt
```

Expected:

```text
  215 /tmp/preprocess_common_skill_files.txt
   40 /tmp/func_vtable_relation_files.txt
   15 /tmp/inherit_vfunc_files.txt
   40 /tmp/func_xref_files.txt
```

- [ ] **Step 2: 用一次性源码迁移脚本批量补齐绝大多数脚本**

运行下面的 one-shot 脚本，它会：

- 为没有 `GENERATE_YAML_DESIRED_FIELDS` 的脚本生成默认字段契约
- 把三元组 `FUNC_VTABLE_RELATIONS` 重写成二元组
- 给每个 `preprocess_common_skill(...)` 调用补 `generate_yaml_desired_fields=GENERATE_YAML_DESIRED_FIELDS`
- 对 `INHERIT_VFUNCS` 中 `generate_func_sig=False` 的条目使用 `INHERIT_VFUNC_FIELDS`

Run:

```bash
python - <<'PY'
from __future__ import annotations

import ast
import re
from pathlib import Path

ROOT = Path("ida_preprocessor_scripts")

FUNC_FIELDS = ["func_name", "func_va", "func_rva", "func_size", "func_sig"]
VFUNC_FIELDS = FUNC_FIELDS + ["vtable_name", "vfunc_offset", "vfunc_index"]
INHERIT_VFUNC_FIELDS = [
    "func_name",
    "func_va",
    "func_rva",
    "func_size",
    "vtable_name",
    "vfunc_offset",
    "vfunc_index",
]
GV_FIELDS = [
    "gv_name",
    "gv_va",
    "gv_rva",
    "gv_sig",
    "gv_sig_va",
    "gv_inst_offset",
    "gv_inst_length",
    "gv_inst_disp",
]
VTABLE_FIELDS = [
    "vtable_class",
    "vtable_symbol",
    "vtable_va",
    "vtable_rva",
    "vtable_size",
    "vtable_numvfunc",
    "vtable_entries",
]
PATCH_FIELDS = ["patch_name", "patch_sig", "patch_bytes"]
STRUCT_MEMBER_FIELDS = [
    "struct_name",
    "member_name",
    "offset",
    "size",
    "offset_sig",
    "offset_sig_disp",
]


def read_literal_constants(text: str) -> dict[str, object]:
    tree = ast.parse(text)
    constants: dict[str, object] = {}
    for node in tree.body:
        if not isinstance(node, ast.Assign) or len(node.targets) != 1:
            continue
        target = node.targets[0]
        if not isinstance(target, ast.Name):
            continue
        try:
            constants[target.id] = ast.literal_eval(node.value)
        except Exception:
            continue
    return constants


def format_block(name: str, value: object) -> str:
    return f"{name} = {repr(value)}\n\n"


for path in sorted(ROOT.glob("find-*.py")):
    text = path.read_text(encoding="utf-8")
    if "preprocess_common_skill(" not in text:
        continue
    if "GENERATE_YAML_DESIRED_FIELDS" in text:
        continue

    constants = read_literal_constants(text)
    desired_fields_map: dict[str, list[str]] = {}

    for func_name in constants.get("TARGET_FUNCTION_NAMES", []):
        desired_fields_map[func_name] = list(FUNC_FIELDS)
    for func_name, *_rest in constants.get("INHERIT_VFUNCS", []):
        generate_func_sig = _rest[2] if len(_rest) >= 3 else True
        desired_fields_map[func_name] = list(
            VFUNC_FIELDS if generate_func_sig else INHERIT_VFUNC_FIELDS
        )
    for func_name, _vtable_class, *_ in constants.get("FUNC_VTABLE_RELATIONS", []):
        desired_fields_map[func_name] = list(VFUNC_FIELDS)
    for gv_name in constants.get("TARGET_GLOBALVAR_NAMES", []):
        desired_fields_map[gv_name] = list(GV_FIELDS)
    for class_name in constants.get("TARGET_CLASS_NAMES", []):
        desired_fields_map[class_name] = list(VTABLE_FIELDS)
    for patch_name in constants.get("TARGET_PATCH_NAMES", []):
        desired_fields_map[patch_name] = list(PATCH_FIELDS)
    for struct_member_name in constants.get("TARGET_STRUCT_MEMBER_NAMES", []):
        desired_fields_map[struct_member_name] = list(STRUCT_MEMBER_FIELDS)

    desired_fields = [(symbol_name, fields) for symbol_name, fields in desired_fields_map.items()]
    if not desired_fields:
        raise SystemExit(f"no desired-field mapping derived for {path}")

    if "FUNC_VTABLE_RELATIONS =" in text:
        old_relations = constants.get("FUNC_VTABLE_RELATIONS", [])
        new_relations = [(func_name, vtable_class) for func_name, vtable_class, *_ in old_relations]
        text = re.sub(
            r"FUNC_VTABLE_RELATIONS\s*=\s*\[(?:.|\n)*?\n\]",
            format_block("FUNC_VTABLE_RELATIONS", new_relations).rstrip(),
            text,
            count=1,
        )
        text = text.replace(
            "# (func_name, vtable_class, generate_vfunc_offset)",
            "# (func_name, vtable_class)",
        )

    generate_block = format_block("GENERATE_YAML_DESIRED_FIELDS", desired_fields)
    text = text.replace("async def preprocess_skill(", generate_block + "async def preprocess_skill(", 1)
    text = text.replace(
        "        debug=debug,\n    )",
        "        generate_yaml_desired_fields=GENERATE_YAML_DESIRED_FIELDS,\n"
        "        debug=debug,\n"
        "    )",
        1,
    )

    path.write_text(text, encoding="utf-8")
PY
```

- [ ] **Step 3: 手工修正特殊 vfunc 脚本与断言**

先修 `ida_preprocessor_scripts/find-INetworkMessages_FindNetworkGroup.py`，把关键片段改成：

```python
FUNC_VTABLE_RELATIONS = [
    ("INetworkMessages_FindNetworkGroup", "INetworkMessages"),
]

GENERATE_YAML_DESIRED_FIELDS = [
    (
        "INetworkMessages_FindNetworkGroup",
        ["func_name", "vfunc_sig", "vfunc_offset", "vfunc_index", "vtable_name"],
    ),
]
```

并确保调用为：

```python
    return await preprocess_common_skill(
        session=session,
        expected_outputs=expected_outputs,
        old_yaml_map=old_yaml_map,
        new_binary_dir=new_binary_dir,
        platform=platform,
        image_base=image_base,
        func_names=TARGET_FUNCTION_NAMES,
        func_vtable_relations=FUNC_VTABLE_RELATIONS,
        llm_decompile_specs=LLM_DECOMPILE,
        generate_yaml_desired_fields=GENERATE_YAML_DESIRED_FIELDS,
        llm_config=llm_config,
        debug=debug,
    )
```

然后修 `ida_preprocessor_scripts/find-CNetworkMessages_FindNetworkGroup.py`，把关键片段改成：

```python
FUNC_VTABLE_RELATIONS = [
    ("CNetworkMessages_FindNetworkGroup", "CNetworkMessages"),
]

GENERATE_YAML_DESIRED_FIELDS = [
    (
        "CNetworkMessages_FindNetworkGroup",
        [
            "func_name",
            "func_va",
            "func_rva",
            "func_size",
            "func_sig",
            "vtable_name",
            "vfunc_offset",
            "vfunc_index",
        ],
    ),
]
```

- [ ] **Step 4: 运行脚本层定向回归，确认迁移生效**

Run:

```bash
uv run python -m unittest \
  tests.test_ida_preprocessor_scripts.TestFindCNetworkMessagesFindNetworkGroup \
  tests.test_ida_preprocessor_scripts.TestFindINetworkMessagesFindNetworkGroup \
  tests.test_ida_preprocessor_scripts.TestFindCBaseEntityCollisionRulesChanged -v
```

Expected:

```text
OK
```

- [ ] **Step 5: 提交脚本层全量迁移**

```bash
git add ida_preprocessor_scripts tests/test_ida_preprocessor_scripts.py
git commit -m "refactor(preprocess): 全量声明脚本输出字段"
```

### Task 6: 做收尾验证与语义清扫

**Files:**
- Modify: `ida_analyze_util.py`
- Modify: `tests/test_ida_analyze_util.py`
- Modify: `tests/test_ida_preprocessor_scripts.py`
- Modify: `ida_preprocessor_scripts/*.py`

- [ ] **Step 1: 运行 util 与 script 的核心回归集**

Run:

```bash
uv run python -m unittest \
  tests.test_ida_analyze_util \
  tests.test_ida_preprocessor_scripts -v
```

Expected:

```text
OK
```

- [ ] **Step 2: 确认仓库里不再残留 `generate_vfunc_offset`**

Run:

```bash
rg -n 'generate_vfunc_offset' ida_analyze_util.py ida_preprocessor_scripts tests
```

Expected:

```text
[no output]
```

- [ ] **Step 3: 确认所有 `preprocess_common_skill(...)` 脚本都声明了字段契约**

Run:

```bash
while IFS= read -r file; do
  rg -q '^GENERATE_YAML_DESIRED_FIELDS\\s*=' "$file" || echo "missing desired fields: $file"
done < <(rg -l 'preprocess_common_skill\(' ida_preprocessor_scripts | sort)
```

Expected:

```text
[no output]
```

- [ ] **Step 4: 确认目标 spec 中列出的特殊脚本 shape 没被回归破坏**

Run:

```bash
rg -n 'INetworkMessages_FindNetworkGroup|GENERATE_YAML_DESIRED_FIELDS|FUNC_VTABLE_RELATIONS' \
  ida_preprocessor_scripts/find-INetworkMessages_FindNetworkGroup.py \
  ida_preprocessor_scripts/find-CNetworkMessages_FindNetworkGroup.py
```

Expected:

```text
输出中至少各包含一条来自以下两个文件的匹配行：
ida_preprocessor_scripts/find-INetworkMessages_FindNetworkGroup.py
ida_preprocessor_scripts/find-CNetworkMessages_FindNetworkGroup.py
```

两份脚本都应同时包含：

- `("INetworkMessages_FindNetworkGroup", "INetworkMessages")`
- `("CNetworkMessages_FindNetworkGroup", "CNetworkMessages")`
- `GENERATE_YAML_DESIRED_FIELDS = [`
- `generate_yaml_desired_fields=GENERATE_YAML_DESIRED_FIELDS`

- [ ] **Step 5: 提交最终验证通过的实现**

```bash
git add ida_analyze_util.py tests/test_ida_analyze_util.py tests/test_ida_preprocessor_scripts.py ida_preprocessor_scripts
git commit -m "fix(preprocess): 强制按字段契约生成 yaml"
```
