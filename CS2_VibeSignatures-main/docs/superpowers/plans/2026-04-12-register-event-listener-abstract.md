# RegisterEventListener_Abstract Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将 `find-CLoopModeGame_OnEventMapCallbacks-client.py` 从 `llm_decompile` 迁移到可复用的 `RegisterEventListener_Abstract` 程序化预处理 helper，并补齐对应单测。

**Architecture:** 新增 `ida_preprocessor_scripts/_register_event_listener_abstract.py` 作为共享 helper，负责从 `CLoopModeGame_RegisterEventMapInternal.{platform}.yaml` 读取入口、在 source function 内双重确认真实的 `RegisterEventListener_Abstract`、枚举所有同 callee 的注册调用并按 `event_name` 映射到声明目标。`find-CLoopModeGame_OnEventMapCallbacks-client.py` 改为薄配置脚本，仅声明锚点事件、目标事件列表和 YAML 输出字段；测试分为 helper 单测和脚本转发单测两层。

**Tech Stack:** Python 3、`unittest`、`AsyncMock`、IDA MCP `py_eval`、Hex-Rays (`ida_hexrays`)、仓库现有 `write_func_yaml` / `preprocess_gen_func_sig_via_mcp`

---

## File Structure

- Create: `ida_preprocessor_scripts/_register_event_listener_abstract.py`
  - 共享 helper；封装 YAML 读取、`py_eval` 构造、候选收集、目标匹配、函数信息查询、YAML 写出。
- Modify: `ida_preprocessor_scripts/find-CLoopModeGame_OnEventMapCallbacks-client.py`
  - 删除 `llm_decompile` 配置，改为薄配置脚本并转发到新 helper。
- Create: `tests/test_register_event_listener_abstract_preprocessor.py`
  - helper 单测，覆盖 `py_eval` 生成、候选收集、半严格匹配、Hex-Rays 不可用失败、YAML 写出。
- Modify: `tests/test_ida_preprocessor_scripts.py`
  - 为 `find-CLoopModeGame_OnEventMapCallbacks-client.py` 新增一条“转发合同”测试。

### Task 1: 建立 helper 测试骨架

**Files:**
- Create: `tests/test_register_event_listener_abstract_preprocessor.py`
- Modify: `tests/test_ida_preprocessor_scripts.py`

- [ ] **Step 1: 写 helper 单测和脚本转发单测**

