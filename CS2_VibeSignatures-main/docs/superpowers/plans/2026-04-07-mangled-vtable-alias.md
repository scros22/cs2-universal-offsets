# Mangled Vtable Alias Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [x]`) syntax for tracking.

**Goal:** 为 `preprocess_common_skill` / `preprocess_vtable_via_mcp` 增加 `mangled_class_names` alias 支持，并接入 `find-CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_vtable`。

**Architecture:** 先用单元测试锁定三条 alias 传播链路：直接 vtable 生成、`preprocess_func_sig_via_mcp()` 的缺失 YAML 回填、`func_vtable_relations` 的 vtable enrich。随后在 `ida_analyze_util.py` 中加入 alias 校验 helper、扩展 py_eval 模板和公共入口透传逻辑，最后补新脚本与 `config.yaml` 注册并做静态核对。

**Tech Stack:** Python 3、`unittest`、PyYAML、IDA MCP `py_eval`、`rg`

---

## File Structure

- Modify: `ida_analyze_util.py`
  - 新增 `mangled_class_names` 校验与别名提取 helper
  - 扩展 `_VTABLE_PY_EVAL_TEMPLATE` / `_build_vtable_py_eval(...)`
  - 扩展 `preprocess_vtable_via_mcp(...)`
  - 扩展 `preprocess_func_sig_via_mcp(...)` 的 `_load_vtable_data(...)` 别名透传
  - 扩展 `preprocess_common_skill(...)` 的参数校验与 `func_vtable_relations` 透传
- Modify: `tests/test_ida_analyze_util.py`
  - 为 py_eval builder、公共 skill 透传、缺失 vtable YAML 回填、`func_vtable_relations` enrich 增加单元测试
- Modify: `tests/test_ida_preprocessor_scripts.py`
  - 为新脚本增加“正确转发 `vtable_class_names` 与 `mangled_class_names`”测试
- Create: `ida_preprocessor_scripts/find-CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_vtable.py`
  - 新增首个使用 `MANGLED_CLASS_NAMES` 的 vtable 预处理脚本
- Modify: `config.yaml`
  - 在 `server` 模块 `skills` 中注册新 skill
  - 在 `server` 模块 `symbols` 中注册新 vtable symbol

**仓库约束：** 当前会话默认不执行 `git commit`。本计划用“定向测试 + `git diff --stat` 检查”替代提交步骤；除非用户明确要求，否则实施阶段不要提交。

### Task 1: 先为 alias 传播链路补 failing tests

**Files:**
- Modify: `tests/test_ida_analyze_util.py`

- [x] **Step 1: 在 `tests/test_ida_analyze_util.py` 追加 alias 相关测试**

先把导入行：

```python
from unittest.mock import AsyncMock
```

改成：

```python
from unittest.mock import AsyncMock, patch
```

再在文件末尾、`if __name__ == "__main__":` 之前加入下面的测试类和辅助断言：

