# RegisterConCommand Helper Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 新增一个可复用的 `RegisterConCommand` 预处理 helper，并落地 `find-BotAdd_CommandHandler.py`，让仓库可以通过 `command_name` 与 `help_string` 稳定提取 `BotAdd_CommandHandler` 的函数 YAML。

**Architecture:** 实现拆成两层：`ida_preprocessor_scripts/_registerconcommand.py` 负责字符串精确匹配、调用点候选提取、平台相关 handler 恢复、字段契约校验与 YAML 输出；`ida_preprocessor_scripts/find-BotAdd_CommandHandler.py` 只负责声明常量并转发给公共 helper。测试层分成两部分：一份独立 helper 单测覆盖 builder 与高层编排，一份脚本转发单测覆盖 `find-BotAdd_CommandHandler.py` 的参数接线；`config.yaml` 最后补技能与 symbol 接线。

**Tech Stack:** Python 3、`unittest`、`unittest.mock`、`pathlib`、`json`、IDA MCP `py_eval`、`yaml.safe_dump`、`uv`

---

## File Structure

- Create: `ida_preprocessor_scripts/_registerconcommand.py`
  - 新公共 helper
  - 封装精确字符串匹配、候选调用点收集、函数信息查询、字段契约组装、YAML 写入
- Create: `ida_preprocessor_scripts/find-BotAdd_CommandHandler.py`
  - 薄封装脚本
  - 只声明 `TARGET_FUNCTION_NAMES`、`COMMAND_NAME`、`HELP_STRING`、`GENERATE_YAML_DESIRED_FIELDS`
- Create: `tests/test_registerconcommand_preprocessor.py`
  - helper 级单测
  - 覆盖 Linux/Windows builder 文本、唯一 handler 成功路径、严格完全匹配失败路径、多 handler 失败路径
- Modify: `tests/test_ida_preprocessor_scripts.py`
  - 新增 `find-BotAdd_CommandHandler.py` 的转发测试
- Modify: `config.yaml`
  - 在 server skills 区域新增 `find-BotAdd_CommandHandler`
  - 在 symbols 区域新增 `BotAdd_CommandHandler`
- Create: `docs/superpowers/plans/2026-04-12-registerconcommand.md`
  - 当前实施计划文档

**仓库约束：**

- 当前仓库默认不强制新建 worktree；若后续用户要求隔离执行，再改用独立 worktree
- 实施阶段优先跑定向 `unittest`
- 不跑无关 build，不扩大到全仓回归
- `git commit` 消息遵循：`<type>(scope): <中文动词开头摘要>`

## 目标实现轮廓

helper 模块最终建议包含以下内部函数与入口：

```python
def _normalize_requested_fields(generate_yaml_desired_fields, target_name, debug=False):
    raise NotImplementedError


def _resolve_output_path(expected_outputs, target_name, platform, debug=False):
    raise NotImplementedError


def _build_registerconcommand_py_eval(
    platform,
    command_name,
    help_string,
    search_window_before_call,
    search_window_after_xref,
):
    raise NotImplementedError


async def _collect_registerconcommand_candidates(
    session,
    platform,
    command_name,
    help_string,
    search_window_before_call,
    search_window_after_xref,
    debug=False,
):
    raise NotImplementedError


async def _query_func_info(session, handler_va, debug=False):
    raise NotImplementedError


def _build_func_payload(target_name, requested_fields, func_info, extra_fields):
    raise NotImplementedError


async def preprocess_registerconcommand_skill(
    session,
    expected_outputs,
    new_binary_dir,
    platform,
    image_base,
    target_name,
    generate_yaml_desired_fields,
    command_name=None,
    help_string=None,
    rename_to=None,
    expected_match_count=1,
    search_window_before_call=48,
    search_window_after_xref=24,
    debug=False,
):
    raise NotImplementedError
```

`find-BotAdd_CommandHandler.py` 的常量目标 shape：

```python
TARGET_FUNCTION_NAMES = [
    "BotAdd_CommandHandler",
]

COMMAND_NAME = "bot_add"
HELP_STRING = (
    "bot_add <t|ct> <type> <difficulty> <name> - "
    "Adds a bot matching the given criteria."
)

GENERATE_YAML_DESIRED_FIELDS = [
    (
        "BotAdd_CommandHandler",
        [
            "func_name",
            "func_sig",
            "func_va",
            "func_rva",
            "func_size",
        ],
    ),
]
```

