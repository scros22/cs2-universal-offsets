# DEFINE_INPUTFUNC Helper Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为 `ShowHudHint` 增加基于 `DEFINE_INPUTFUNC` 数据描述符的脚本化 IDA 预处理路径。

**Architecture:** 新增 `ida_preprocessor_scripts/_define_inputfunc.py` 作为单字符串单 handler 的共享 helper，负责精确字符串查找、数据段 xref 过滤、`xref_from + 0x10` handler 指针读取、`.text` 段校验、签名生成和 YAML 写出。将 `ida_preprocessor_scripts/find-ShowHudHint.py` 改为薄配置脚本，只声明常量并转调 helper。

**Tech Stack:** Python 3、`unittest`、`unittest.mock.AsyncMock`、IDA MCP `py_eval` / `rename`、PyYAML 写出工具、`rg`

---

## File Structure

- Create: `ida_preprocessor_scripts/_define_inputfunc.py`
  - 导出 `preprocess_define_inputfunc_skill(...)`
  - 内部包含 `_build_define_inputfunc_py_eval(...)`、`_collect_define_inputfunc_candidates(...)`、字段/输出路径规范化、函数信息查询、payload 构造和 best-effort rename
- Modify: `ida_preprocessor_scripts/find-ShowHudHint.py`
  - 删除 `preprocess_common_skill` 路径
  - 声明 `TARGET_NAME`、`INPUT_NAME`、`HANDLER_PTR_OFFSET`、`ALLOWED_SEGMENT_NAMES`、`RENAME_TO`
  - `preprocess_skill(...)` 转调 `preprocess_define_inputfunc_skill(...)`
- Create: `tests/test_define_inputfunc_preprocessor.py`
  - 覆盖 py_eval builder、候选解析、`.text` handler 过滤、成功写 YAML、非 `.text` handler 失败
- Modify: `tests/test_ida_preprocessor_scripts.py`
  - 增加 `find-ShowHudHint.py` 薄脚本转发契约测试
- No change: `config.yaml`
  - 当前已有 `find-ShowHudHint` 与 `ShowHudHint.{platform}.yaml` 配置，不需要修改

**仓库约束：** 当前会话默认不执行 `git commit`，且不默认主动运行 test/build。计划中列出的 `pytest` 命令用于获得用户明确许可后的定向验证；若未获得许可，执行阶段只做静态核对和 `git diff` 检查。

### Task 1: Add DEFINE_INPUTFUNC helper tests

**Files:**
- Create: `tests/test_define_inputfunc_preprocessor.py`

- [ ] **Step 1: Create the test module scaffold and builder tests**

Create `tests/test_define_inputfunc_preprocessor.py` with this initial content:

```python
import json
import unittest
from unittest.mock import AsyncMock, patch

from ida_preprocessor_scripts import _define_inputfunc as define_inputfunc


class _FakeTextContent:
    def __init__(self, text: str) -> None:
        self.text = text


class _FakeCallToolResult:
    def __init__(self, payload: dict[str, object]) -> None:
        self.content = [_FakeTextContent(json.dumps(payload))]


def _py_eval_payload(payload: object) -> _FakeCallToolResult:
    return _FakeCallToolResult(
        {
            "result": json.dumps(payload),
            "stdout": "",
            "stderr": "",
        }
    )


class TestBuildDefineInputFuncPyEval(unittest.TestCase):
    def test_build_define_inputfunc_py_eval_embeds_exact_string_offset_and_text_filter(
        self,
    ) -> None:
        code = define_inputfunc._build_define_inputfunc_py_eval(
            input_name="ShowHudHint",
            handler_ptr_offset=0x10,
            allowed_segment_names=(".data",),
        )

        self.assertIn("ShowHudHint", code)
        self.assertIn("handler_ptr_offset = 16", code)
        self.assertIn("allowed_segment_names", code)
        self.assertIn("idautils.Strings", code)
        self.assertIn("idautils.XrefsTo", code)
        self.assertIn("idaapi.getseg", code)
        self.assertIn("ida_bytes.get_qword", code)
        self.assertIn("handler_seg_name == '.text'", code)
        compile(code, "<define_inputfunc_py_eval>", "exec")

    def test_build_define_inputfunc_py_eval_embeds_custom_segment_names(self) -> None:
        code = define_inputfunc._build_define_inputfunc_py_eval(
            input_name="CustomInput",
            handler_ptr_offset=0x18,
            allowed_segment_names=(".data", ".data.rel.ro"),
        )

        self.assertIn("CustomInput", code)
        self.assertIn("handler_ptr_offset = 24", code)
        self.assertIn(".data.rel.ro", code)
        compile(code, "<define_inputfunc_py_eval_custom>", "exec")
```