```python
class TestVtableAliasSupport(unittest.IsolatedAsyncioTestCase):
    async def test_preprocess_common_skill_rejects_invalid_mangled_class_names(self) -> None:
        result = await ida_analyze_util.preprocess_common_skill(
            session="session",
            expected_outputs=["/tmp/Foo_vtable.windows.yaml"],
            vtable_class_names=["Foo"],
            mangled_class_names=["bad-config"],
            platform="windows",
            image_base=0x180000000,
            debug=False,
        )

        self.assertFalse(result)

    def test_build_vtable_py_eval_embeds_candidate_symbols(self) -> None:
        py_code = ida_analyze_util._build_vtable_py_eval(
            "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem",
            [
                "??_7?$CGameSystemReallocatingFactory@VCSpawnGroupMgrGameSystem@@V1@@@6B@",
                "_ZTV30CGameSystemReallocatingFactoryI24CSpawnGroupMgrGameSystemS0_E",
            ],
        )

        self.assertIn(
            '"CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem"',
            py_code,
        )
        self.assertIn("candidate_symbols = [", py_code)
        self.assertIn(
            "??_7?$CGameSystemReallocatingFactory@VCSpawnGroupMgrGameSystem@@V1@@@6B@",
            py_code,
        )
        self.assertIn(
            "_ZTV30CGameSystemReallocatingFactoryI24CSpawnGroupMgrGameSystemS0_E",
            py_code,
        )

    async def test_preprocess_common_skill_passes_aliases_to_vtable_lookup(self) -> None:
        alias_map = {
            "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem": [
                "??_7?$CGameSystemReallocatingFactory@VCSpawnGroupMgrGameSystem@@V1@@@6B@",
                "_ZTV30CGameSystemReallocatingFactoryI24CSpawnGroupMgrGameSystemS0_E",
            ]
        }
        fake_vtable_data = {
            "vtable_class": "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem",
            "vtable_symbol": "??_7?$CGameSystemReallocatingFactory@VCSpawnGroupMgrGameSystem@@V1@@@6B@",
            "vtable_va": "0x180010000",
            "vtable_rva": "0x10000",
            "vtable_size": "0x20",
            "vtable_numvfunc": 4,
            "vtable_entries": {0: "0x180020000"},
        }

        with patch.object(
            ida_analyze_util,
            "preprocess_vtable_via_mcp",
            AsyncMock(return_value=fake_vtable_data),
        ) as mock_preprocess_vtable, patch.object(
            ida_analyze_util,
            "write_vtable_yaml",
        ) as mock_write_vtable_yaml:
            result = await ida_analyze_util.preprocess_common_skill(
                session="session",
                expected_outputs=[
                    "/tmp/CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_vtable.windows.yaml"
                ],
                vtable_class_names=[
                    "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem"
                ],
                mangled_class_names=alias_map,
                platform="windows",
                image_base=0x180000000,
                debug=True,
            )

        self.assertTrue(result)
        mock_preprocess_vtable.assert_awaited_once_with(
            session="session",
            class_name="CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem",
            image_base=0x180000000,
            platform="windows",
            debug=True,
            symbol_aliases=alias_map[
                "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem"
            ],
        )
        mock_write_vtable_yaml.assert_called_once()

    async def test_preprocess_func_sig_uses_aliases_when_generating_missing_vtable_yaml(self) -> None:
        alias_map = {
            "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem": [
                "??_7?$CGameSystemReallocatingFactory@VCSpawnGroupMgrGameSystem@@V1@@@6B@",
                "_ZTV30CGameSystemReallocatingFactoryI24CSpawnGroupMgrGameSystemS0_E",
            ]
        }

        with tempfile.TemporaryDirectory() as temp_dir:
            module_dir = Path(temp_dir) / "bin" / "14141" / "server"
            old_dir = Path(temp_dir) / "old"
            new_path = module_dir / "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_Think.windows.yaml"
            old_path = old_dir / "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_Think.windows.yaml"

            old_dir.mkdir(parents=True, exist_ok=True)
            module_dir.mkdir(parents=True, exist_ok=True)
            old_path.write_text(
                yaml.safe_dump(
                    {
                        "vtable_name": "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem",
                        "vfunc_sig": "AA BB CC DD",
                        "vfunc_index": 1,
                    },
                    sort_keys=False,
                ),
                encoding="utf-8",
            )

            session = AsyncMock()
            session.call_tool.side_effect = [
                _FakeCallToolResult(
                    [{"matches": ["0x180003000"], "n": 1}]
                ),
                _py_eval_payload(
                    {
                        "func_va": "0x180004000",
                        "func_size": "0x40",
                    }
                ),
            ]

            with patch.object(
                ida_analyze_util,
                "preprocess_vtable_via_mcp",
                AsyncMock(
                    return_value={
                        "vtable_class": "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem",
                        "vtable_symbol": "_ZTV30CGameSystemReallocatingFactoryI24CSpawnGroupMgrGameSystemS0_E + 0x10",
                        "vtable_va": "0x180001000",
                        "vtable_rva": "0x1000",
                        "vtable_size": "0x10",
                        "vtable_numvfunc": 2,
                        "vtable_entries": {
                            0: "0x180002000",
                            1: "0x180004000",
                        },
                    }
                ),
            ) as mock_preprocess_vtable:
                result = await ida_analyze_util.preprocess_func_sig_via_mcp(
                    session=session,
                    new_path=str(new_path),
                    old_path=str(old_path),
                    image_base=0x180000000,
                    new_binary_dir=str(module_dir),
                    platform="windows",
                    func_name="CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_Think",
                    debug=True,
                    mangled_class_names=alias_map,
                )

        self.assertIsNotNone(result)
        mock_preprocess_vtable.assert_awaited_once_with(
            session,
            "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem",
            0x180000000,
            "windows",
            True,
            alias_map[
                "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem"
            ],
        )

    async def test_func_vtable_relations_use_aliases_for_index_enrichment(self) -> None:
        alias_map = {
            "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem": [
                "??_7?$CGameSystemReallocatingFactory@VCSpawnGroupMgrGameSystem@@V1@@@6B@",
                "_ZTV30CGameSystemReallocatingFactoryI24CSpawnGroupMgrGameSystemS0_E",
            ]
        }

        with patch.object(
            ida_analyze_util,
            "preprocess_func_sig_via_mcp",
            AsyncMock(
                return_value={
                    "func_name": "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_Think",
                    "func_va": "0x180004000",
                    "func_rva": "0x4000",
                    "func_size": "0x40",
                    "func_sig": "AA BB",
                }
            ),
        ) as mock_preprocess_func, patch.object(
            ida_analyze_util,
            "preprocess_vtable_via_mcp",
            AsyncMock(
                return_value={
                    "vtable_class": "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem",
                    "vtable_symbol": "??_7?$CGameSystemReallocatingFactory@VCSpawnGroupMgrGameSystem@@V1@@@6B@",
                    "vtable_va": "0x180001000",
                    "vtable_rva": "0x1000",
                    "vtable_size": "0x20",
                    "vtable_numvfunc": 4,
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
                expected_outputs=[
                    "/tmp/CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_Think.windows.yaml"
                ],
                old_yaml_map={},
                new_binary_dir="/tmp",
                platform="windows",
                image_base=0x180000000,
                func_names=[
                    "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_Think"
                ],
                func_vtable_relations=[
                    (
                        "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_Think",
                        "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem",
                        True,
                    )
                ],
                mangled_class_names=alias_map,
                debug=True,
            )

        self.assertTrue(result)
        mock_preprocess_func.assert_awaited_once()
        mock_preprocess_vtable.assert_awaited_once_with(
            session="session",
            class_name="CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem",
            image_base=0x180000000,
            platform="windows",
            debug=True,
            symbol_aliases=alias_map[
                "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem"
            ],
        )
        written_payload = mock_write_func_yaml.call_args.args[1]
        self.assertEqual(1, written_payload["vfunc_index"])
        self.assertEqual("0x8", written_payload["vfunc_offset"])
```