### Task 1: 先搭测试骨架并锁定 helper 外形

**Files:**
- Create: `tests/test_registerconcommand_preprocessor.py`
- Create: `ida_preprocessor_scripts/_registerconcommand.py`

- [ ] **Step 1: 写 helper 的第一批 failing tests**

新建 `tests/test_registerconcommand_preprocessor.py`，先放入导入、假的 `py_eval` payload helper，以及 builder 轮廓测试：

```python
import json
import unittest
from unittest.mock import AsyncMock, patch

from ida_preprocessor_scripts import _registerconcommand as registerconcommand


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


class TestBuildRegisterConCommandPyEval(unittest.TestCase):
    def test_build_registerconcommand_py_eval_linux_embeds_exact_match_and_linux_registers(
        self,
    ) -> None:
        code = registerconcommand._build_registerconcommand_py_eval(
            platform="linux",
            command_name="bot_add",
            help_string=(
                "bot_add <t|ct> <type> <difficulty> <name> - "
                "Adds a bot matching the given criteria."
            ),
            search_window_before_call=48,
            search_window_after_xref=24,
        )

        self.assertIn("bot_add", code)
        self.assertIn("Adds a bot matching the given criteria.", code)
        self.assertIn("('rsi', 'esi')", code)
        self.assertIn("'handler_va'", code)
        self.assertIn("RegisterConCommand", code)

    def test_build_registerconcommand_py_eval_windows_embeds_slot_recovery_logic(
        self,
    ) -> None:
        code = registerconcommand._build_registerconcommand_py_eval(
            platform="windows",
            command_name="bot_add",
            help_string="bot_add <t|ct> <type> <difficulty> <name> - Adds a bot matching the given criteria.",
            search_window_before_call=48,
            search_window_after_xref=24,
        )

        self.assertIn("handler_slot_addr", code)
        self.assertIn("slot_value_addr", code)
        self.assertIn("lea", code)
        self.assertIn("'command_name'", code)
```

- [ ] **Step 2: 跑这批测试，确认当前确实失败**

Run:

```bash
uv run python -m unittest tests.test_registerconcommand_preprocessor -v
```

Expected:

```text
FAIL: ModuleNotFoundError or AttributeError for _build_registerconcommand_py_eval
```

- [ ] **Step 3: 创建最小 helper 骨架，让测试从“导入失败”推进到“行为失败”**

新建 `ida_preprocessor_scripts/_registerconcommand.py`，先写出公共入口和内部函数骨架：

```python
#!/usr/bin/env python3
"""Shared preprocess helpers for RegisterConCommand-like skills."""

from ida_analyze_util import (
    parse_mcp_result,
    preprocess_gen_func_sig_via_mcp,
    write_func_yaml,
)


def _normalize_requested_fields(generate_yaml_desired_fields, target_name, debug=False):
    raise NotImplementedError


def _resolve_output_path(expected_outputs, target_name, platform, debug=False):
    raise NotImplementedError


def _build_registerconcommand_py_eval(
    platform,
    command_name,
    help_string,
    search_window_before_call,
    search_window_after_xref,
):
    return ""


async def _collect_registerconcommand_candidates(
    session,
    platform,
    command_name,
    help_string,
    search_window_before_call,
    search_window_after_xref,
    debug=False,
):
    return []


async def _query_func_info(session, handler_va, debug=False):
    return None


def _build_func_payload(target_name, requested_fields, func_info, extra_fields):
    raise NotImplementedError


async def preprocess_registerconcommand_skill(
    session,
    expected_outputs,
    new_binary_dir,
    platform,
    image_base,
    target_name,
    generate_yaml_desired_fields,
    command_name=None,
    help_string=None,
    rename_to=None,
    expected_match_count=1,
    search_window_before_call=48,
    search_window_after_xref=24,
    debug=False,
):
    return False
```

- [ ] **Step 4: 再跑一次测试，确认失败已经收敛到具体断言**

Run:

```bash
uv run python -m unittest tests.test_registerconcommand_preprocessor.TestBuildRegisterConCommandPyEval -v
```

Expected:

```text
FAIL: assertion failures for missing embedded strings / register logic
```