```python
import json
import unittest
from unittest.mock import AsyncMock, patch

from ida_preprocessor_scripts import (
    _register_event_listener_abstract as register_event_listener,
)


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


class TestBuildRegisterEventListenerPyEval(unittest.TestCase):
    def test_build_register_event_listener_py_eval_windows_embeds_hexrays_and_slot_recovery(
        self,
    ) -> None:
        code = register_event_listener._build_register_event_listener_py_eval(
            platform="windows",
            anchor_event_name="CLoopModeGame::OnClientPollNetworking",
            search_window_after_anchor=24,
            search_window_before_call=64,
        )

        self.assertIn("ida_hexrays", code)
        self.assertIn("anchor_event_name", code)
        self.assertIn("temp_callback_slot", code)
        self.assertIn("RegisterEventListener_Abstract", code)
        compile(code, "<register_event_listener_windows>", "exec")


class TestCollectRegisterEventListenerCandidates(unittest.IsolatedAsyncioTestCase):
    async def test_collect_candidates_returns_register_function_and_items(self) -> None:
        session = AsyncMock()
        session.call_tool.return_value = _py_eval_payload(
            {
                "ok": True,
                "register_func_va": "0x180055000",
                "items": [
                    {
                        "event_name": "CLoopModeGame::OnClientPollNetworking",
                        "callback_va": "0x180066000",
                        "call_ea": "0x180012345",
                        "temp_base": "0x28",
                        "temp_callback_slot": "0x30",
                    }
                ],
            }
        )

        result = await register_event_listener._collect_register_event_listener_candidates(
            session=session,
            platform="windows",
            anchor_event_name="CLoopModeGame::OnClientPollNetworking",
            search_window_after_anchor=24,
            search_window_before_call=64,
            debug=True,
        )

        self.assertEqual("0x180055000", result["register_func_va"])
        self.assertEqual(
            "CLoopModeGame::OnClientPollNetworking",
            result["items"][0]["event_name"],
        )

    async def test_collect_candidates_returns_none_when_hexrays_is_unavailable(
        self,
    ) -> None:
        session = AsyncMock()
        session.call_tool.return_value = _py_eval_payload(
            {"ok": False, "error": "ida_hexrays unavailable"}
        )

        result = await register_event_listener._collect_register_event_listener_candidates(
            session=session,
            platform="linux",
            anchor_event_name="CLoopModeGame::OnClientPollNetworking",
            search_window_after_anchor=24,
            search_window_before_call=64,
            debug=True,
        )

        self.assertIsNone(result)


class TestPreprocessRegisterEventListenerAbstractSkill(unittest.IsolatedAsyncioTestCase):
    async def test_preprocess_skill_writes_register_function_and_target_callbacks(
        self,
    ) -> None:
        target_specs = [
            {
                "target_name": "CLoopModeGame_OnClientPollNetworking",
                "event_name": "CLoopModeGame::OnClientPollNetworking",
                "rename_to": "CLoopModeGame_OnClientPollNetworking",
            },
            {
                "target_name": "CLoopModeGame_OnClientAdvanceTick",
                "event_name": "CLoopModeGame::OnClientAdvanceTick",
                "rename_to": "CLoopModeGame_OnClientAdvanceTick",
            },
        ]

        requested_fields = [
            ("RegisterEventListener_Abstract", ["func_name", "func_va"]),
            ("CLoopModeGame_OnClientPollNetworking", ["func_name", "func_va"]),
            ("CLoopModeGame_OnClientAdvanceTick", ["func_name", "func_va"]),
        ]

        with patch.object(
            register_event_listener,
            "_read_yaml",
            return_value={"func_va": "0x180010000"},
        ), patch.object(
            register_event_listener,
            "_collect_register_event_listener_candidates",
            AsyncMock(
                return_value={
                    "register_func_va": "0x180055000",
                    "items": [
                        {
                            "event_name": "CLoopModeGame::OnClientPollNetworking",
                            "callback_va": "0x180066000",
                            "call_ea": "0x180012345",
                            "temp_base": "0x28",
                            "temp_callback_slot": "0x30",
                        },
                        {
                            "event_name": "CLoopModeGame::OnClientAdvanceTick",
                            "callback_va": "0x180077000",
                            "call_ea": "0x180012390",
                            "temp_base": "0x28",
                            "temp_callback_slot": "0x30",
                        },
                        {
                            "event_name": "CLoopModeGame::OnUnusedNullsub",
                            "callback_va": "0x180088000",
                            "call_ea": "0x1800123D0",
                            "temp_base": "0x28",
                            "temp_callback_slot": "0x30",
                        },
                    ],
                }
            ),
        ), patch.object(
            register_event_listener,
            "_query_func_info",
            AsyncMock(
                side_effect=[
                    {"func_va": "0x180055000", "func_size": "0x40"},
                    {"func_va": "0x180066000", "func_size": "0x50"},
                    {"func_va": "0x180077000", "func_size": "0x60"},
                ]
            ),
        ), patch.object(
            register_event_listener,
            "write_func_yaml",
        ) as mock_write:
            result = await register_event_listener.preprocess_register_event_listener_abstract_skill(
                session=AsyncMock(),
                expected_outputs=[
                    "/tmp/RegisterEventListener_Abstract.windows.yaml",
                    "/tmp/CLoopModeGame_OnClientPollNetworking.windows.yaml",
                    "/tmp/CLoopModeGame_OnClientAdvanceTick.windows.yaml",
                ],
                new_binary_dir="/tmp",
                platform="windows",
                image_base=0x180000000,
                source_yaml_stem="CLoopModeGame_RegisterEventMapInternal",
                register_func_target_name="RegisterEventListener_Abstract",
                anchor_event_name="CLoopModeGame::OnClientPollNetworking",
                target_specs=target_specs,
                generate_yaml_desired_fields=requested_fields,
                debug=True,
            )

        self.assertTrue(result)
        self.assertEqual(3, mock_write.call_count)
```