- [x] **Step 2: 运行定向单元测试并确认它们先失败**

Run:

```bash
python -m unittest tests.test_ida_analyze_util -v
```

Expected: 新增测试失败，报错集中在以下一种或多种情况：

- `preprocess_common_skill()` 还不会拒绝非法 `mangled_class_names`
- `_build_vtable_py_eval()` 还不接受 alias 列表
- `preprocess_common_skill()` 还不接受 `mangled_class_names`
- `preprocess_func_sig_via_mcp()` 还不接受 `mangled_class_names`
- `preprocess_vtable_via_mcp()` 还没有 `symbol_aliases`

### Task 2: 在 `ida_analyze_util.py` 实现 alias 解析与透传

**Files:**
- Modify: `ida_analyze_util.py`

- [x] **Step 1: 新增 alias 校验与读取 helper**

在 `parse_mcp_result(...)` 后、`build_remote_text_export_py_eval(...)` 前加入下面两个 helper：

```python
def _normalize_mangled_class_names(mangled_class_names, debug=False):
    if mangled_class_names is None:
        return {}
    if not isinstance(mangled_class_names, dict):
        if debug:
            print(
                "    Preprocess: mangled_class_names must be a dict, got "
                f"{type(mangled_class_names).__name__}"
            )
        return None

    normalized = {}
    for class_name, aliases in mangled_class_names.items():
        if not isinstance(class_name, str) or not class_name:
            if debug:
                print(
                    "    Preprocess: invalid mangled_class_names key: "
                    f"{class_name!r}"
                )
            return None
        if not isinstance(aliases, (list, tuple)):
            if debug:
                print(
                    "    Preprocess: aliases for "
                    f"{class_name} must be a list/tuple"
                )
            return None

        normalized_aliases = []
        for alias in aliases:
            if not isinstance(alias, str) or not alias:
                if debug:
                    print(
                        "    Preprocess: invalid alias for "
                        f"{class_name}: {alias!r}"
                    )
                return None
            normalized_aliases.append(alias)

        normalized[class_name] = normalized_aliases

    return normalized


def _get_mangled_class_aliases(mangled_class_names, class_name):
    aliases = (mangled_class_names or {}).get(class_name, [])
    return list(aliases)
```