- [ ] **Step 5: 提交测试骨架与 helper 外形**

```bash
git add tests/test_registerconcommand_preprocessor.py ida_preprocessor_scripts/_registerconcommand.py
git commit -m "test(preprocess): 添加 RegisterConCommand 测试骨架"
```

### Task 2: 先实现高层编排、字段契约与唯一性校验

**Files:**
- Modify: `tests/test_registerconcommand_preprocessor.py`
- Modify: `ida_preprocessor_scripts/_registerconcommand.py`

- [ ] **Step 1: 追加高层 helper 的 failing tests**

在 `tests/test_registerconcommand_preprocessor.py` 追加高层成功/失败路径：

```python
class TestPreprocessRegisterConCommandSkill(unittest.IsolatedAsyncioTestCase):
    async def test_preprocess_registerconcommand_skill_writes_requested_func_payload(
        self,
    ) -> None:
        session = AsyncMock()
        requested_fields = [
            (
                "BotAdd_CommandHandler",
                ["func_name", "func_sig", "func_va", "func_rva", "func_size"],
            )
        ]

        with patch.object(
            registerconcommand,
            "_collect_registerconcommand_candidates",
            AsyncMock(
                return_value=[
                    {
                        "command_name": "bot_add",
                        "help_string": "bot_add <t|ct> <type> <difficulty> <name> - Adds a bot matching the given criteria.",
                        "handler_va": "0x180055000",
                    }
                ]
            ),
        ), patch.object(
            registerconcommand,
            "_query_func_info",
            AsyncMock(return_value={"func_va": "0x180055000", "func_size": "0x90"}),
        ), patch.object(
            registerconcommand,
            "preprocess_gen_func_sig_via_mcp",
            AsyncMock(
                return_value={
                    "func_va": "0x180055000",
                    "func_rva": "0x55000",
                    "func_size": "0x90",
                    "func_sig": "48 89 5C 24 ?? 57",
                }
            ),
        ), patch.object(registerconcommand, "write_func_yaml") as mock_write:
            result = await registerconcommand.preprocess_registerconcommand_skill(
                session=session,
                expected_outputs=["/tmp/BotAdd_CommandHandler.windows.yaml"],
                new_binary_dir="/tmp",
                platform="windows",
                image_base=0x180000000,
                target_name="BotAdd_CommandHandler",
                generate_yaml_desired_fields=requested_fields,
                command_name="bot_add",
                help_string="bot_add <t|ct> <type> <difficulty> <name> - Adds a bot matching the given criteria.",
                debug=True,
            )

        self.assertTrue(result)
        mock_write.assert_called_once_with(
            "/tmp/BotAdd_CommandHandler.windows.yaml",
            {
                "func_name": "BotAdd_CommandHandler",
                "func_va": "0x180055000",
                "func_rva": "0x55000",
                "func_size": "0x90",
                "func_sig": "48 89 5C 24 ?? 57",
            },
        )

    async def test_preprocess_registerconcommand_skill_requires_exact_command_name_match(
        self,
    ) -> None:
        with patch.object(
            registerconcommand,
            "_collect_registerconcommand_candidates",
            AsyncMock(
                return_value=[
                    {
                        "command_name": "bot_add_cheat",
                        "help_string": "bot_add <t|ct> <type> <difficulty> <name> - Adds a bot matching the given criteria.",
                        "handler_va": "0x180055000",
                    }
                ]
            ),
        ):
            result = await registerconcommand.preprocess_registerconcommand_skill(
                session=AsyncMock(),
                expected_outputs=["/tmp/BotAdd_CommandHandler.windows.yaml"],
                new_binary_dir="/tmp",
                platform="windows",
                image_base=0x180000000,
                target_name="BotAdd_CommandHandler",
                generate_yaml_desired_fields=[
                    ("BotAdd_CommandHandler", ["func_name", "func_va"])
                ],
                command_name="bot_add",
                help_string=None,
                debug=True,
            )

        self.assertFalse(result)

    async def test_preprocess_registerconcommand_skill_rejects_multiple_handlers(
        self,
    ) -> None:
        with patch.object(
            registerconcommand,
            "_collect_registerconcommand_candidates",
            AsyncMock(
                return_value=[
                    {"command_name": "bot_add", "help_string": "a", "handler_va": "0x180010000"},
                    {"command_name": "bot_add", "help_string": "a", "handler_va": "0x180020000"},
                ]
            ),
        ):
            result = await registerconcommand.preprocess_registerconcommand_skill(
                session=AsyncMock(),
                expected_outputs=["/tmp/BotAdd_CommandHandler.windows.yaml"],
                new_binary_dir="/tmp",
                platform="windows",
                image_base=0x180000000,
                target_name="BotAdd_CommandHandler",
                generate_yaml_desired_fields=[
                    ("BotAdd_CommandHandler", ["func_name", "func_va"])
                ],
                command_name="bot_add",
                help_string=None,
                debug=True,
            )

        self.assertFalse(result)
```