```python
ON_EVENT_MAP_CALLBACKS_CLIENT_SCRIPT_PATH = Path(
    "ida_preprocessor_scripts/"
    "find-CLoopModeGame_OnEventMapCallbacks-client.py"
)


class TestFindCLoopModeGameOnEventMapCallbacksClient(unittest.IsolatedAsyncioTestCase):
    async def test_preprocess_skill_forwards_register_event_listener_contract(self) -> None:
        module = _load_module(
            ON_EVENT_MAP_CALLBACKS_CLIENT_SCRIPT_PATH,
            "find_CLoopModeGame_OnEventMapCallbacks_client",
        )
        mock_helper = AsyncMock(return_value=True)

        with patch.object(
            module,
            "preprocess_register_event_listener_abstract_skill",
            mock_helper,
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
        mock_helper.assert_awaited_once()
        kwargs = mock_helper.await_args.kwargs
        self.assertEqual(
            "CLoopModeGame_RegisterEventMapInternal",
            kwargs["source_yaml_stem"],
        )
        self.assertEqual(
            "RegisterEventListener_Abstract",
            kwargs["register_func_target_name"],
        )
        self.assertEqual(
            "CLoopModeGame::OnClientPollNetworking",
            kwargs["anchor_event_name"],
        )
        self.assertEqual(module.TARGET_SPECS, kwargs["target_specs"])
```

- [ ] **Step 2: 跑新增测试，确认当前失败**

Run: `python -m pytest tests/test_register_event_listener_abstract_preprocessor.py tests/test_ida_preprocessor_scripts.py -k "register_event_listener or OnEventMapCallbacks" -v`

Expected: FAIL，报错包含 `ModuleNotFoundError` 或 `AttributeError: module ... has no attribute 'preprocess_register_event_listener_abstract_skill'`

- [ ] **Step 3: 提交测试骨架**

```bash
git add tests/test_register_event_listener_abstract_preprocessor.py tests/test_ida_preprocessor_scripts.py
git commit -m "test(preprocessor): 添加事件监听预处理测试"
```

### Task 2: 实现候选收集 helper

**Files:**
- Create: `ida_preprocessor_scripts/_register_event_listener_abstract.py`
- Test: `tests/test_register_event_listener_abstract_preprocessor.py`

- [ ] **Step 1: 写最小 helper 骨架和公共小函数**

```python
#!/usr/bin/env python3
"""Shared preprocess helpers for RegisterEventListener_Abstract-like skills."""

import json
import os

try:
    import yaml
except ImportError:
    yaml = None

from ida_analyze_util import parse_mcp_result, write_func_yaml


def _read_yaml(path):
    try:
        with open(path, "r", encoding="utf-8") as handle:
            return yaml.safe_load(handle)
    except Exception:
        return None


def _normalize_requested_fields(generate_yaml_desired_fields, target_name, debug=False):
    desired_map = {
        symbol_name: list(fields)
        for symbol_name, fields in generate_yaml_desired_fields
    }
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


async def _call_py_eval_json(session, code, debug=False, error_label="py_eval"):
    try:
        result = await session.call_tool(name="py_eval", arguments={"code": code})
        result_data = parse_mcp_result(result)
    except Exception:
        if debug:
            print(f"    Preprocess: {error_label} error")
        return None

    raw = result_data.get("result", "") if isinstance(result_data, dict) else str(result_data or "")
    if not raw:
        return None

    try:
        return json.loads(raw)
    except (TypeError, json.JSONDecodeError):
        if debug:
            print(f"    Preprocess: invalid JSON result from {error_label}")
        return None
```

- [ ] **Step 2: 写 `py_eval` 构造函数和候选收集函数**