- [ ] **Step 2: Add collector tests to the same file**

Append this code to `tests/test_define_inputfunc_preprocessor.py`:

```python
class TestCollectDefineInputFuncCandidates(unittest.IsolatedAsyncioTestCase):
    async def test_collect_define_inputfunc_candidates_uses_py_eval_and_returns_candidates(
        self,
    ) -> None:
        session = AsyncMock()
        session.call_tool.return_value = _py_eval_payload(
            {
                "ok": True,
                "string_eas": ["0x180800000"],
                "items": [
                    {
                        "string_ea": "0x180800000",
                        "xref_from": "0x180900000",
                        "xref_seg_name": ".data",
                        "handler_ptr_ea": "0x180900010",
                        "handler_va": "0x180123450",
                        "handler_seg_name": ".text",
                    }
                ],
            }
        )

        result = await define_inputfunc._collect_define_inputfunc_candidates(
            session=session,
            input_name="ShowHudHint",
            handler_ptr_offset=0x10,
            allowed_segment_names=(".data",),
            debug=True,
        )

        self.assertEqual(
            {
                "string_eas": ["0x180800000"],
                "items": [
                    {
                        "string_ea": "0x180800000",
                        "xref_from": "0x180900000",
                        "xref_seg_name": ".data",
                        "handler_ptr_ea": "0x180900010",
                        "handler_va": "0x180123450",
                        "handler_seg_name": ".text",
                    }
                ],
            },
            result,
        )
        code = session.call_tool.await_args.kwargs["arguments"]["code"]
        self.assertIn("ShowHudHint", code)
        self.assertIn("handler_ptr_offset = 16", code)

    async def test_collect_define_inputfunc_candidates_returns_none_on_invalid_payload(
        self,
    ) -> None:
        session = AsyncMock()
        session.call_tool.return_value = _py_eval_payload({"unexpected": []})

        result = await define_inputfunc._collect_define_inputfunc_candidates(
            session=session,
            input_name="ShowHudHint",
            handler_ptr_offset=0x10,
            allowed_segment_names=(".data",),
            debug=True,
        )

        self.assertIsNone(result)

    async def test_collect_define_inputfunc_candidates_returns_none_on_non_text_handler(
        self,
    ) -> None:
        session = AsyncMock()
        session.call_tool.return_value = _py_eval_payload(
            {
                "ok": True,
                "string_eas": ["0x180800000"],
                "items": [
                    {
                        "string_ea": "0x180800000",
                        "xref_from": "0x180900000",
                        "xref_seg_name": ".data",
                        "handler_ptr_ea": "0x180900010",
                        "handler_va": "0x180A00000",
                        "handler_seg_name": ".rdata",
                    }
                ],
            }
        )

        result = await define_inputfunc._collect_define_inputfunc_candidates(
            session=session,
            input_name="ShowHudHint",
            handler_ptr_offset=0x10,
            allowed_segment_names=(".data",),
            debug=True,
        )

        self.assertIsNone(result)
```

- [ ] **Step 3: If user authorizes test execution, run the new failing tests**

Run:

```bash
python -m pytest tests/test_define_inputfunc_preprocessor.py -q
```

Expected before implementation:

```text
FAILED tests/test_define_inputfunc_preprocessor.py::TestBuildDefineInputFuncPyEval::test_build_define_inputfunc_py_eval_embeds_exact_string_offset_and_text_filter
```