- [ ] **Step 2: 跑高层 helper 测试，确认当前失败**

Run:

```bash
uv run python -m unittest tests.test_registerconcommand_preprocessor.TestPreprocessRegisterConCommandSkill -v
```

Expected:

```text
FAIL: preprocess_registerconcommand_skill returns False or payload mismatch
```

- [ ] **Step 3: 实现字段契约、输出路径、唯一性过滤与 payload 组装**

把 `ida_preprocessor_scripts/_registerconcommand.py` 补到可跑高层编排，至少写入下面这段核心逻辑：

```python
import json
import os


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
    matches = [path for path in expected_outputs if os.path.basename(path) == filename]
    if len(matches) != 1:
        if debug:
            print(f"    Preprocess: expected exactly one output for {filename}")
        return None
    return matches[0]


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


async def preprocess_registerconcommand_skill(
    session,
    expected_outputs,
    new_binary_dir,
    platform,
    image_base,
    target_name,
    generate_yaml_desired_fields,
    command_name=None,
    help_string=None,
    rename_to=None,
    expected_match_count=1,
    search_window_before_call=48,
    search_window_after_xref=24,
    debug=False,
):
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

    candidates = await _collect_registerconcommand_candidates(
        session=session,
        platform=platform,
        command_name=command_name,
        help_string=help_string,
        search_window_before_call=search_window_before_call,
        search_window_after_xref=search_window_after_xref,
        debug=debug,
    )

    filtered = [
        item
        for item in candidates
        if (command_name is None or item.get("command_name") == command_name)
        and (help_string is None or item.get("help_string") == help_string)
    ]
    handler_values = sorted({item.get("handler_va") for item in filtered if item.get("handler_va")})
    if len(handler_values) != expected_match_count:
        return False

    func_info = await _query_func_info(session, handler_values[0], debug=debug)
    if not isinstance(func_info, dict):
        return False

    extra_fields = {}
    if "func_rva" in requested_fields:
        extra_fields["func_rva"] = hex(int(func_info["func_va"], 16) - image_base)
    if "func_sig" in requested_fields:
        sig_info = await preprocess_gen_func_sig_via_mcp(
            session=session,
            func_va=handler_values[0],
            image_base=image_base,
            debug=debug,
        )
        if not sig_info:
            return False
        extra_fields["func_sig"] = sig_info["func_sig"]
        extra_fields["func_rva"] = sig_info["func_rva"]
        extra_fields["func_size"] = sig_info["func_size"]

    try:
        payload = _build_func_payload(target_name, requested_fields, func_info, extra_fields)
    except KeyError:
        return False

    write_func_yaml(output_path, payload)
    return True
```

- [ ] **Step 4: 跑高层 helper 测试，确认三条路径通过**

Run:

```bash
uv run python -m unittest tests.test_registerconcommand_preprocessor.TestPreprocessRegisterConCommandSkill -v
```

Expected:

```text
OK
```

- [ ] **Step 5: 提交高层编排实现**

```bash
git add tests/test_registerconcommand_preprocessor.py ida_preprocessor_scripts/_registerconcommand.py
git commit -m "feat(preprocess): 实现 RegisterConCommand 公共流程"
```

### Task 3: 实现平台相关 `py_eval` builder 与候选提取

**Files:**
- Modify: `tests/test_registerconcommand_preprocessor.py`
- Modify: `ida_preprocessor_scripts/_registerconcommand.py`

- [ ] **Step 1: 为候选提取补充 failing tests**

继续在 `tests/test_registerconcommand_preprocessor.py` 追加 builder/collector 测试：

