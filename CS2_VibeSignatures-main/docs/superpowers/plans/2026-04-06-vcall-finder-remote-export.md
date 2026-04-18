# Vcall Finder Remote Export Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace large `vcall_finder` function-dump return payloads with IDA-side pure-YAML file export plus small JSON ack, and extract the write-path wrapper into a reusable helper for future large `py_eval` exports.

**Architecture:** Add a generic helper in `ida_analyze_util.py` that wraps arbitrary IDA-side content generation with absolute-path validation, atomic write, and small ack semantics. Refactor `ida_vcall_finder.py` to generate pure YAML inside IDA using `PyYAML`, switch `export_object_xref_details_via_mcp()` to ack-based accounting, and keep downstream aggregation unchanged. Cover the change with focused `unittest` regression tests and update the failure-analysis doc to recommend IDA-side direct export.

**Tech Stack:** Python 3.10, asyncio, `unittest`, `unittest.mock`, `PyYAML`, MCP `py_eval`

---

## File Map

- `ida_analyze_util.py`
  - Add the reusable `build_remote_text_export_py_eval()` helper near `parse_mcp_result()`.
  - Keep the helper generic: it only writes text plus a small ack; it does not own business-specific YAML payload structure.
- `ida_vcall_finder.py`
  - Replace `build_function_dump_py_eval()` with `build_function_dump_export_py_eval()`.
  - Update `export_object_xref_details_via_mcp()` to resolve absolute paths, call the new export builder, and count success/failure from ack payloads instead of in-memory function dumps.
- `tests/test_ida_remote_export.py`
  - Add unit coverage for the reusable export helper contract.
- `tests/test_ida_vcall_finder.py`
  - Add regression coverage for ack-based export accounting and pure-YAML script generation.
- `docs/too_large_content_break_structuredContent.md`
  - Replace the old “client-side truncation” recommendation with the new “IDA-side direct YAML export” recommendation.

## Task 1: Add helper regression tests

**Files:**
- Create: `tests/test_ida_remote_export.py`
- Modify: `ida_analyze_util.py:129-137`
- Check: `pyproject.toml:1-14`

- [ ] **Step 1: Write the failing test file for the reusable helper**

```python
import unittest

from ida_analyze_util import build_remote_text_export_py_eval


class TestBuildRemoteTextExportPyEval(unittest.TestCase):
    def test_build_remote_text_export_py_eval_rejects_relative_output_path(self) -> None:
        with self.assertRaises(ValueError):
            build_remote_text_export_py_eval(
                output_path="relative/detail.yaml",
                producer_code="payload_text = 'ok'",
                content_var="payload_text",
            )

    def test_build_remote_text_export_py_eval_contains_atomic_write_and_small_ack(self) -> None:
        script = build_remote_text_export_py_eval(
            output_path="/tmp/vcall-detail.yaml",
            producer_code="payload_text = 'ok'",
            content_var="payload_text",
            format_name="yaml",
        )
        self.assertIn("os.path.isabs(output_path)", script)
        self.assertIn("tmp_path = output_path + '.tmp'", script)
        self.assertIn("os.replace(tmp_path, output_path)", script)
        self.assertIn("'bytes_written'", script)
        self.assertIn("'format': format_name", script)


if __name__ == "__main__":
    unittest.main()
```

- [ ] **Step 2: Run the helper tests and verify they fail before implementation**

Run:

```bash
uv run python -m unittest discover -s tests -p 'test_ida_remote_export.py' -v
```

Expected: FAIL with `ImportError` or `AttributeError` because `build_remote_text_export_py_eval` does not exist yet.

- [ ] **Step 3: Commit the failing test scaffold**

```bash
git add tests/test_ida_remote_export.py
git commit -m "test(ida): 增加远端导出辅助测试"
```

## Task 2: Implement the reusable remote export helper

**Files:**
- Modify: `ida_analyze_util.py:1-190`
- Test: `tests/test_ida_remote_export.py`

- [ ] **Step 1: Add the import needed for script assembly**

```python
import json
import os
import textwrap
```