- [x] **Step 2: 扩展 `_VTABLE_PY_EVAL_TEMPLATE`、`_build_vtable_py_eval(...)` 与 `preprocess_vtable_via_mcp(...)`**

把顶部模板和 builder 改成下面的结构，重点是同时注入 `class_name` 与 `candidate_symbols`：

```python
_VTABLE_PY_EVAL_TEMPLATE = r'''
import ida_bytes, ida_name, idaapi, idautils, ida_segment, json

class_name = CLASS_NAME_PLACEHOLDER
candidate_symbols = CANDIDATE_SYMBOLS_PLACEHOLDER
ptr_size = 8 if idaapi.inf_is_64bit() else 4

vtable_start = None
vtable_symbol = ""
is_linux = False

def _try_direct_symbol(symbol_name):
    global vtable_start, vtable_symbol, is_linux
    if not symbol_name:
        return False
    addr = ida_name.get_name_ea(idaapi.BADADDR, symbol_name)
    if addr == idaapi.BADADDR:
        return False
    if symbol_name.startswith("_ZTV"):
        vtable_start = addr + 0x10
        vtable_symbol = symbol_name + " + 0x10"
        is_linux = True
    else:
        vtable_start = addr
        vtable_symbol = symbol_name
        is_linux = False
    return True

for symbol_name in candidate_symbols:
    if _try_direct_symbol(symbol_name):
        break

if vtable_start is None:
    win_name = "??_7" + class_name + "@@6B@"
    _try_direct_symbol(win_name)

if vtable_start is None:
    linux_name = "_ZTV" + str(len(class_name)) + class_name
    _try_direct_symbol(linux_name)

# 下面保留原有 RTTI / typeinfo fallback 与 entries 解析逻辑
'''


def _build_vtable_py_eval(class_name, symbol_aliases=None):
    return (
        _VTABLE_PY_EVAL_TEMPLATE
        .replace("CLASS_NAME_PLACEHOLDER", json.dumps(class_name))
        .replace(
            "CANDIDATE_SYMBOLS_PLACEHOLDER",
            json.dumps(list(symbol_aliases or [])),
        )
    )


async def preprocess_vtable_via_mcp(
    session,
    class_name,
    image_base,
    platform,
    debug=False,
    symbol_aliases=None,
):
    _ = platform
    py_code = _build_vtable_py_eval(class_name, symbol_aliases=symbol_aliases)
```

要求：

- `symbol_aliases` 放在 `debug=False` 后面，保留现有五参位置调用兼容性
- Linux `_ZTV...` alias 命中时继续保留 `+0x10`
- RTTI fallback 与返回 payload 字段顺序不变

- [x] **Step 3: 扩展 `preprocess_func_sig_via_mcp(...)` 与 `preprocess_common_skill(...)`**

把 `preprocess_func_sig_via_mcp(...)` 签名扩成：