```python
class TestCollectRegisterConCommandCandidates(unittest.IsolatedAsyncioTestCase):
    async def test_collect_registerconcommand_candidates_uses_py_eval_and_returns_candidates(
        self,
    ) -> None:
        session = AsyncMock()
        session.call_tool.return_value = _py_eval_payload(
            {
                "candidates": [
                    {
                        "command_name": "bot_add",
                        "help_string": "bot_add <t|ct> <type> <difficulty> <name> - Adds a bot matching the given criteria.",
                        "handler_va": "0x180055000",
                    }
                ]
            }
        )

        candidates = await registerconcommand._collect_registerconcommand_candidates(
            session=session,
            platform="linux",
            command_name="bot_add",
            help_string="bot_add <t|ct> <type> <difficulty> <name> - Adds a bot matching the given criteria.",
            search_window_before_call=48,
            search_window_after_xref=24,
            debug=True,
        )

        self.assertEqual(
            [
                {
                    "command_name": "bot_add",
                    "help_string": "bot_add <t|ct> <type> <difficulty> <name> - Adds a bot matching the given criteria.",
                    "handler_va": "0x180055000",
                }
            ],
            candidates,
        )
        code = session.call_tool.await_args.kwargs["arguments"]["code"]
        self.assertIn("bot_add", code)
        self.assertIn("search_window_before_call = 48", code)
        self.assertIn("search_window_after_xref = 24", code)

    async def test_collect_registerconcommand_candidates_returns_empty_on_invalid_payload(
        self,
    ) -> None:
        session = AsyncMock()
        session.call_tool.return_value = _py_eval_payload({"unexpected": []})

        candidates = await registerconcommand._collect_registerconcommand_candidates(
            session=session,
            platform="windows",
            command_name="bot_add",
            help_string=None,
            search_window_before_call=48,
            search_window_after_xref=24,
            debug=True,
        )

        self.assertEqual([], candidates)
```

- [ ] **Step 2: 跑候选提取测试，确认当前失败**

Run:

```bash
uv run python -m unittest tests.test_registerconcommand_preprocessor.TestCollectRegisterConCommandCandidates -v
```

Expected:

```text
FAIL: empty result, missing py_eval code, or payload parse errors
```

- [ ] **Step 3: 实现 builder 与候选提取**

在 `ida_preprocessor_scripts/_registerconcommand.py` 中补齐 builder 与 collector。核心代码保持下面这个 shape：

```python
def _build_registerconcommand_py_eval(
    platform,
    command_name,
    help_string,
    search_window_before_call,
    search_window_after_xref,
):
    params = json.dumps(
        {
            "platform": platform,
            "command_name": command_name,
            "help_string": help_string,
            "search_window_before_call": search_window_before_call,
            "search_window_after_xref": search_window_after_xref,
        }
    )
    return (
        "import idaapi, idautils, idc, ida_bytes, json\n"
        f"params = json.loads({params!r})\n"
        "platform = params['platform']\n"
        "search_window_before_call = params['search_window_before_call']\n"
        "search_window_after_xref = params['search_window_after_xref']\n"
        "command_name = params['command_name']\n"
        "help_string = params['help_string']\n"
        "candidates = []\n"
        "handler_slot_addr = None\n"
        "slot_value_addr = None\n"
        "reg_names_linux = [('rsi', 'esi'), ('rdx',), ('r8',)]\n"
        "reg_names_windows = [('rdx',), ('r8',), ('r9',)]\n"
        "def _scan_exact_strings(target_text):\n"
        "    return [] if not target_text else []\n"
        "def _append_candidate(command_value, help_value, handler_va):\n"
        "    if handler_va:\n"
        "        candidates.append({'command_name': command_value, 'help_string': help_value, 'handler_va': handler_va})\n"
        "result = json.dumps({'candidates': candidates})\n"
    )


async def _collect_registerconcommand_candidates(
    session,
    platform,
    command_name,
    help_string,
    search_window_before_call,
    search_window_after_xref,
    debug=False,
):
    code = _build_registerconcommand_py_eval(
        platform=platform,
        command_name=command_name,
        help_string=help_string,
        search_window_before_call=search_window_before_call,
        search_window_after_xref=search_window_after_xref,
    )
    result = await session.call_tool(name='py_eval', arguments={'code': code})
    payload = parse_mcp_result(result)
    raw = payload.get('result', '') if isinstance(payload, dict) else str(payload)
    parsed = json.loads(raw) if raw else {}
    candidates = parsed.get('candidates', [])
    return candidates if isinstance(candidates, list) else []
```