```python
def _build_register_event_listener_py_eval(
    platform,
    anchor_event_name,
    search_window_after_anchor,
    search_window_before_call,
):
    params = json.dumps(
        {
            "platform": platform,
            "anchor_event_name": anchor_event_name,
            "search_window_after_anchor": search_window_after_anchor,
            "search_window_before_call": search_window_before_call,
        }
    )
    return (
        "import idaapi, idautils, idc, ida_bytes, json, traceback\n"
        "try:\n"
        "    import ida_hexrays\n"
        "except Exception:\n"
        "    result = json.dumps({'ok': False, 'error': 'ida_hexrays unavailable'})\n"
        "else:\n"
        f"    params = json.loads({params!r})\n"
        "    platform = params['platform']\n"
        "    anchor_event_name = params['anchor_event_name']\n"
        "    search_window_after_anchor = int(params['search_window_after_anchor'])\n"
        "    search_window_before_call = int(params['search_window_before_call'])\n"
        "    register_func_va = None\n"
        "    items = []\n"
        "    def _scan_exact_strings(target_text):\n"
        "        hits = []\n"
        "        for item in idautils.Strings():\n"
        "            try:\n"
        "                if str(item) == target_text:\n"
        "                    hits.append(int(item.ea))\n"
        "            except Exception:\n"
        "                pass\n"
        "        return hits\n"
        "    def _read_string(ea):\n"
        "        if ea in (None, 0, idaapi.BADADDR):\n"
        "            return None\n"
        "        raw = idc.get_strlit_contents(ea, -1, idc.STRTYPE_C)\n"
        "        if raw is None:\n"
        "            return None\n"
        "        return raw.decode('utf-8', errors='ignore') if isinstance(raw, bytes) else str(raw)\n"
        "    def _resolve_call_callee(call_ea):\n"
        "        if idc.print_insn_mnem(call_ea) not in ('call', 'jmp'):\n"
        "            return None\n"
        "        value = idc.get_operand_value(call_ea, 0)\n"
        "        return value if value not in (None, 0, idaapi.BADADDR) else None\n"
        "    def _recover_temp_base(call_ea):\n"
        "        reg_name = 'rdx' if platform == 'windows' else 'rsi'\n"
        "        min_ea = max(0, call_ea - search_window_before_call)\n"
        "        cur = idc.prev_head(call_ea, min_ea)\n"
        "        while cur != idaapi.BADADDR and cur >= min_ea:\n"
        "            if idc.print_insn_mnem(cur) == 'lea' and (idc.print_operand(cur, 0) or '').lower() == reg_name:\n"
        "                return idc.get_operand_value(cur, 1)\n"
        "            next_cur = idc.prev_head(cur, min_ea)\n"
        "            if next_cur == cur:\n"
        "                break\n"
        "            cur = next_cur\n"
        "        return None\n"
        "    def _recover_event_name(call_ea):\n"
        "        min_ea = max(0, call_ea - search_window_before_call)\n"
        "        cur = idc.prev_head(call_ea, min_ea)\n"
        "        while cur != idaapi.BADADDR and cur >= min_ea:\n"
        "            mnem = idc.print_insn_mnem(cur)\n"
        "            if platform == 'linux' and mnem == 'push':\n"
        "                text = _read_string(idc.get_operand_value(cur, 0))\n"
        "                if text:\n"
        "                    return text\n"
        "            if mnem == 'mov' and '[rsp+' in (idc.print_operand(cur, 0) or '').lower():\n"
        "                text = _read_string(idc.get_operand_value(cur, 1))\n"
        "                if text:\n"
        "                    return text\n"
        "            next_cur = idc.prev_head(cur, min_ea)\n"
        "            if next_cur == cur:\n"
        "                break\n"
        "            cur = next_cur\n"
        "        return None\n"
        "    def _recover_callback_va(call_ea, temp_base):\n"
        "        if temp_base in (None, 0, idaapi.BADADDR):\n"
        "            return None, None\n"
        "        target_slot = temp_base + 8\n"
        "        min_ea = max(0, call_ea - search_window_before_call)\n"
        "        cur = idc.prev_head(call_ea, min_ea)\n"
        "        while cur != idaapi.BADADDR and cur >= min_ea:\n"
        "            if idc.print_insn_mnem(cur) == 'mov' and idc.get_operand_value(cur, 0) == target_slot:\n"
        "                value = idc.get_operand_value(cur, 1)\n"
        "                if value not in (None, 0, idaapi.BADADDR):\n"
        "                    return value, target_slot\n"
        "            next_cur = idc.prev_head(cur, min_ea)\n"
        "            if next_cur == cur:\n"
        "                break\n"
        "            cur = next_cur\n"
        "        return None, target_slot\n"
        "    anchor_hits = _scan_exact_strings(anchor_event_name)\n"
        "    anchor_calls = []\n"
        "    for string_ea in anchor_hits:\n"
        "        for xref in idautils.XrefsTo(string_ea, 0):\n"
        "            xref_ea = int(xref.frm)\n"
        "            max_ea = xref_ea + search_window_after_anchor\n"
        "            cur = xref_ea\n"
        "            while cur != idaapi.BADADDR and cur <= max_ea:\n"
        "                callee = _resolve_call_callee(cur)\n"
        "                if callee is not None:\n"
        "                    anchor_calls.append((cur, callee))\n"
        "                    break\n"
        "                next_cur = idc.next_head(cur, max_ea + 1)\n"
        "                if next_cur in (idaapi.BADADDR, cur):\n"
        "                    break\n"
        "                cur = next_cur\n"
        "    unique_callees = sorted({callee for _, callee in anchor_calls})\n"
        "    if len(unique_callees) != 1:\n"
        "        result = json.dumps({'ok': False, 'error': 'anchor callee is not unique'})\n"
        "    else:\n"
        "        register_func_va = hex(unique_callees[0])\n"
        "        source_func = idaapi.get_func(anchor_calls[0][0]) if anchor_calls else None\n"
        "        cfunc = ida_hexrays.decompile(source_func.start_ea) if source_func else None\n"
        "        if cfunc is None:\n"
        "            result = json.dumps({'ok': False, 'error': 'failed to decompile source function'})\n"
        "        else:\n"
        "            pseudocode = '\\n'.join(line.line for line in cfunc.get_pseudocode())\n"
        "            if anchor_event_name not in pseudocode:\n"
        "                result = json.dumps({'ok': False, 'error': 'anchor string missing in pseudocode'})\n"
        "            else:\n"
        "                for xref in idautils.XrefsTo(unique_callees[0], 0):\n"
        "                    call_ea = int(xref.frm)\n"
        "                    event_name = _recover_event_name(call_ea)\n"
        "                    temp_base = _recover_temp_base(call_ea)\n"
        "                    callback_va, callback_slot = _recover_callback_va(call_ea, temp_base)\n"
        "                    if event_name and callback_va:\n"
        "                        items.append({\n"
        "                            'event_name': event_name,\n"
        "                            'callback_va': hex(callback_va),\n"
        "                            'call_ea': hex(call_ea),\n"
        "                            'temp_base': hex(temp_base),\n"
        "                            'temp_callback_slot': hex(callback_slot),\n"
        "                        })\n"
        "                result = json.dumps({'ok': True, 'register_func_va': register_func_va, 'items': items})\n"
    )


async def _collect_register_event_listener_candidates(
    session,
    platform,
    anchor_event_name,
    search_window_after_anchor,
    search_window_before_call,
    debug=False,
):
    code = _build_register_event_listener_py_eval(
        platform=platform,
        anchor_event_name=anchor_event_name,
        search_window_after_anchor=search_window_after_anchor,
        search_window_before_call=search_window_before_call,
    )
    parsed = await _call_py_eval_json(
        session=session,
        code=code,
        debug=debug,
        error_label="py_eval collecting RegisterEventListener candidates",
    )
    if not isinstance(parsed, dict):
        return None
    if parsed.get("ok") is False:
        if debug:
            print(parsed.get("error", "RegisterEventListener candidate collection failed"))
        return None
    if "register_func_va" not in parsed or "items" not in parsed:
        return None
    if not isinstance(parsed["items"], list):
        return None
    return parsed
```