The exact number of failing tests can be greater than one because `_define_inputfunc.py` does not exist yet.

### Task 2: Add preprocess flow and wrapper forwarding tests

**Files:**
- Modify: `tests/test_define_inputfunc_preprocessor.py`
- Modify: `tests/test_ida_preprocessor_scripts.py`

- [ ] **Step 1: Add success and failure flow tests for the shared helper**

Append this code to `tests/test_define_inputfunc_preprocessor.py`:

```python
class TestPreprocessDefineInputFuncSkill(unittest.IsolatedAsyncioTestCase):
    async def test_preprocess_define_inputfunc_skill_writes_requested_func_payload(
        self,
    ) -> None:
        session = AsyncMock()
        requested_fields = [
            (
                "ShowHudHint",
                ["func_name", "func_va", "func_rva", "func_size", "func_sig"],
            )
        ]

        with patch.object(
            define_inputfunc,
            "_collect_define_inputfunc_candidates",
            AsyncMock(
                return_value={
                    "string_eas": ["0x180800000"],
                    "items": [
                        {
                            "string_ea": "0x180800000",
                            "xref_from": "0x180900000",
                            "xref_seg_name": ".data",
                            "handler_ptr_ea": "0x180900010",
                            "handler_va": "0x180123450",
                            "handler_seg_name": ".text",
                        }
                    ],
                }
            ),
        ), patch.object(
            define_inputfunc,
            "_query_func_info",
            AsyncMock(
                return_value={"func_va": "0x180123450", "func_size": "0x90"}
            ),
        ), patch.object(
            define_inputfunc,
            "preprocess_gen_func_sig_via_mcp",
            AsyncMock(
                return_value={
                    "func_sig": "48 89 5C 24 ? 57 48 83 EC ?",
                    "func_rva": "0x123450",
                    "func_size": "0x90",
                }
            ),
        ), patch.object(
            define_inputfunc,
            "write_func_yaml",
        ) as mock_write, patch.object(
            define_inputfunc,
            "_rename_func_best_effort",
            AsyncMock(),
        ) as mock_rename:
            result = await define_inputfunc.preprocess_define_inputfunc_skill(
                session=session,
                expected_outputs=["/tmp/ShowHudHint.windows.yaml"],
                platform="windows",
                image_base=0x180000000,
                target_name="ShowHudHint",
                input_name="ShowHudHint",
                generate_yaml_desired_fields=requested_fields,
                handler_ptr_offset=0x10,
                allowed_segment_names=(".data",),
                rename_to="ShowHudHint",
                debug=True,
            )

        self.assertTrue(result)
        mock_write.assert_called_once_with(
            "/tmp/ShowHudHint.windows.yaml",
            {
                "func_name": "ShowHudHint",
                "func_va": "0x180123450",
                "func_rva": "0x123450",
                "func_size": "0x90",
                "func_sig": "48 89 5C 24 ? 57 48 83 EC ?",
            },
        )
        mock_rename.assert_awaited_once_with(
            session=session,
            func_va="0x180123450",
            func_name="ShowHudHint",
            debug=True,
        )

    async def test_preprocess_define_inputfunc_skill_rejects_multiple_text_handlers(
        self,
    ) -> None:
        requested_fields = [
            (
                "ShowHudHint",
                ["func_name", "func_va", "func_rva", "func_size", "func_sig"],
            )
        ]

        with patch.object(
            define_inputfunc,
            "_collect_define_inputfunc_candidates",
            AsyncMock(
                return_value={
                    "string_eas": ["0x180800000"],
                    "items": [
                        {
                            "string_ea": "0x180800000",
                            "xref_from": "0x180900000",
                            "xref_seg_name": ".data",
                            "handler_ptr_ea": "0x180900010",
                            "handler_va": "0x180123450",
                            "handler_seg_name": ".text",
                        },
                        {
                            "string_ea": "0x180800000",
                            "xref_from": "0x180910000",
                            "xref_seg_name": ".data",
                            "handler_ptr_ea": "0x180910010",
                            "handler_va": "0x180223450",
                            "handler_seg_name": ".text",
                        },
                    ],
                }
            ),
        ), patch.object(define_inputfunc, "write_func_yaml") as mock_write:
            result = await define_inputfunc.preprocess_define_inputfunc_skill(
                session=AsyncMock(),
                expected_outputs=["/tmp/ShowHudHint.windows.yaml"],
                platform="windows",
                image_base=0x180000000,
                target_name="ShowHudHint",
                input_name="ShowHudHint",
                generate_yaml_desired_fields=requested_fields,
                handler_ptr_offset=0x10,
                allowed_segment_names=(".data",),
                rename_to="ShowHudHint",
                debug=True,
            )

        self.assertFalse(result)
        mock_write.assert_not_called()
```