实现时把伪代码里的空扫描替换成真实 IDAPython：

- 字符串必须按完整文本匹配
- Linux 采集 `rsi / rdx / r8`
- Windows 采集 `rdx / r8 / r9` 并恢复栈上的 handler slot
- 最终返回候选列表，每项至少含 `command_name`、`help_string`、`handler_va`

- [ ] **Step 4: 跑 helper 全量单测**

Run:

```bash
uv run python -m unittest tests.test_registerconcommand_preprocessor -v
```

Expected:

```text
OK
```

- [ ] **Step 5: 提交平台提取实现**

```bash
git add tests/test_registerconcommand_preprocessor.py ida_preprocessor_scripts/_registerconcommand.py
git commit -m "feat(preprocess): 完成 RegisterConCommand 平台提取"
```

### Task 4: 新增 `find-BotAdd_CommandHandler.py` 并接入 `config.yaml`

**Files:**
- Create: `ida_preprocessor_scripts/find-BotAdd_CommandHandler.py`
- Modify: `tests/test_ida_preprocessor_scripts.py`
- Modify: `config.yaml`

- [ ] **Step 1: 先写脚本转发的 failing test**

在 `tests/test_ida_preprocessor_scripts.py` 里新增常量和测试类：

```python
BOT_ADD_COMMAND_HANDLER_SCRIPT_PATH = Path(
    "ida_preprocessor_scripts/"
    "find-BotAdd_CommandHandler.py"
)


class TestFindBotAddCommandHandler(unittest.IsolatedAsyncioTestCase):
    async def test_preprocess_skill_forwards_registerconcommand_contract(self) -> None:
        module = _load_module(
            BOT_ADD_COMMAND_HANDLER_SCRIPT_PATH,
            "find_BotAdd_CommandHandler",
        )
        mock_preprocess_registerconcommand_skill = AsyncMock(return_value=True)
        expected_generate_yaml_desired_fields = [
            (
                "BotAdd_CommandHandler",
                [
                    "func_name",
                    "func_sig",
                    "func_va",
                    "func_rva",
                    "func_size",
                ],
            )
        ]

        with patch.object(
            module,
            "preprocess_registerconcommand_skill",
            mock_preprocess_registerconcommand_skill,
        ):
            result = await module.preprocess_skill(
                session="session",
                skill_name="skill",
                expected_outputs=["out.yaml"],
                old_yaml_map={"k": "v"},
                new_binary_dir="bin_dir",
                platform="linux",
                image_base=0x400000,
                debug=True,
            )

        self.assertTrue(result)
        mock_preprocess_registerconcommand_skill.assert_awaited_once_with(
            session="session",
            expected_outputs=["out.yaml"],
            new_binary_dir="bin_dir",
            platform="linux",
            image_base=0x400000,
            target_name="BotAdd_CommandHandler",
            generate_yaml_desired_fields=expected_generate_yaml_desired_fields,
            command_name="bot_add",
            help_string="bot_add <t|ct> <type> <difficulty> <name> - Adds a bot matching the given criteria.",
            debug=True,
        )
```

- [ ] **Step 2: 跑脚本转发测试，确认当前失败**

Run:

```bash
uv run python -m unittest tests.test_ida_preprocessor_scripts.TestFindBotAddCommandHandler -v
```

Expected:

```text
FAIL: file not found for ida_preprocessor_scripts/find-BotAdd_CommandHandler.py
```

- [ ] **Step 3: 创建薄封装脚本并补 `config.yaml`**

新建 `ida_preprocessor_scripts/find-BotAdd_CommandHandler.py`：