- [ ] **Step 3: 跑 helper 低层测试，确认转绿**

Run: `python -m pytest tests/test_register_event_listener_abstract_preprocessor.py -k "BuildRegisterEventListenerPyEval or CollectRegisterEventListenerCandidates" -v`

Expected: PASS，输出包含 `2 passed` 到 `4 passed`

- [ ] **Step 4: 提交 helper 候选收集实现**

```bash
git add ida_preprocessor_scripts/_register_event_listener_abstract.py tests/test_register_event_listener_abstract_preprocessor.py
git commit -m "feat(ida): 提取事件监听候选收集辅助"
```

### Task 3: 实现高层预处理编排和 YAML 写出

**Files:**
- Modify: `ida_preprocessor_scripts/_register_event_listener_abstract.py`
- Test: `tests/test_register_event_listener_abstract_preprocessor.py`

- [ ] **Step 1: 先补高层失败用例**

```python
    async def test_preprocess_skill_returns_false_when_declared_event_is_missing(
        self,
    ) -> None:
        with patch.object(
            register_event_listener,
            "_read_yaml",
            return_value={"func_va": "0x180010000"},
        ), patch.object(
            register_event_listener,
            "_collect_register_event_listener_candidates",
            AsyncMock(
                return_value={
                    "register_func_va": "0x180055000",
                    "items": [
                        {
                            "event_name": "CLoopModeGame::OnClientPollNetworking",
                            "callback_va": "0x180066000",
                            "call_ea": "0x180012345",
                            "temp_base": "0x28",
                            "temp_callback_slot": "0x30",
                        }
                    ],
                }
            ),
        ):
            result = await register_event_listener.preprocess_register_event_listener_abstract_skill(
                session=AsyncMock(),
                expected_outputs=["/tmp/CLoopModeGame_OnClientAdvanceTick.windows.yaml"],
                new_binary_dir="/tmp",
                platform="windows",
                image_base=0x180000000,
                source_yaml_stem="CLoopModeGame_RegisterEventMapInternal",
                register_func_target_name="RegisterEventListener_Abstract",
                anchor_event_name="CLoopModeGame::OnClientPollNetworking",
                target_specs=[
                    {
                        "target_name": "CLoopModeGame_OnClientAdvanceTick",
                        "event_name": "CLoopModeGame::OnClientAdvanceTick",
                    }
                ],
                generate_yaml_desired_fields=[
                    ("CLoopModeGame_OnClientAdvanceTick", ["func_name", "func_va"])
                ],
                debug=True,
            )

        self.assertFalse(result)
```