- [ ] **Step 2: Add the ShowHudHint wrapper script path constant**

In `tests/test_ida_preprocessor_scripts.py`, add this constant after `BOT_ADD_COMMAND_HANDLER_SCRIPT_PATH`:

```python
SHOW_HUD_HINT_SCRIPT_PATH = Path(
    "ida_preprocessor_scripts/"
    "find-ShowHudHint.py"
)
```

- [ ] **Step 3: Add wrapper forwarding test**

In `tests/test_ida_preprocessor_scripts.py`, add this test class after `TestFindBotAddCommandHandler`:

```python
class TestFindShowHudHint(unittest.IsolatedAsyncioTestCase):
    async def test_preprocess_skill_forwards_define_inputfunc_contract(self) -> None:
        module = _load_module(
            SHOW_HUD_HINT_SCRIPT_PATH,
            "find_ShowHudHint",
        )
        mock_helper = AsyncMock(return_value=True)
        expected_generate_yaml_desired_fields = [
            (
                "ShowHudHint",
                ["func_name", "func_va", "func_rva", "func_size", "func_sig"],
            )
        ]

        with patch.object(
            module,
            "preprocess_define_inputfunc_skill",
            mock_helper,
            create=True,
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
        mock_helper.assert_awaited_once_with(
            session="session",
            expected_outputs=["out.yaml"],
            platform="windows",
            image_base=0x180000000,
            target_name="ShowHudHint",
            input_name="ShowHudHint",
            generate_yaml_desired_fields=expected_generate_yaml_desired_fields,
            handler_ptr_offset=0x10,
            allowed_segment_names=(".data",),
            rename_to="ShowHudHint",
            debug=True,
        )
```

- [ ] **Step 4: If user authorizes test execution, run the targeted failing tests**

Run:

```bash
python -m pytest tests/test_define_inputfunc_preprocessor.py tests/test_ida_preprocessor_scripts.py::TestFindShowHudHint -q
```

Expected before implementation:

```text
FAILED tests/test_define_inputfunc_preprocessor.py
FAILED tests/test_ida_preprocessor_scripts.py::TestFindShowHudHint::test_preprocess_skill_forwards_define_inputfunc_contract
```

### Task 3: Implement the shared helper

**Files:**
- Create: `ida_preprocessor_scripts/_define_inputfunc.py`

- [ ] **Step 1: Create helper module imports and common utility functions**

Create `ida_preprocessor_scripts/_define_inputfunc.py` with these imports and utility functions:

```python
#!/usr/bin/env python3
"""Shared preprocess helpers for DEFINE_INPUTFUNC-like skills."""

import json
import os

from ida_analyze_util import (
    parse_mcp_result,
    preprocess_gen_func_sig_via_mcp,
    write_func_yaml,
)


def _normalize_requested_fields(generate_yaml_desired_fields, target_name, debug=False):
    if not generate_yaml_desired_fields:
        if debug:
            print("    Preprocess: missing generate_yaml_desired_fields")
        return None

    desired_map = {}
    for symbol_name, fields in generate_yaml_desired_fields:
        desired_map[symbol_name] = list(fields)

    fields = desired_map.get(target_name)
    if not fields:
        if debug:
            print(f"    Preprocess: missing desired fields for {target_name}")
        return None
    return fields


def _resolve_output_path(expected_outputs, target_name, platform, debug=False):
    filename = f"{target_name}.{platform}.yaml"
    matches = [
        path for path in expected_outputs if os.path.basename(path) == filename
    ]
    if len(matches) != 1:
        if debug:
            print(f"    Preprocess: expected exactly one output for {filename}")
        return None
    return matches[0]


def _normalize_addr(value):
    if value is None or isinstance(value, bool):
        return None
    try:
        if isinstance(value, str):
            raw = value.strip()
            if not raw:
                return None
            return hex(int(raw, 0))
        return hex(int(value))
    except (TypeError, ValueError):
        return None


def _normalize_segment_names(allowed_segment_names):
    if isinstance(allowed_segment_names, str):
        values = [allowed_segment_names]
    else:
        try:
            values = list(allowed_segment_names)
        except TypeError:
            return None
    normalized = tuple(str(value) for value in values if isinstance(value, str) and value)
    if not normalized:
        return None
    return normalized


def _build_func_payload(target_name, requested_fields, func_info, extra_fields):
    merged = {"func_name": target_name}
    merged.update(func_info)
    merged.update(extra_fields)

    payload = {}
    for field in requested_fields:
        if field not in merged:
            raise KeyError(field)
        payload[field] = merged[field]
    return payload
```

- [ ] **Step 2: Add the `py_eval` builder**

Append this function to `ida_preprocessor_scripts/_define_inputfunc.py`:

```python
def _build_define_inputfunc_py_eval(
    input_name,
    handler_ptr_offset=0x10,
    allowed_segment_names=(".data",),
):
    normalized_segments = _normalize_segment_names(allowed_segment_names)
    if normalized_segments is None:
        normalized_segments = ()
    params = json.dumps(
        {
            "input_name": input_name,
            "handler_ptr_offset": int(handler_ptr_offset),
            "allowed_segment_names": list(normalized_segments),
        }
    )
    body_lines = [
        "import idaapi, idautils, idc, ida_bytes",
        "input_name = params['input_name']",
        "handler_ptr_offset = params['handler_ptr_offset']",
        "allowed_segment_names = set(params['allowed_segment_names'])",
        "string_eas = []",
        "items = []",
        "def _seg_name(ea):",
        "    seg = idaapi.getseg(ea)",
        "    if not seg:",
        "        return None",
        "    return idc.get_segm_name(seg.start_ea)",
        "for item in idautils.Strings():",
        "    try:",
        "        if str(item) == input_name:",
        "            string_eas.append(hex(int(item.ea)))",
        "    except Exception:",
        "        pass",
        "if len(string_eas) == 1:",
        "    string_ea = int(string_eas[0], 16)",
        "    for xref in idautils.XrefsTo(string_ea, 0):",
        "        xref_from = int(xref.frm)",
        "        xref_seg_name = _seg_name(xref_from)",
        "        if xref_seg_name not in allowed_segment_names:",
        "            continue",
        "        handler_ptr_ea = xref_from + handler_ptr_offset",
        "        try:",
        "            handler_va = int(ida_bytes.get_qword(handler_ptr_ea))",
        "        except Exception:",
        "            continue",
        "        handler_seg_name = _seg_name(handler_va)",
        "        if handler_seg_name == '.text':",
        "            items.append({",
        "                'string_ea': hex(string_ea),",
        "                'xref_from': hex(xref_from),",
        "                'xref_seg_name': xref_seg_name,",
        "                'handler_ptr_ea': hex(handler_ptr_ea),",
        "                'handler_va': hex(handler_va),",
        "                'handler_seg_name': handler_seg_name,",
        "            })",
        "return {'string_eas': string_eas, 'items': items}",
    ]
    lines = [
        "import json, traceback",
        f"params = json.loads({params!r})",
        "def _collect_candidates(params):",
    ]
    lines.extend(f"    {line}" for line in body_lines)
    lines.extend(
        [
            "try:",
            "    collected = _collect_candidates(params)",
            "    result = json.dumps({",
            "        'ok': True,",
            "        'string_eas': collected['string_eas'],",
            "        'items': collected['items'],",
            "    })",
            "except Exception:",
            "    result = json.dumps({",
            "        'ok': False,",
            "        'traceback': traceback.format_exc(),",
            "    })",
        ]
    )
    return "\n".join(lines) + "\n"
```