- [ ] **Step 2: Add `build_remote_text_export_py_eval()` next to `parse_mcp_result()`**

```python
def build_remote_text_export_py_eval(
    *,
    output_path,
    producer_code,
    content_var="payload_text",
    format_name="text",
):
    """Build a py_eval script that writes large text to disk and returns a small ack."""
    output_path_str = os.fspath(output_path)
    if not os.path.isabs(output_path_str):
        raise ValueError(f"output_path must be absolute, got {output_path_str!r}")
    if not str(producer_code).strip():
        raise ValueError("producer_code cannot be empty")
    if not str(content_var).strip():
        raise ValueError("content_var cannot be empty")

    producer_block = textwrap.indent(str(producer_code).rstrip(), "    ")
    return (
        "import json, os, traceback\n"
        f"output_path = {output_path_str!r}\n"
        f"format_name = {str(format_name)!r}\n"
        "tmp_path = output_path + '.tmp'\n"
        "def _truncate_text(value, limit=800):\n"
        "    text = '' if value is None else str(value)\n"
        "    return text if len(text) <= limit else text[:limit] + ' [truncated]'\n"
        "try:\n"
        "    if not os.path.isabs(output_path):\n"
        "        raise ValueError(f'output_path must be absolute: {output_path}')\n"
        f"{producer_block}\n"
        f"    payload_text = str({content_var})\n"
        "    parent_dir = os.path.dirname(output_path)\n"
        "    if parent_dir:\n"
        "        os.makedirs(parent_dir, exist_ok=True)\n"
        "    with open(tmp_path, 'w', encoding='utf-8') as handle:\n"
        "        handle.write(payload_text)\n"
        "    os.replace(tmp_path, output_path)\n"
        "    result = json.dumps({\n"
        "        'ok': True,\n"
        "        'output_path': output_path,\n"
        "        'bytes_written': len(payload_text.encode('utf-8')),\n"
        "        'format': format_name,\n"
        "    })\n"
        "except Exception as exc:\n"
        "    try:\n"
        "        if os.path.exists(tmp_path):\n"
        "            os.unlink(tmp_path)\n"
        "    except Exception:\n"
        "        pass\n"
        "    result = json.dumps({\n"
        "        'ok': False,\n"
        "        'output_path': output_path,\n"
        "        'error': _truncate_text(exc),\n"
        "        'traceback': _truncate_text(traceback.format_exc()),\n"
        "    })\n"
    )
```

- [ ] **Step 3: Run the helper tests and verify they pass**

Run:

```bash
uv run python -m unittest discover -s tests -p 'test_ida_remote_export.py' -v
```

Expected: PASS for both helper tests.

- [ ] **Step 4: Commit the helper implementation**

```bash
git add ida_analyze_util.py tests/test_ida_remote_export.py
git commit -m "feat(ida): 增加远端文本落盘辅助"
```

## Task 3: Add `vcall_finder` export regression tests

**Files:**
- Create: `tests/test_ida_vcall_finder.py`
- Modify: `ida_vcall_finder.py:747-988`
- Check: `docs/superpowers/specs/2026-04-06-vcall-finder-remote-export-design.md:1`

- [ ] **Step 1: Write the failing regression tests for script generation and ack accounting**