- [ ] **Step 2: 实现高层编排函数、目标匹配和 YAML 写出**

```python
from ida_analyze_util import (
    parse_mcp_result,
    preprocess_gen_func_sig_via_mcp,
    write_func_yaml,
)


async def _query_func_info(session, func_va, debug=False):
    code = (
        "import idaapi, json\n"
        f"addr = {func_va}\n"
        "f = idaapi.get_func(addr)\n"
        "if f and f.start_ea == addr:\n"
        "    result = json.dumps({'func_va': hex(f.start_ea), 'func_size': hex(f.end_ea - f.start_ea)})\n"
        "else:\n"
        "    result = json.dumps(None)\n"
    )
    data = await _call_py_eval_json(
        session=session,
        code=code,
        debug=debug,
        error_label=f\"py_eval querying function info for {func_va}\",
    )
    return data if isinstance(data, dict) else None


def _build_func_payload(target_name, requested_fields, func_info, extra_fields):
    merged = {"func_name": target_name}
    merged.update(func_info)
    merged.update(extra_fields)
    return {field: merged[field] for field in requested_fields}


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


async def preprocess_register_event_listener_abstract_skill(
    session,
    expected_outputs,
    new_binary_dir,
    platform,
    image_base,
    source_yaml_stem,
    register_func_target_name,
    anchor_event_name,
    target_specs,
    generate_yaml_desired_fields,
    register_func_rename_to=None,
    allow_extra_events=True,
    search_window_after_anchor=24,
    search_window_before_call=64,
    debug=False,
):
    src_path = os.path.join(new_binary_dir, f"{source_yaml_stem}.{platform}.yaml")
    src_data = _read_yaml(src_path)
    if not isinstance(src_data, dict) or not src_data.get("func_va"):
        return False

    collected = await _collect_register_event_listener_candidates(
        session=session,
        platform=platform,
        anchor_event_name=anchor_event_name,
        search_window_after_anchor=search_window_after_anchor,
        search_window_before_call=search_window_before_call,
        debug=debug,
    )
    if not isinstance(collected, dict):
        return False

    register_func_va = collected["register_func_va"]
    items = collected["items"]
    items_by_event = {}
    for item in items:
        items_by_event.setdefault(item["event_name"], []).append(item)

    if register_func_rename_to:
        await _rename_func_best_effort(
            session=session,
            func_va=register_func_va,
            func_name=register_func_rename_to,
            debug=debug,
        )

    write_targets = [
        (register_func_target_name, register_func_va),
    ]
    for spec in target_specs:
        matches = items_by_event.get(spec["event_name"], [])
        if len(matches) != 1:
            return False
        if spec.get("rename_to"):
            await _rename_func_best_effort(
                session=session,
                func_va=matches[0]["callback_va"],
                func_name=spec["rename_to"],
                debug=debug,
            )
        write_targets.append((spec["target_name"], matches[0]["callback_va"]))

    if not allow_extra_events:
        declared_events = {spec["event_name"] for spec in target_specs}
        extra_events = sorted(
            {
                item["event_name"]
                for item in items
                if item["event_name"] not in declared_events
            }
        )
        if extra_events:
            return False

    for target_name, func_va in write_targets:
        requested_fields = _normalize_requested_fields(
            generate_yaml_desired_fields,
            target_name,
            debug=debug,
        )
        output_path = _resolve_output_path(
            expected_outputs,
            target_name,
            platform,
            debug=debug,
        )
        func_info = await _query_func_info(session, func_va, debug=debug)
        if not requested_fields or output_path is None or not isinstance(func_info, dict):
            return False

        extra_fields = {}
        if "func_rva" in requested_fields:
            extra_fields["func_rva"] = hex(int(func_info["func_va"], 16) - image_base)
        if "func_sig" in requested_fields:
            sig_info = await preprocess_gen_func_sig_via_mcp(
                session=session,
                func_va=func_va,
                image_base=image_base,
                debug=debug,
            )
            if not sig_info:
                return False
            extra_fields["func_sig"] = sig_info["func_sig"]
            extra_fields["func_rva"] = sig_info["func_rva"]
            extra_fields["func_size"] = sig_info["func_size"]

        write_func_yaml(
            output_path,
            _build_func_payload(target_name, requested_fields, func_info, extra_fields),
        )

    return True
```