```python
async def preprocess_func_sig_via_mcp(
    session,
    new_path,
    old_path,
    image_base,
    new_binary_dir,
    platform,
    func_name=None,
    debug=False,
    mangled_class_names=None,
):
```

并把内部 `_load_vtable_data(...)` 改成：

```python
    normalized_mangled_class_names = _normalize_mangled_class_names(
        mangled_class_names,
        debug=debug,
    )
    if normalized_mangled_class_names is None:
        return None

    async def _load_vtable_data(vtable_name):
        vtable_yaml_path = os.path.join(
            new_binary_dir,
            f"{vtable_name}_vtable.{platform}.yaml"
        )

        if not os.path.exists(vtable_yaml_path):
            vtable_gen_data = await preprocess_vtable_via_mcp(
                session,
                vtable_name,
                image_base,
                platform,
                debug,
                _get_mangled_class_aliases(
                    normalized_mangled_class_names,
                    vtable_name,
                ),
            )
            if vtable_gen_data is None:
                if debug:
                    print(
                        "    Preprocess: vtable YAML not found and generation failed: "
                        f"{os.path.basename(vtable_yaml_path)}"
                    )
                return None
            write_vtable_yaml(vtable_yaml_path, vtable_gen_data)
```

再把 `preprocess_common_skill(...)` 的签名和关键分支改成：

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
    mangled_class_names=None,
    debug=False,
):
    ...
    normalized_mangled_class_names = _normalize_mangled_class_names(
        mangled_class_names,
        debug=debug,
    )
    if normalized_mangled_class_names is None:
        return False
```

直接 vtable 分支：

```python
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
```

`preprocess_func_sig_via_mcp(...)` 调用分支：

```python
        func_data = await preprocess_func_sig_via_mcp(
            session=session,
            new_path=target_output,
            old_path=old_path,
            image_base=image_base,
            new_binary_dir=new_binary_dir,
            platform=platform,
            func_name=func_name,
            debug=debug,
            mangled_class_names=normalized_mangled_class_names,
        )
```

`func_vtable_relations` enrich 分支：

```python
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
```

要求：

- `mangled_class_names` 非法时在 `debug=True` 下报错并整体失败
- alias 全未命中时继续走自动推导名与 RTTI fallback，不要提前失败
- 不修改 YAML schema

- [x] **Step 4: 重新运行 `ida_analyze_util` 定向测试并确认通过**

Run:

```bash
python -m unittest tests.test_ida_analyze_util -v
```

Expected: `tests.test_ida_analyze_util` 全部 PASS，包括新增的 4 个 alias 相关测试。

### Task 3: 新增 vtable skill 脚本并补脚本层测试

**Files:**
- Modify: `tests/test_ida_preprocessor_scripts.py`
- Create: `ida_preprocessor_scripts/find-CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_vtable.py`

- [x] **Step 1: 先在 `tests/test_ida_preprocessor_scripts.py` 添加 failing test**

在文件顶部路径常量区追加：

```python
REALLOCATING_FACTORY_SCRIPT_PATH = Path(
    "ida_preprocessor_scripts/"
    "find-CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_vtable.py"
)
```

在两个现有测试类后追加：

```python
class TestFindCGameSystemReallocatingFactoryCSpawnGroupMgrGameSystemVtable(
    unittest.IsolatedAsyncioTestCase
):
    async def test_preprocess_skill_forwards_expected_vtable_and_aliases(self) -> None:
        module = _load_module(
            REALLOCATING_FACTORY_SCRIPT_PATH,
            "find_CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_vtable",
        )
        mock_preprocess_common_skill = AsyncMock(return_value=True)
        expected_vtable_class_names = [
            "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem"
        ]
        expected_mangled_class_names = {
            "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem": [
                "??_7?$CGameSystemReallocatingFactory@VCSpawnGroupMgrGameSystem@@V1@@@6B@",
                "_ZTV30CGameSystemReallocatingFactoryI24CSpawnGroupMgrGameSystemS0_E",
            ]
        }

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
            vtable_class_names=expected_vtable_class_names,
            mangled_class_names=expected_mangled_class_names,
            platform="windows",
            image_base=0x180000000,
            debug=True,
        )