```python
import json
import tempfile
import unittest
from pathlib import Path
from unittest.mock import AsyncMock

import ida_vcall_finder


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


class TestBuildFunctionDumpExportPyEval(unittest.TestCase):
    def test_build_function_dump_export_py_eval_embeds_yaml_dump_and_absolute_path(self) -> None:
        output_path = str(Path("/tmp/vcall-detail.yaml").resolve())
        script = ida_vcall_finder.build_function_dump_export_py_eval(
            0x3EA720,
            output_path=output_path,
            object_name="g_pNetworkMessages",
            module_name="networksystem",
            platform="linux",
        )
        self.assertIn("import yaml", script)
        self.assertIn("PyYAML is required for vcall_finder detail export", script)
        self.assertIn("yaml.dump", script)
        self.assertIn(output_path, script)


class TestExportObjectXrefDetailsViaMcp(unittest.IsolatedAsyncioTestCase):
    async def test_export_object_xref_details_via_mcp_counts_success_from_remote_ack(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            detail_path = ida_vcall_finder.build_vcall_detail_path(
                temp_dir,
                "14141b",
                "g_pNetworkMessages",
                "networksystem",
                "linux",
                "sub_2000",
            ).resolve()
            session = AsyncMock()
            session.call_tool.side_effect = [
                _py_eval_payload(
                    {
                        "object_ea": "0x1000",
                        "functions": [
                            {
                                "func_name": "sub_2000",
                                "func_va": "0x2000",
                            }
                        ],
                    }
                ),
                _py_eval_payload(
                    {
                        "ok": True,
                        "output_path": str(detail_path),
                        "bytes_written": 512,
                        "format": "yaml",
                    }
                ),
            ]

            summary = await ida_vcall_finder.export_object_xref_details_via_mcp(
                session,
                output_root=temp_dir,
                gamever="14141b",
                module_name="networksystem",
                platform="linux",
                object_name="g_pNetworkMessages",
                debug=False,
            )

            self.assertEqual("success", summary["status"])
            self.assertEqual(1, summary["exported_functions"])
            self.assertEqual(0, summary["failed_functions"])
            second_code = session.call_tool.await_args_list[1].kwargs["arguments"]["code"]
            self.assertIn(str(detail_path), second_code)
            self.assertIn("yaml.dump", second_code)

    async def test_export_object_xref_details_via_mcp_counts_failure_from_remote_ack(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            session = AsyncMock()
            session.call_tool.side_effect = [
                _py_eval_payload(
                    {
                        "object_ea": "0x1000",
                        "functions": [
                            {
                                "func_name": "sub_2000",
                                "func_va": "0x2000",
                            }
                        ],
                    }
                ),
                _py_eval_payload(
                    {
                        "ok": False,
                        "output_path": str(Path(temp_dir, "detail.yaml")),
                        "error": "permission denied",
                    }
                ),
            ]

            summary = await ida_vcall_finder.export_object_xref_details_via_mcp(
                session,
                output_root=temp_dir,
                gamever="14141b",
                module_name="networksystem",
                platform="linux",
                object_name="g_pNetworkMessages",
                debug=False,
            )

            self.assertEqual("failed", summary["status"])
            self.assertEqual(0, summary["exported_functions"])
            self.assertEqual(1, summary["failed_functions"])
            self.assertEqual(0, summary["skipped_functions"])


if __name__ == "__main__":
    unittest.main()
```

- [ ] **Step 2: Run the new `vcall_finder` tests and verify they fail before the refactor**

Run:

```bash
uv run python -m unittest discover -s tests -p 'test_ida_vcall_finder.py' -v
```

Expected: FAIL because `build_function_dump_export_py_eval()` does not exist and `export_object_xref_details_via_mcp()` still expects full dump payloads.

- [ ] **Step 3: Commit the failing regression tests**

```bash
git add tests/test_ida_vcall_finder.py
git commit -m "test(vcall_finder): 增加远端导出回归测试"
```

## Task 4: Refactor `vcall_finder` to export YAML directly from IDA

**Files:**
- Modify: `ida_vcall_finder.py:7-16`
- Modify: `ida_vcall_finder.py:747-988`
- Modify: `ida_analyze_util.py:129-177`
- Test: `tests/test_ida_vcall_finder.py`

- [ ] **Step 1: Import the reusable helper into `ida_vcall_finder.py`**

```python
import yaml
from ida_analyze_util import build_remote_text_export_py_eval, parse_mcp_result
from openai import OpenAI
```

- [ ] **Step 2: Replace `build_function_dump_py_eval()` with `build_function_dump_export_py_eval()`**