```python
#!/usr/bin/env python3
"""Preprocess script for find-BotAdd_CommandHandler skill."""

from ida_preprocessor_scripts._registerconcommand import (
    preprocess_registerconcommand_skill,
)


TARGET_FUNCTION_NAMES = [
    "BotAdd_CommandHandler",
]

COMMAND_NAME = "bot_add"
HELP_STRING = (
    "bot_add <t|ct> <type> <difficulty> <name> - "
    "Adds a bot matching the given criteria."
)

GENERATE_YAML_DESIRED_FIELDS = [
    (
        "BotAdd_CommandHandler",
        [
            "func_name",
            "func_sig",
            "func_va",
            "func_rva",
            "func_size",
        ],
    ),
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
    return await preprocess_registerconcommand_skill(
        session=session,
        expected_outputs=expected_outputs,
        new_binary_dir=new_binary_dir,
        platform=platform,
        image_base=image_base,
        target_name=TARGET_FUNCTION_NAMES[0],
        generate_yaml_desired_fields=GENERATE_YAML_DESIRED_FIELDS,
        command_name=COMMAND_NAME,
        help_string=HELP_STRING,
        debug=debug,
    )
```

在 `config.yaml` 的 server skills 区域，紧跟 bot 相关 skill 后插入：

```yaml
      - name: find-BotAdd_CommandHandler
        expected_output:
          - BotAdd_CommandHandler.{platform}.yaml
```

在 symbols 区域 bot 相关函数附近插入：

```yaml
      - name: BotAdd_CommandHandler
        category: func
```

- [ ] **Step 4: 跑 wrapper + helper 定向回归**

Run:

```bash
uv run python -m unittest \
  tests.test_registerconcommand_preprocessor \
  tests.test_ida_preprocessor_scripts.TestFindBotAddCommandHandler -v
```

Expected:

```text
OK
```

- [ ] **Step 5: 提交脚本与配置接线**

```bash
git add \
  ida_preprocessor_scripts/find-BotAdd_CommandHandler.py \
  tests/test_ida_preprocessor_scripts.py \
  config.yaml
git commit -m "feat(server): 接入 BotAdd 命令处理定位"
```

### Task 5: 做最终定向验证并整理交付说明

**Files:**
- Test: `tests/test_registerconcommand_preprocessor.py`
- Test: `tests/test_ida_preprocessor_scripts.py`
- Review: `config.yaml`

- [ ] **Step 1: 跑最终组合回归**

Run:

```bash
uv run python -m unittest \
  tests.test_registerconcommand_preprocessor \
  tests.test_ida_preprocessor_scripts -v
```

Expected:

```text
OK
```

- [ ] **Step 2: 检查 BotAdd 相关 diff 是否只落在预期文件**

Run:

```bash
git diff -- \
  ida_preprocessor_scripts/_registerconcommand.py \
  ida_preprocessor_scripts/find-BotAdd_CommandHandler.py \
  tests/test_registerconcommand_preprocessor.py \
  tests/test_ida_preprocessor_scripts.py \
  config.yaml
```

Expected:

```text
Only the five target files above appear in the diff
```

- [ ] **Step 3: 用最终提交整理实现**

```bash
git add \
  ida_preprocessor_scripts/_registerconcommand.py \
  ida_preprocessor_scripts/find-BotAdd_CommandHandler.py \
  tests/test_registerconcommand_preprocessor.py \
  tests/test_ida_preprocessor_scripts.py \
  config.yaml
git commit -m "feat(preprocess): 添加 RegisterConCommand 命令定位"
```

- [ ] **Step 4: 交付时明确说明未做的验证边界**

交付说明应包含：

```text
1. 已跑的单测命令与结果
2. Linux/Windows 平台逻辑当前由 builder 与 helper 单测覆盖
3. 尚未在真实 IDA MCP 会话里跑 bot_add 实机 smoke（如果本轮没有环境）
4. config.yaml 已接入 BotAdd_CommandHandler 输出
```

## Self-Review Checklist

- Spec coverage:
  - 新 helper：Task 1-3
  - `find-BotAdd_CommandHandler.py`：Task 4
  - `GENERATE_YAML_DESIRED_FIELDS` 契约：Task 2、Task 4
  - Linux 大函数场景的调用点分析：Task 3
  - `command_name` 完全匹配：Task 2
  - `config.yaml` 接线：Task 4
- Placeholder scan:
  - 没有 `TODO`、`TBD`、`implement later`
  - 所有命令都给了精确路径
  - 所有代码步骤都提供了具体代码块
- Type consistency:
  - helper 入口统一使用 `preprocess_registerconcommand_skill(...)`
  - 输出 symbol 名统一为 `BotAdd_CommandHandler`
  - 配置、脚本、测试的 skill 文件名统一为 `find-BotAdd_CommandHandler.py`