- [ ] **Step 3: Add MCP JSON parsing, candidate collection, and func query helpers**

Append this code to `ida_preprocessor_scripts/_define_inputfunc.py`:

```python
async def _call_py_eval_json(session, code, debug=False, error_label="py_eval"):
    try:
        result = await session.call_tool(name="py_eval", arguments={"code": code})
        result_data = parse_mcp_result(result)
    except Exception:
        if debug:
            print(f"    Preprocess: {error_label} error")
        return None
    if isinstance(result_data, dict):
        raw = result_data.get("result", "")
    elif result_data is not None:
        raw = str(result_data)
    else:
        raw = ""
    if not raw:
        return None
    try:
        return json.loads(raw)
    except (TypeError, json.JSONDecodeError):
        if debug:
            print(f"    Preprocess: invalid JSON result from {error_label}")
        return None


async def _collect_define_inputfunc_candidates(
    session,
    input_name,
    handler_ptr_offset=0x10,
    allowed_segment_names=(".data",),
    debug=False,
):
    code = _build_define_inputfunc_py_eval(
        input_name=input_name,
        handler_ptr_offset=handler_ptr_offset,
        allowed_segment_names=allowed_segment_names,
    )
    parsed = await _call_py_eval_json(
        session=session,
        code=code,
        debug=debug,
        error_label="py_eval collecting DEFINE_INPUTFUNC candidates",
    )
    if not isinstance(parsed, dict) or parsed.get("ok") is not True:
        if debug and isinstance(parsed, dict):
            traceback_text = parsed.get("traceback")
            if isinstance(traceback_text, str) and traceback_text.strip():
                print(traceback_text.rstrip())
        return None

    string_eas = parsed.get("string_eas")
    items = parsed.get("items")
    if not isinstance(string_eas, list) or len(string_eas) != 1:
        return None
    if not isinstance(items, list) or not items:
        return None

    normalized_items = []
    required_keys = {
        "string_ea",
        "xref_from",
        "xref_seg_name",
        "handler_ptr_ea",
        "handler_va",
        "handler_seg_name",
    }
    for item in items:
        if not isinstance(item, dict) or not required_keys.issubset(item):
            return None
        if item.get("handler_seg_name") != ".text":
            return None
        normalized = dict(item)
        for key in ("string_ea", "xref_from", "handler_ptr_ea", "handler_va"):
            addr = _normalize_addr(normalized.get(key))
            if addr is None:
                return None
            normalized[key] = addr
        if not isinstance(normalized.get("xref_seg_name"), str):
            return None
        normalized_items.append(normalized)

    return {"string_eas": [_normalize_addr(string_eas[0])], "items": normalized_items}


async def _query_func_info(session, handler_va, debug=False):
    fi_code = (
        "import idaapi, json\n"
        f"addr = {handler_va}\n"
        "f = idaapi.get_func(addr)\n"
        "if f and f.start_ea == addr:\n"
        "    result = json.dumps({'func_va': hex(f.start_ea), "
        "'func_size': hex(f.end_ea - f.start_ea)})\n"
        "else:\n"
        "    result = json.dumps(None)\n"
    )
    parsed = await _call_py_eval_json(
        session=session,
        code=fi_code,
        debug=debug,
        error_label=f"py_eval querying func info for {handler_va}",
    )
    if not isinstance(parsed, dict):
        return None
    if "func_va" not in parsed or "func_size" not in parsed:
        return None
    return {"func_va": parsed["func_va"], "func_size": parsed["func_size"]}
```

- [ ] **Step 4: Add rename helper and main preprocess entry**

Append this code to `ida_preprocessor_scripts/_define_inputfunc.py`:

```python
async def _rename_func_best_effort(session, func_va, func_name, debug=False):
    if not func_va or not func_name:
        return
    try:
        await session.call_tool(
            name="rename",
            arguments={"batch": {"func": {"addr": str(func_va), "name": str(func_name)}}},
        )
    except Exception:
        if debug:
            print(f"    Preprocess: failed to rename {func_name} (non-fatal)")


async def preprocess_define_inputfunc_skill(
    session,
    expected_outputs,
    platform,
    image_base,
    target_name,
    input_name,
    generate_yaml_desired_fields,
    handler_ptr_offset=0x10,
    allowed_segment_names=(".data",),
    rename_to=None,
    debug=False,
):
    if not isinstance(target_name, str) or not target_name:
        return False
    if not isinstance(input_name, str) or not input_name:
        return False
    try:
        handler_ptr_offset = int(handler_ptr_offset)
    except (TypeError, ValueError):
        return False
    if handler_ptr_offset < 0:
        return False
    allowed_segment_names = _normalize_segment_names(allowed_segment_names)
    if allowed_segment_names is None:
        return False
    try:
        image_base_int = int(str(image_base), 0)
    except (TypeError, ValueError):
        return False

    requested_fields = _normalize_requested_fields(
        generate_yaml_desired_fields,
        target_name,
        debug=debug,
    )
    if requested_fields is None:
        return False
    output_path = _resolve_output_path(
        expected_outputs,
        target_name,
        platform,
        debug=debug,
    )
    if output_path is None:
        return False

    candidates = await _collect_define_inputfunc_candidates(
        session=session,
        input_name=input_name,
        handler_ptr_offset=handler_ptr_offset,
        allowed_segment_names=allowed_segment_names,
        debug=debug,
    )
    if not isinstance(candidates, dict):
        return False

    items = candidates.get("items")
    if not isinstance(items, list):
        return False
    filtered_items = [
        item
        for item in items
        if item.get("xref_seg_name") in allowed_segment_names
        and item.get("handler_seg_name") == ".text"
    ]
    handler_values = sorted({item.get("handler_va") for item in filtered_items})
    if len(handler_values) != 1:
        if debug:
            print(
                f"    Preprocess: expected exactly one .text handler for {input_name}, got {len(handler_values)}"
            )
        return False

    handler_va = handler_values[0]
    func_info = await _query_func_info(session, handler_va, debug=debug)
    if not isinstance(func_info, dict):
        return False

    extra_fields = {}
    if "func_rva" in requested_fields:
        try:
            extra_fields["func_rva"] = hex(int(str(func_info["func_va"]), 0) - image_base_int)
        except (KeyError, TypeError, ValueError):
            return False
    if "func_sig" in requested_fields:
        sig_info = await preprocess_gen_func_sig_via_mcp(
            session=session,
            func_va=handler_va,
            image_base=image_base_int,
            debug=debug,
        )
        if not sig_info:
            return False
        try:
            extra_fields["func_sig"] = sig_info["func_sig"]
            extra_fields["func_rva"] = sig_info["func_rva"]
            extra_fields["func_size"] = sig_info["func_size"]
        except KeyError:
            return False

    try:
        payload = _build_func_payload(target_name, requested_fields, func_info, extra_fields)
    except KeyError:
        return False

    write_func_yaml(output_path, payload)
    await _rename_func_best_effort(
        session=session,
        func_va=handler_va,
        func_name=rename_to,
        debug=debug,
    )
    return True
```

- [ ] **Step 5: If user authorizes test execution, run the helper tests**

Run:

```bash
python -m pytest tests/test_define_inputfunc_preprocessor.py -q
```

Expected after implementation:

```text
7 passed
```

If the exact count changes because additional tests were added during implementation, all tests in `tests/test_define_inputfunc_preprocessor.py` must pass.

### Task 4: Rewrite the ShowHudHint thin script

**Files:**
- Modify: `ida_preprocessor_scripts/find-ShowHudHint.py`
- Modify: `tests/test_ida_preprocessor_scripts.py`

- [ ] **Step 1: Replace `find-ShowHudHint.py` with the thin wrapper**

Replace the full content of `ida_preprocessor_scripts/find-ShowHudHint.py` with:

```python
#!/usr/bin/env python3
"""Preprocess script for find-ShowHudHint skill."""

from ida_preprocessor_scripts._define_inputfunc import (
    preprocess_define_inputfunc_skill,
)

TARGET_NAME = "ShowHudHint"
INPUT_NAME = "ShowHudHint"
HANDLER_PTR_OFFSET = 0x10
ALLOWED_SEGMENT_NAMES = (".data",)
RENAME_TO = "ShowHudHint"

GENERATE_YAML_DESIRED_FIELDS = [
    # (symbol_name, generate_yaml_fields)
    (
        "ShowHudHint",
        [
            "func_name",
            "func_va",
            "func_rva",
            "func_size",
            "func_sig",
        ],
    ),
]


async def preprocess_skill(
    session, skill_name, expected_outputs, old_yaml_map,
    new_binary_dir, platform, image_base, debug=False,
):
    """Locate the ShowHudHint input handler from its DEFINE_INPUTFUNC descriptor."""
    return await preprocess_define_inputfunc_skill(
        session=session,
        expected_outputs=expected_outputs,
        platform=platform,
        image_base=image_base,
        target_name=TARGET_NAME,
        input_name=INPUT_NAME,
        generate_yaml_desired_fields=GENERATE_YAML_DESIRED_FIELDS,
        handler_ptr_offset=HANDLER_PTR_OFFSET,
        allowed_segment_names=ALLOWED_SEGMENT_NAMES,
        rename_to=RENAME_TO,
        debug=debug,
    )
```

- [ ] **Step 2: If user authorizes test execution, run the wrapper test**

Run:

```bash
python -m pytest tests/test_ida_preprocessor_scripts.py::TestFindShowHudHint -q
```

Expected after implementation:

```text
1 passed
```

- [ ] **Step 3: Check that `config.yaml` still points to the existing output file**

Run this read-only check:

```bash
rg -n "find-ShowHudHint|ShowHudHint\.\{platform\}\.yaml" config.yaml
```

Expected output includes:

```text
2068:      - name: find-ShowHudHint
2070:          - ShowHudHint.{platform}.yaml
```

Line numbers may shift if unrelated config edits have happened, but both strings must remain present.

### Task 5: Static verification and handoff

**Files:**
- Read: `ida_preprocessor_scripts/_define_inputfunc.py`
- Read: `ida_preprocessor_scripts/find-ShowHudHint.py`
- Read: `tests/test_define_inputfunc_preprocessor.py`
- Read: `tests/test_ida_preprocessor_scripts.py`

- [ ] **Step 1: Run syntax-only verification**

Run:

```bash
python -m py_compile ida_preprocessor_scripts/_define_inputfunc.py ida_preprocessor_scripts/find-ShowHudHint.py tests/test_define_inputfunc_preprocessor.py tests/test_ida_preprocessor_scripts.py
```

Expected output:

```text

```

An empty output with exit code `0` means syntax verification succeeded.

- [ ] **Step 2: If user authorizes test execution, run the focused test set**

Run:

```bash
python -m pytest tests/test_define_inputfunc_preprocessor.py tests/test_ida_preprocessor_scripts.py::TestFindShowHudHint -q
```

Expected output:

```text
8 passed
```

If the exact count changes because the implementation adds more focused tests, all selected tests must pass.

- [ ] **Step 3: Inspect the final diff surface**

Run:

```bash
git diff --stat -- ida_preprocessor_scripts/_define_inputfunc.py ida_preprocessor_scripts/find-ShowHudHint.py tests/test_define_inputfunc_preprocessor.py tests/test_ida_preprocessor_scripts.py
```

Expected changed files:

```text
ida_preprocessor_scripts/_define_inputfunc.py
ida_preprocessor_scripts/find-ShowHudHint.py
tests/test_define_inputfunc_preprocessor.py
tests/test_ida_preprocessor_scripts.py
```

- [ ] **Step 4: Report verification accurately**

In the handoff summary, state exactly which commands were run. If test execution was not authorized, report only syntax/static checks and do not claim pytest passed.