```python
def build_function_dump_export_py_eval(
    func_va: int | str,
    *,
    output_path: str | Path,
    object_name: str,
    module_name: str,
    platform: str,
) -> str:
    func_va_int = _parse_int_value(func_va, "func_va")
    output_path_str = str(Path(output_path).resolve())
    producer_code = (
        "import ida_funcs, ida_lines, ida_segment, idautils, idc\n"
        "try:\n"
        "    import ida_hexrays\n"
        "except Exception:\n"
        "    ida_hexrays = None\n"
        "try:\n"
        "    import yaml\n"
        "except Exception as exc:\n"
        "    raise RuntimeError('PyYAML is required for vcall_finder detail export') from exc\n"
        "class LiteralDumper(yaml.SafeDumper):\n"
        "    pass\n"
        "def _literal_str_representer(dumper, value):\n"
        "    style = '|' if '\\n' in value else None\n"
        "    return dumper.represent_scalar('tag:yaml.org,2002:str', value, style=style)\n"
        "LiteralDumper.add_representer(str, _literal_str_representer)\n"
        f"func_ea = {func_va_int}\n"
        f"object_name = {object_name!r}\n"
        f"module_name = {module_name!r}\n"
        f"platform_name = {platform!r}\n"
        "def get_disasm(start_ea):\n"
        "    func = ida_funcs.get_func(start_ea)\n"
        "    if func is None:\n"
        "        return ''\n"
        "    lines = []\n"
        "    for ea in idautils.FuncItems(func.start_ea):\n"
        "        if ea < func.start_ea or ea >= func.end_ea:\n"
        "            continue\n"
        "        seg = ida_segment.getseg(ea)\n"
        "        seg_name = ida_segment.get_segm_name(seg) if seg else ''\n"
        "        address_text = f'{seg_name}:{ea:016X}' if seg_name else f'{ea:016X}'\n"
        "        disasm_line = idc.generate_disasm_line(ea, 0) or ''\n"
        "        lines.append(f\"{address_text}                 {ida_lines.tag_remove(disasm_line)}\")\n"
        "    return '\\n'.join(lines)\n"
        "def get_pseudocode(start_ea):\n"
        "    if ida_hexrays is None:\n"
        "        return ''\n"
        "    try:\n"
        "        if not ida_hexrays.init_hexrays_plugin():\n"
        "            return ''\n"
        "        cfunc = ida_hexrays.decompile(start_ea)\n"
        "    except Exception:\n"
        "        return ''\n"
        "    if not cfunc:\n"
        "        return ''\n"
        "    return '\\n'.join(ida_lines.tag_remove(line.line) for line in cfunc.get_pseudocode())\n"
        "func = ida_funcs.get_func(func_ea)\n"
        "if func is None:\n"
        "    raise ValueError(f'Function not found: {hex(func_ea)}')\n"
        "func_start = int(func.start_ea)\n"
        "payload = {\n"
        "    'object_name': object_name,\n"
        "    'module': module_name,\n"
        "    'platform': platform_name,\n"
        "    'func_name': ida_funcs.get_func_name(func_start) or f'sub_{func_start:X}',\n"
        "    'func_va': hex(func_start),\n"
        "    'disasm_code': get_disasm(func_start),\n"
        "    'procedure': get_pseudocode(func_start),\n"
        "}\n"
        "payload_text = yaml.dump(\n"
        "    payload,\n"
        "    Dumper=LiteralDumper,\n"
        "    sort_keys=False,\n"
        "    allow_unicode=True,\n"
        ")\n"
    )
    return build_remote_text_export_py_eval(
        output_path=output_path_str,
        producer_code=producer_code,
        content_var="payload_text",
        format_name="yaml",
    )
```

- [ ] **Step 3: Switch `export_object_xref_details_via_mcp()` to absolute-path export and ack parsing**