```

- [x] **Step 2: 运行脚本层测试并确认它先失败**

Run:

```bash
python -m unittest tests.test_ida_preprocessor_scripts -v
```

Expected: 新增测试失败，原因是脚本文件尚不存在。

- [x] **Step 3: 创建新脚本文件**

创建 `ida_preprocessor_scripts/find-CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_vtable.py`，内容直接使用下面这份：

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
    """Generate CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem vtable YAML by class-name lookup via MCP."""
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

- [x] **Step 4: 重新运行脚本层测试并确认通过**

Run:

```bash
python -m unittest tests.test_ida_preprocessor_scripts -v
```

Expected: `tests.test_ida_preprocessor_scripts` 全部 PASS，包括新加的脚本转发测试。

### Task 4: 注册 `config.yaml` 并做最终定向核对

**Files:**
- Modify: `config.yaml`

- [x] **Step 1: 在 `server` 模块注册新 skill 与 symbol**

在 `config.yaml` 的 `server.skills` 里、紧跟 `find-CSpawnGroupMgrGameSystem_vtable` 后插入：

```yaml
      - name: find-CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_vtable
        expected_output:
          - CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_vtable.{platform}.yaml
```

在 `server.symbols` 的 vtable 区域、紧跟 `CSpawnGroupMgrGameSystem_vtable` 后插入：

```yaml
      - name: CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_vtable
        category: vtable
```

- [x] **Step 2: 用静态命令核对新增注册点和调用点**

Run:

```bash
rg -n "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_vtable|mangled_class_names|symbol_aliases" \
  ida_analyze_util.py \
  ida_preprocessor_scripts/find-CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_vtable.py \
  tests/test_ida_analyze_util.py \
  tests/test_ida_preprocessor_scripts.py \
  config.yaml
```

Expected:

- `ida_analyze_util.py` 命中 `mangled_class_names`、`symbol_aliases` 相关定义与透传点
- 新脚本文件命中新 skill 名、`TARGET_CLASS_NAMES`、`MANGLED_CLASS_NAMES`
- 两个测试文件都命中新增长测试
- `config.yaml` 同时命中新 skill 和新 symbol

- [x] **Step 3: 运行最终定向测试组合**

Run:

```bash
python -m unittest \
  tests.test_ida_analyze_util \
  tests.test_ida_preprocessor_scripts \
  -v
```

Expected: 以上两个测试模块全部 PASS。

- [x] **Step 4: 做一次 diff checkpoint，确认改动面收敛**

Run:

```bash
git diff --stat -- \
  ida_analyze_util.py \
  tests/test_ida_analyze_util.py \
  tests/test_ida_preprocessor_scripts.py \
  ida_preprocessor_scripts/find-CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_vtable.py \
  config.yaml
```

Expected: 只出现上述 5 个文件，且没有额外无关改动。

## Self-Review

### Spec Coverage

- `mangled_class_names` 新接口：Task 2 Step 1 / Step 3
- `preprocess_vtable_via_mcp(...)` 显式 alias 优先：Task 1 Step 1、Task 2 Step 2
- `_load_vtable_data(...)` 缺失 YAML 回填复用 alias：Task 1 Step 1、Task 2 Step 3
- `func_vtable_relations` enrich 复用 alias：Task 1 Step 1、Task 2 Step 3
- 新 skill 脚本：Task 3 Step 3
- `config.yaml` 注册：Task 4 Step 1
- 保持 YAML schema 不变：Task 2 Step 2 / Step 3 明确约束

### Placeholder Scan

- 本计划没有未定占位词或“以后再补”的描述
- 每个代码修改步骤都给出可直接落地的代码块
- 每个验证步骤都给出明确命令与期望结果

### Type Consistency

- 统一使用 `mangled_class_names: dict[str, list[str]]`
- 统一使用 `symbol_aliases` 作为 `preprocess_vtable_via_mcp(...)` 的显式候选参数名
- 统一使用 `CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem` 作为规范类名与产物命名