- [ ] **Step 3: 跑 helper 全量单测**

Run: `python -m pytest tests/test_register_event_listener_abstract_preprocessor.py -v`

Expected: PASS，输出包含 `passed`

- [ ] **Step 4: 提交高层编排实现**

```bash
git add ida_preprocessor_scripts/_register_event_listener_abstract.py tests/test_register_event_listener_abstract_preprocessor.py
git commit -m "feat(ida): 完成事件监听预处理编排"
```

### Task 4: 改写客户端脚本为薄配置层

**Files:**
- Modify: `ida_preprocessor_scripts/find-CLoopModeGame_OnEventMapCallbacks-client.py`
- Modify: `tests/test_ida_preprocessor_scripts.py`
- Test: `tests/test_register_event_listener_abstract_preprocessor.py`

- [ ] **Step 1: 改写脚本常量和 helper 转发**

```python
#!/usr/bin/env python3
"""Preprocess script for find-CLoopModeGame_OnEventMapCallbacks-client skill."""

from ida_preprocessor_scripts._register_event_listener_abstract import (
    preprocess_register_event_listener_abstract_skill,
)


SOURCE_YAML_STEM = "CLoopModeGame_RegisterEventMapInternal"
REGISTER_FUNC_TARGET_NAME = "RegisterEventListener_Abstract"
ANCHOR_EVENT_NAME = "CLoopModeGame::OnClientPollNetworking"
SEARCH_WINDOW_AFTER_ANCHOR = 24
SEARCH_WINDOW_BEFORE_CALL = 64

TARGET_SPECS = [
    {
        "target_name": "CLoopModeGame_OnClientPollNetworking",
        "event_name": "CLoopModeGame::OnClientPollNetworking",
        "rename_to": "CLoopModeGame_OnClientPollNetworking",
    },
    {
        "target_name": "CLoopModeGame_OnClientAdvanceTick",
        "event_name": "CLoopModeGame::OnClientAdvanceTick",
        "rename_to": "CLoopModeGame_OnClientAdvanceTick",
    },
    {
        "target_name": "CLoopModeGame_OnClientPostAdvanceTick",
        "event_name": "CLoopModeGame::OnClientPostAdvanceTick",
        "rename_to": "CLoopModeGame_OnClientPostAdvanceTick",
    },
    {
        "target_name": "CLoopModeGame_OnClientPreSimulate",
        "event_name": "CLoopModeGame::OnClientPreSimulate",
        "rename_to": "CLoopModeGame_OnClientPreSimulate",
    },
    {
        "target_name": "CLoopModeGame_OnClientPreOutput",
        "event_name": "CLoopModeGame::OnClientPreOutput",
        "rename_to": "CLoopModeGame_OnClientPreOutput",
    },
    {
        "target_name": "CLoopModeGame_OnClientPreOutputParallelWithServer",
        "event_name": "CLoopModeGame::OnClientPreOutputParallelWithServer",
        "rename_to": "CLoopModeGame_OnClientPreOutputParallelWithServer",
    },
    {
        "target_name": "CLoopModeGame_OnClientPostOutput",
        "event_name": "CLoopModeGame::OnClientPostOutput",
        "rename_to": "CLoopModeGame_OnClientPostOutput",
    },
    {
        "target_name": "CLoopModeGame_OnClientFrameSimulate",
        "event_name": "CLoopModeGame::OnClientFrameSimulate",
        "rename_to": "CLoopModeGame_OnClientFrameSimulate",
    },
    {
        "target_name": "CLoopModeGame_OnClientAdvanceNonRenderedFrame",
        "event_name": "CLoopModeGame::OnClientAdvanceNonRenderedFrame",
        "rename_to": "CLoopModeGame_OnClientAdvanceNonRenderedFrame",
    },
    {
        "target_name": "CLoopModeGame_OnClientPostSimulate",
        "event_name": "CLoopModeGame::OnClientPostSimulate",
        "rename_to": "CLoopModeGame_OnClientPostSimulate",
    },
    {
        "target_name": "CLoopModeGame_OnClientPauseSimulate",
        "event_name": "CLoopModeGame::OnClientPauseSimulate",
        "rename_to": "CLoopModeGame_OnClientPauseSimulate",
    },
    {
        "target_name": "CLoopModeGame_OnClientSimulate",
        "event_name": "CLoopModeGame::OnClientSimulate",
        "rename_to": "CLoopModeGame_OnClientSimulate",
    },
    {
        "target_name": "CLoopModeGame_OnPostDataUpdate",
        "event_name": "CLoopModeGame::OnPostDataUpdate",
        "rename_to": "CLoopModeGame_OnPostDataUpdate",
    },
    {
        "target_name": "CLoopModeGame_OnPreDataUpdate",
        "event_name": "CLoopModeGame::OnPreDataUpdate",
        "rename_to": "CLoopModeGame_OnPreDataUpdate",
    },
    {
        "target_name": "CLoopModeGame_OnFrameBoundary",
        "event_name": "CLoopModeGame::OnFrameBoundary",
        "rename_to": "CLoopModeGame_OnFrameBoundary",
    },
]


async def preprocess_skill(
    session,
    skill_name,
    expected_outputs,
    old_yaml_map,
    new_binary_dir,
    platform,
    image_base,
    debug=False,
):
    _ = skill_name, old_yaml_map
    return await preprocess_register_event_listener_abstract_skill(
        session=session,
        expected_outputs=expected_outputs,
        new_binary_dir=new_binary_dir,
        platform=platform,
        image_base=image_base,
        source_yaml_stem=SOURCE_YAML_STEM,
        register_func_target_name=REGISTER_FUNC_TARGET_NAME,
        anchor_event_name=ANCHOR_EVENT_NAME,
        target_specs=TARGET_SPECS,
        generate_yaml_desired_fields=GENERATE_YAML_DESIRED_FIELDS,
        search_window_after_anchor=SEARCH_WINDOW_AFTER_ANCHOR,
        search_window_before_call=SEARCH_WINDOW_BEFORE_CALL,
        debug=debug,
    )
```