```python
        detail_path = build_vcall_detail_path(
            output_root_path,
            gamever,
            object_name,
            module_name,
            platform,
            func_name,
        ).resolve()
        if detail_path.exists():
            skipped_functions += 1
            continue

        try:
            func_va_int = int(func_va_text, 0)
        except ValueError:
            if debug:
                print(
                    f"    vcall_finder: invalid func_va '{func_va_text}' in object '{object_name}'"
                )
            failed_functions += 1
            continue

        function_scope = _format_function_scope(
            gamever,
            module_name,
            platform,
            object_name,
            func_name,
            func_va_text,
        )
        try:
            if debug:
                print(f"    vcall_finder: calling py_eval (function-export) with {function_scope}")
            export_query_result = await session.call_tool(
                name="py_eval",
                arguments={
                    "code": build_function_dump_export_py_eval(
                        func_va_int,
                        output_path=detail_path,
                        object_name=object_name,
                        module_name=module_name,
                        platform=platform,
                    )
                },
            )
        except Exception as exc:
            if debug:
                print(
                    "    vcall_finder: py_eval failed at function-export step "
                    f"with {function_scope}: {exc!r}"
                )
            failed_functions += 1
            continue

        export_ack = _parse_py_eval_json_payload(
            export_query_result,
            debug=debug,
            context=f"function export ({function_scope})",
            expected_keys=("ok", "output_path", "bytes_written", "format"),
        )
        if not isinstance(export_ack, Mapping) or not bool(export_ack.get("ok")):
            if debug:
                print(f"    vcall_finder: invalid function-export ack with {function_scope}")
            failed_functions += 1
            continue

        exported_functions += 1
```

- [ ] **Step 4: Run the `vcall_finder` regression tests and verify they pass**

Run:

```bash
uv run python -m unittest discover -s tests -p 'test_ida_vcall_finder.py' -v
```

Expected: PASS for the script-generation, success-accounting, and failure-accounting cases.

- [ ] **Step 5: Commit the refactor**

```bash
git add ida_vcall_finder.py tests/test_ida_vcall_finder.py
git commit -m "fix(vcall_finder): 改为IDA端直写详情"
```

## Task 5: Update the troubleshooting document

**Files:**
- Modify: `docs/too_large_content_break_structuredContent.md:50-102`
- Check: `docs/superpowers/specs/2026-04-06-vcall-finder-remote-export-design.md:1`
- Regression: `tests/test_ida_remote_export.py`, `tests/test_ida_vcall_finder.py`

- [ ] **Step 1: Replace the old recommended fix with the new IDA-side direct-export recommendation**

```markdown
## Fix Options

### Option A: Export the detail YAML directly from IDA (recommended)

Keep the object-xref query as-is, but stop returning full function dumps through `py_eval`.
Instead, build the detail payload inside IDA, serialize it to pure YAML with `PyYAML`, write it to the target absolute path, and return only a small ack such as:

    {"ok": true, "output_path": "/abs/path/detail.yaml", "bytes_written": 58280, "format": "yaml"}

This avoids the oversized `structuredContent` path entirely, keeps the full disassembly and pseudocode intact, and matches the repository's chosen implementation direction.

**Pros**: Preserves full content, avoids MCP schema-validation failure, and creates a reusable export pattern for other large `py_eval` results.
**Cons**: Assumes `PyYAML` is available in the IDA Python environment, and the target absolute path must be writable from the IDA-side process.

### Option B: Fix the IDA MCP server

The IDA MCP server should either:

1. **Not inject** truncation metadata into `structuredContent`, or
2. **Update** the tool schema to permit those fields.

**Pros**: Fixes the root problem for all clients.
**Cons**: Requires upstream work outside this repository.

### Option C: Pin MCP SDK version (temporary workaround)

Downgrade `mcp` to a version before strict result validation.

**Pros**: Immediate unblock.
**Cons**: Throws away future SDK fixes and still leaves the schema mismatch unresolved.
```

- [ ] **Step 2: Run the focused regression suite after the doc edit to confirm the documented commands still match reality**

Run:

```bash
uv run python -m unittest discover -s tests -p 'test_ida_remote_export.py' -v
uv run python -m unittest discover -s tests -p 'test_ida_vcall_finder.py' -v
```

Expected: PASS for both test files; no code changes are introduced by the doc-only task.

- [ ] **Step 3: Commit the doc update**

```bash
git add docs/too_large_content_break_structuredContent.md
git commit -m "docs(vcall_finder): 更新大结果导出方案"
```