- [ ] **Step 2: 跑脚本转发测试和 helper 回归测试**

Run: `python -m pytest tests/test_ida_preprocessor_scripts.py tests/test_register_event_listener_abstract_preprocessor.py -k "OnEventMapCallbacks or register_event_listener" -v`

Expected: PASS，输出包含 `TestFindCLoopModeGameOnEventMapCallbacksClient` 和 helper 测试均通过

- [ ] **Step 3: 跑更宽的相关回归**

Run: `python -m pytest tests/test_registerconcommand_preprocessor.py tests/test_ida_preprocessor_scripts.py -k "registerconcommand or OnEventMapCallbacks or register_event_listener" -v`

Expected: PASS，现有 `registerconcommand` 测试不回归，新 helper 测试继续通过

- [ ] **Step 4: 提交脚本迁移**

```bash
git add ida_preprocessor_scripts/find-CLoopModeGame_OnEventMapCallbacks-client.py tests/test_ida_preprocessor_scripts.py
git commit -m "refactor(ida): 改写客户端事件映射回调脚本"
```

## Self-Review

- Spec coverage
  - 共享 helper：Task 2、Task 3
  - 双重确认 `RegisterEventListener_Abstract`：Task 2 的 `py_eval` 构造步骤
  - 半严格模式：Task 3 的匹配与失败用例
  - client 薄脚本重构：Task 4
  - 测试覆盖：Task 1 到 Task 4
- Placeholder scan
  - 未保留未完成标记或“稍后实现”式描述
  - 每个代码步骤都给出明确代码块或命令
- Type consistency
  - helper 名称统一为 `preprocess_register_event_listener_abstract_skill`
  - 中间结果字段统一为 `register_func_va`、`event_name`、`callback_va`、`call_ea`、`temp_base`、`temp_callback_slot`
