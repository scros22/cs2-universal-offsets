# Reference YAML Generator Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为仓库新增 `generate_reference_yaml.py` 独立 CLI，并配套 project-level SKILL，使其能够从现有 `bin/<gamever>/<module>/<func_name>.<platform>.yaml` 或 `config.yaml` + IDA MCP 自动生成 `ida_preprocessor_scripts/references/<module>/<func_name>.<platform>.yaml`。

**Architecture:** 先在 `tests/test_generate_reference_yaml.py` 中用现有 `unittest` 风格锁定纯 helper、函数地址解析、MCP 导出与 attach/auto-start 两种连接模式；实现上把首版 CLI 保持在单文件 `generate_reference_yaml.py`，内部拆成“路径/配置解析”“函数地址解析”“MCP 会话适配”“reference YAML 导出与落盘”四层小函数，避免首版扩散到多模块。最后新增 `.claude/skills/generate-reference-yaml/SKILL.md` 与 README/README_CN 文档，把 reference YAML 准备步骤、命令行入口和 SKILL 触发方式统一到同一条后端路径。

**Tech Stack:** Python 3.10, `argparse`, `asyncio`, `unittest`, `unittest.mock`, `httpx`, MCP `streamable_http_client`, `PyYAML`, `uv run idalib-mcp`

---

## File Structure

- `generate_reference_yaml.py`
  - 新增仓库根目录 CLI。
  - 承载参数解析、路径规范化、旧 YAML 优先解析、`config.yaml` alias 收集、MCP 会话管理、reference YAML 导出与落盘。
- `tests/test_generate_reference_yaml.py`
  - 新增定向测试文件。
  - 覆盖纯 helper、解析优先级、歧义错误、导出 payload、attach/auto-start 会话编排。
- `.claude/skills/generate-reference-yaml/SKILL.md`
  - 新增 project-level SKILL。
  - 只调用 `uv run generate_reference_yaml.py ...`，不直接编排 IDA API。
- `README.md`
  - 增加英文版 “Generate reference YAML” 使用章节。
- `README_CN.md`
  - 增加中文版 “生成 reference YAML” 使用章节。

## Validation Notes

- 纯 helper / 异步解析 / 会话编排都走现有 `unittest`：
  - `uv run python -m unittest tests.test_generate_reference_yaml -v`
- CLI 语法与导入最小验证：
  - `uv run python -m py_compile generate_reference_yaml.py`
- 手动验收场景聚焦：
  - `CNetworkGameClient_RecordEntityBandwidth`
  - 输出应落到 `ida_preprocessor_scripts/references/engine/CNetworkGameClient_RecordEntityBandwidth.windows.yaml`

## Task 1: 先锁定纯 helper 与 CLI 参数约束

**Files:**
- Create: `tests/test_generate_reference_yaml.py`
- Create: `generate_reference_yaml.py`

- [ ] **Step 1: 先写失败测试，锁定路径、旧 YAML 优先级、`config.yaml` alias 收集与参数约束**

```python
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import yaml

import generate_reference_yaml


def _write_yaml(path: Path, payload: dict[str, object]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(yaml.safe_dump(payload, sort_keys=False), encoding="utf-8")


class TestReferenceYamlPureHelpers(unittest.TestCase):
    def test_build_reference_output_path_puts_module_and_platform_in_path(self) -> None:
        path = generate_reference_yaml.build_reference_output_path(
            repo_root=Path("/repo"),
            module="engine",
            func_name="CNetworkMessages_FindNetworkGroup",
            platform="windows",
        )

        self.assertEqual(
            Path(
                "/repo/ida_preprocessor_scripts/references/engine/"
                "CNetworkMessages_FindNetworkGroup.windows.yaml"
            ),
            path,
        )

    def test_load_existing_func_va_prefers_bin_yaml(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            _write_yaml(
                repo_root / "bin" / "14141" / "engine" / "Foo.windows.yaml",
                {
                    "func_name": "Foo",
                    "func_va": "0x180123450",
                },
            )

            self.assertEqual(
                "0x180123450",
                generate_reference_yaml.load_existing_func_va(
                    repo_root=repo_root,
                    gamever="14141",
                    module="engine",
                    func_name="Foo",
                    platform="windows",
                ),
            )

    def test_load_symbol_aliases_collects_name_then_alias(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            (repo_root / "config.yaml").write_text(
                """
modules:
  - name: engine
    symbols:
      - name: CNetworkGameClient_RecordEntityBandwidth
        category: func
        alias:
          - CNetworkGameClient::RecordEntityBandwidth
          - RecordEntityBandwidth
""".strip(),
                encoding="utf-8",
            )

            self.assertEqual(
                [
                    "CNetworkGameClient_RecordEntityBandwidth",
                    "CNetworkGameClient::RecordEntityBandwidth",
                    "RecordEntityBandwidth",
                ],
                generate_reference_yaml.load_symbol_aliases(
                    repo_root=repo_root,
                    module="engine",
                    func_name="CNetworkGameClient_RecordEntityBandwidth",
                ),
            )

    def test_parse_args_rejects_auto_start_without_binary(self) -> None:
        with self.assertRaises(SystemExit):
            generate_reference_yaml.parse_args(
                [
                    "-gamever",
                    "14141",
                    "-module",
                    "engine",
                    "-platform",
                    "windows",
                    "-func_name",
                    "CNetworkGameClient_RecordEntityBandwidth",
                    "-auto_start_mcp",
                ]
            )
```

- [ ] **Step 2: 运行测试，确认当前失败**

Run:

```bash
uv run python -m unittest tests.test_generate_reference_yaml -v
```

Expected: FAIL，因为 `generate_reference_yaml.py` 还不存在，且上述 helper 与参数约束尚未实现。

- [ ] **Step 3: 写最小可测 helper 层与 CLI 参数解析**

```python
#!/usr/bin/env python3

from __future__ import annotations

import argparse
from pathlib import Path

import yaml


class ReferenceGenerationError(RuntimeError):
    pass


class LiteralDumper(yaml.SafeDumper):
    pass


def _literal_str_representer(dumper, value):
    style = "|" if "\n" in value else None
    return dumper.represent_scalar("tag:yaml.org,2002:str", value, style=style)


LiteralDumper.add_representer(str, _literal_str_representer)


def parse_args(argv=None):
    parser = argparse.ArgumentParser(
        description="Generate minimal reference YAML from IDA MCP."
    )
    parser.add_argument("-gamever", required=True)
    parser.add_argument("-module", required=True)
    parser.add_argument("-platform", choices=["windows", "linux"], required=True)
    parser.add_argument("-func_name", required=True)
    parser.add_argument("-mcp_host", default="127.0.0.1")
    parser.add_argument("-mcp_port", type=int, default=13337)
    parser.add_argument("-binary", default=None)
    parser.add_argument("-auto_start_mcp", action="store_true")
    parser.add_argument("-ida_args", default="")
    parser.add_argument("-debug", action="store_true")
    args = parser.parse_args(argv)
    if args.auto_start_mcp and not args.binary:
        parser.error("-auto_start_mcp requires -binary")
    if args.binary and not args.auto_start_mcp:
        parser.error("-binary requires -auto_start_mcp")
    return args


def load_yaml_mapping(path: Path) -> dict[str, object]:
    if not path.exists():
        return {}
    parsed = yaml.safe_load(path.read_text(encoding="utf-8"))
    if parsed is None:
        return {}
    if not isinstance(parsed, dict):
        raise ReferenceGenerationError(
            f"YAML root must be mapping: {path}"
        )
    return parsed


def build_reference_output_path(*, repo_root: Path, module: str, func_name: str, platform: str) -> Path:
    return (
        Path(repo_root)
        / "ida_preprocessor_scripts"
        / "references"
        / module
        / f"{func_name}.{platform}.yaml"
    )


def build_existing_yaml_path(*, repo_root: Path, gamever: str, module: str, func_name: str, platform: str) -> Path:
    return Path(repo_root) / "bin" / gamever / module / f"{func_name}.{platform}.yaml"


def load_existing_func_va(*, repo_root: Path, gamever: str, module: str, func_name: str, platform: str) -> str | None:
    data = load_yaml_mapping(
        build_existing_yaml_path(
            repo_root=repo_root,
            gamever=gamever,
            module=module,
            func_name=func_name,
            platform=platform,
        )
    )
    func_va = str(data.get("func_va", "")).strip()
    return func_va or None


def load_symbol_aliases(*, repo_root: Path, module: str, func_name: str) -> list[str]:
    config_data = load_yaml_mapping(Path(repo_root) / "config.yaml")
    modules = config_data.get("modules", [])
    for module_data in modules:
        if not isinstance(module_data, dict) or module_data.get("name") != module:
            continue
        for symbol_data in module_data.get("symbols", []):
            if not isinstance(symbol_data, dict) or symbol_data.get("name") != func_name:
                continue
            names = [func_name]
            names.extend(str(alias).strip() for alias in symbol_data.get("alias", []) if str(alias).strip())
            return names
    raise ReferenceGenerationError(
        f"symbol not found in config.yaml: module={module}, func_name={func_name}"
    )
```

- [ ] **Step 4: 重跑纯 helper 测试，确认通过**

Run:

```bash
uv run python -m unittest tests.test_generate_reference_yaml.TestReferenceYamlPureHelpers -v
```

Expected: PASS，路径、旧 YAML 优先级、`config.yaml` alias 收集与 `-auto_start_mcp/-binary` 配对约束全部通过。

- [ ] **Step 5: 提交当前 helper 骨架**

```bash
git add generate_reference_yaml.py tests/test_generate_reference_yaml.py
git commit -m "test(reference): 增加 reference yaml CLI 基础测试"
```

## Task 2: 实现函数地址解析与 reference payload 导出

**Files:**
- Modify: `generate_reference_yaml.py`
- Modify: `tests/test_generate_reference_yaml.py`

- [ ] **Step 1: 先补失败测试，锁定“旧 YAML 优先、IDA alias fallback、歧义报错、伪代码可空”**

```python
import json
from unittest.mock import AsyncMock


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


class TestResolveFuncVa(unittest.IsolatedAsyncioTestCase):
    async def test_resolve_func_va_uses_existing_yaml_before_ida_lookup(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            _write_yaml(
                repo_root / "bin" / "14141" / "engine" / "Foo.windows.yaml",
                {"func_name": "Foo", "func_va": "0x180123450"},
            )
            session = AsyncMock()

            func_va = await generate_reference_yaml.resolve_func_va(
                session=session,
                repo_root=repo_root,
                gamever="14141",
                module="engine",
                platform="windows",
                func_name="Foo",
                debug=False,
            )

            self.assertEqual("0x180123450", func_va)
            session.call_tool.assert_not_awaited()

    async def test_resolve_func_va_falls_back_to_config_aliases(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            (repo_root / "config.yaml").write_text(
                """
modules:
  - name: engine
    symbols:
      - name: CNetworkGameClient_RecordEntityBandwidth
        category: func
        alias:
          - CNetworkGameClient::RecordEntityBandwidth
          - RecordEntityBandwidth
""".strip(),
                encoding="utf-8",
            )
            session = AsyncMock()
            session.call_tool.return_value = _py_eval_payload(
                [
                    {
                        "query": "RecordEntityBandwidth",
                        "func_name": "CNetworkGameClient_RecordEntityBandwidth",
                        "func_va": "0x180123450",
                    }
                ]
            )

            func_va = await generate_reference_yaml.resolve_func_va(
                session=session,
                repo_root=repo_root,
                gamever="14141",
                module="engine",
                platform="windows",
                func_name="CNetworkGameClient_RecordEntityBandwidth",
                debug=False,
            )

            self.assertEqual("0x180123450", func_va)

    async def test_resolve_func_va_raises_on_ambiguous_matches(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            (repo_root / "config.yaml").write_text(
                """
modules:
  - name: engine
    symbols:
      - name: Foo
        category: func
        alias:
          - FooAlias
""".strip(),
                encoding="utf-8",
            )
            session = AsyncMock()
            session.call_tool.return_value = _py_eval_payload(
                [
                    {"query": "Foo", "func_name": "Foo", "func_va": "0x180100000"},
                    {"query": "FooAlias", "func_name": "Foo_impl", "func_va": "0x180200000"},
                ]
            )

            with self.assertRaisesRegex(
                generate_reference_yaml.ReferenceGenerationError,
                "ambiguous",
            ):
                await generate_reference_yaml.resolve_func_va(
                    session=session,
                    repo_root=repo_root,
                    gamever="14141",
                    module="engine",
                    platform="windows",
                    func_name="Foo",
                    debug=False,
                )


class TestExportReferencePayload(unittest.IsolatedAsyncioTestCase):
    async def test_export_reference_payload_keeps_empty_procedure_when_hexrays_missing(self) -> None:
        session = AsyncMock()
        session.call_tool.return_value = _py_eval_payload(
            {
                "func_name": "CNetworkGameClient_RecordEntityBandwidth",
                "func_va": "0x180123450",
                "disasm_code": "text:0000000180123450 mov eax, 1",
                "procedure": "",
            }
        )

        payload = await generate_reference_yaml.export_reference_payload_via_mcp(
            session=session,
            func_name="CNetworkGameClient_RecordEntityBandwidth",
            func_va="0x180123450",
            debug=False,
        )

        self.assertEqual("CNetworkGameClient_RecordEntityBandwidth", payload["func_name"])
        self.assertEqual("0x180123450", payload["func_va"])
        self.assertEqual("", payload["procedure"])
        self.assertIn("mov eax, 1", payload["disasm_code"])
```

- [ ] **Step 2: 运行解析/导出测试，确认当前失败**

Run:

```bash
uv run python -m unittest \
  tests.test_generate_reference_yaml.TestResolveFuncVa \
  tests.test_generate_reference_yaml.TestExportReferencePayload \
  -v
```

Expected: FAIL，因为 `resolve_func_va(...)`、`find_function_addr_by_names(...)`、`export_reference_payload_via_mcp(...)` 还未实现。

- [ ] **Step 3: 实现解析顺序、IDA alias 搜索与最小 reference payload 导出**

```python
import json

from ida_analyze_util import parse_mcp_result


async def find_function_addr_by_names(session, candidate_names, *, debug=False) -> str:
    py_code = (
        "import idaapi, ida_funcs, ida_name, json\n"
        f"candidate_names = {json.dumps(list(candidate_names), ensure_ascii=False)}\n"
        "matches = []\n"
        "seen = set()\n"
        "for candidate in candidate_names:\n"
        "    if not candidate:\n"
        "        continue\n"
        "    ea = ida_name.get_name_ea(idaapi.BADADDR, candidate)\n"
        "    if ea == idaapi.BADADDR:\n"
        "        continue\n"
        "    func = ida_funcs.get_func(ea)\n"
        "    if func is None:\n"
        "        continue\n"
        "    start_ea = int(func.start_ea)\n"
        "    if start_ea in seen:\n"
        "        continue\n"
        "    seen.add(start_ea)\n"
        "    matches.append({\n"
        "        'query': candidate,\n"
        "        'func_name': ida_funcs.get_func_name(start_ea) or f'sub_{start_ea:X}',\n"
        "        'func_va': hex(start_ea),\n"
        "    })\n"
        "result = json.dumps(matches)\n"
    )
    result = await session.call_tool(name="py_eval", arguments={"code": py_code})
    payload = parse_mcp_result(result)
    result_text = payload.get("result", "") if isinstance(payload, dict) else ""
    matches = json.loads(result_text) if result_text else []
    if not matches:
        raise ReferenceGenerationError("unable to locate function address via IDA")
    if len(matches) != 1:
        raise ReferenceGenerationError(f"ambiguous function matches: {matches}")
    return str(matches[0]["func_va"])


async def resolve_func_va(
    *,
    session,
    repo_root: Path,
    gamever: str,
    module: str,
    platform: str,
    func_name: str,
    debug: bool,
) -> str:
    existing = load_existing_func_va(
        repo_root=repo_root,
        gamever=gamever,
        module=module,
        func_name=func_name,
        platform=platform,
    )
    if existing:
        return existing
    candidate_names = load_symbol_aliases(
        repo_root=repo_root,
        module=module,
        func_name=func_name,
    )
    return await find_function_addr_by_names(
        session,
        candidate_names,
        debug=debug,
    )


async def export_reference_payload_via_mcp(session, *, func_name: str, func_va: str, debug=False) -> dict[str, str]:
    func_va_int = int(str(func_va), 0)
    py_code = (
        "import ida_funcs, ida_lines, ida_segment, idautils, idc, json\n"
        "try:\n"
        "    import ida_hexrays\n"
        "except Exception:\n"
        "    ida_hexrays = None\n"
        f"func_ea = {func_va_int}\n"
        f"expected_name = {func_name!r}\n"
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
        "    result = json.dumps(None)\n"
        "else:\n"
        "    func_start = int(func.start_ea)\n"
        "    result = json.dumps({\n"
        "        'func_name': expected_name,\n"
        "        'func_va': hex(func_start),\n"
        "        'disasm_code': get_disasm(func_start),\n"
        "        'procedure': get_pseudocode(func_start),\n"
        "    })\n"
    )
    result = await session.call_tool(name="py_eval", arguments={"code": py_code})
    payload = parse_mcp_result(result)
    result_text = payload.get("result", "") if isinstance(payload, dict) else ""
    exported = json.loads(result_text) if result_text else None
    if not isinstance(exported, dict):
        raise ReferenceGenerationError(f"failed to export reference payload for {func_name}")
    return {
        "func_name": str(exported.get("func_name") or func_name),
        "func_va": str(exported.get("func_va") or func_va),
        "disasm_code": str(exported.get("disasm_code", "") or ""),
        "procedure": str(exported.get("procedure", "") or ""),
    }


def write_reference_yaml(path: Path, payload: dict[str, str]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        yaml.dump(
            payload,
            Dumper=LiteralDumper,
            sort_keys=False,
            allow_unicode=True,
        ),
        encoding="utf-8",
    )
```

- [ ] **Step 4: 重跑解析/导出测试，确认通过**

Run:

```bash
uv run python -m unittest \
  tests.test_generate_reference_yaml.TestResolveFuncVa \
  tests.test_generate_reference_yaml.TestExportReferencePayload \
  -v
```

Expected: PASS，解析顺序固定为“旧 YAML `func_va` 优先，其次 `config.yaml` + IDA alias 搜索”，并且导出的 YAML payload 保持最小 schema 与空 `procedure` 兼容。

- [ ] **Step 5: 提交解析与导出实现**

```bash
git add generate_reference_yaml.py tests/test_generate_reference_yaml.py
git commit -m "feat(reference): 实现地址解析与导出"
```

## Task 3: 接入 attach/auto-start MCP 两种模式并打通 CLI 主流程

**Files:**
- Modify: `generate_reference_yaml.py`
- Modify: `tests/test_generate_reference_yaml.py`

- [ ] **Step 1: 先补失败测试，锁定 attach 模式、auto-start 模式与主流程编排**

```python
from contextlib import asynccontextmanager
from types import SimpleNamespace
from unittest.mock import patch


class _FakeStreamableHttpClient:
    async def __aenter__(self):
        return ("read-stream", "write-stream", None)

    async def __aexit__(self, exc_type, exc, tb):
        return False


class _FakeAsyncClient:
    def __init__(self, *args, **kwargs):
        pass

    async def __aenter__(self):
        return self

    async def __aexit__(self, exc_type, exc, tb):
        return False


class _FakeClientSession:
    def __init__(self, read_stream, write_stream):
        self.read_stream = read_stream
        self.write_stream = write_stream

    async def __aenter__(self):
        return self

    async def __aexit__(self, exc_type, exc, tb):
        return False

    async def initialize(self):
        return None


class TestMcpSessionModes(unittest.IsolatedAsyncioTestCase):
    @patch.object(generate_reference_yaml, "check_mcp_health", AsyncMock(return_value=True))
    @patch.object(generate_reference_yaml, "httpx")
    @patch.object(generate_reference_yaml, "streamable_http_client", return_value=_FakeStreamableHttpClient())
    @patch.object(generate_reference_yaml, "ClientSession", _FakeClientSession)
    async def test_attach_existing_mcp_session_checks_health_first(
        self,
        _mock_streamable,
        mock_httpx,
        mock_health,
    ) -> None:
        mock_httpx.AsyncClient = _FakeAsyncClient

        async with generate_reference_yaml.attach_existing_mcp_session(
            host="127.0.0.1",
            port=13337,
            debug=False,
        ) as session:
            self.assertIsInstance(session, _FakeClientSession)

        mock_health.assert_awaited_once_with("127.0.0.1", 13337)

    @patch.object(generate_reference_yaml, "quit_ida_gracefully")
    @patch.object(generate_reference_yaml, "start_idalib_mcp", return_value=object())
    @patch.object(generate_reference_yaml, "httpx")
    @patch.object(generate_reference_yaml, "streamable_http_client", return_value=_FakeStreamableHttpClient())
    @patch.object(generate_reference_yaml, "ClientSession", _FakeClientSession)
    async def test_autostart_mcp_session_starts_and_quits_process(
        self,
        _mock_streamable,
        mock_httpx,
        mock_start,
        mock_quit,
    ) -> None:
        mock_httpx.AsyncClient = _FakeAsyncClient

        async with generate_reference_yaml.autostart_mcp_session(
            binary_path="bin/14141/engine/engine2.dll",
            host="127.0.0.1",
            port=13337,
            ida_args="",
            debug=False,
        ) as session:
            self.assertIsInstance(session, _FakeClientSession)

        mock_start.assert_called_once()
        mock_quit.assert_called_once()


class TestRunReferenceGeneration(unittest.IsolatedAsyncioTestCase):
    async def test_run_reference_generation_uses_attach_mode_by_default(self) -> None:
        args = SimpleNamespace(
            gamever="14141",
            module="engine",
            platform="windows",
            func_name="Foo",
            mcp_host="127.0.0.1",
            mcp_port=13337,
            binary=None,
            auto_start_mcp=False,
            ida_args="",
            debug=False,
        )

        fake_session = AsyncMock()

        @asynccontextmanager
        async def _fake_attach(*args, **kwargs):
            yield fake_session

        with (
            patch.object(generate_reference_yaml, "attach_existing_mcp_session", _fake_attach),
            patch.object(generate_reference_yaml, "resolve_func_va", AsyncMock(return_value="0x180123450")),
            patch.object(
                generate_reference_yaml,
                "export_reference_payload_via_mcp",
                AsyncMock(
                    return_value={
                        "func_name": "Foo",
                        "func_va": "0x180123450",
                        "disasm_code": "mov eax, 1",
                        "procedure": "",
                    }
                ),
            ),
        ):
            with tempfile.TemporaryDirectory() as temp_dir:
                output_path = await generate_reference_yaml.run_reference_generation(
                    args,
                    repo_root=Path(temp_dir),
                )

        self.assertEqual(
            Path(temp_dir)
            / "ida_preprocessor_scripts"
            / "references"
            / "engine"
            / "Foo.windows.yaml",
            output_path,
        )
```

- [ ] **Step 2: 运行会话编排测试，确认当前失败**

Run:

```bash
uv run python -m unittest \
  tests.test_generate_reference_yaml.TestMcpSessionModes \
  tests.test_generate_reference_yaml.TestRunReferenceGeneration \
  -v
```

Expected: FAIL，因为 attach/auto-start 会话管理器和 `run_reference_generation(...)` 还未实现。

- [ ] **Step 3: 实现 MCP 会话适配、主流程编排与 CLI 入口**

```python
import asyncio
import httpx
from contextlib import asynccontextmanager

from ida_analyze_bin import check_mcp_health, quit_ida_gracefully, start_idalib_mcp

try:
    from mcp import ClientSession
    from mcp.client.streamable_http import streamable_http_client
except ImportError:
    ClientSession = None
    streamable_http_client = None


@asynccontextmanager
async def _open_mcp_session(host: str, port: int):
    server_url = f"http://{host}:{port}/mcp"
    async with httpx.AsyncClient(
        follow_redirects=True,
        timeout=httpx.Timeout(30.0, read=300.0),
        trust_env=False,
    ) as http_client:
        async with streamable_http_client(server_url, http_client=http_client) as (
            read_stream,
            write_stream,
            _,
        ):
            async with ClientSession(read_stream, write_stream) as session:
                await session.initialize()
                yield session


@asynccontextmanager
async def attach_existing_mcp_session(*, host: str, port: int, debug: bool):
    healthy = await check_mcp_health(host, port)
    if not healthy:
        raise ReferenceGenerationError(
            f"MCP server is not reachable at {host}:{port}"
        )
    async with _open_mcp_session(host, port) as session:
        yield session


@asynccontextmanager
async def autostart_mcp_session(*, binary_path: str, host: str, port: int, ida_args: str, debug: bool):
    process = start_idalib_mcp(binary_path, host, port, ida_args, debug)
    if process is None:
        raise ReferenceGenerationError(f"failed to start idalib-mcp for {binary_path}")
    try:
        async with _open_mcp_session(host, port) as session:
            yield session
    finally:
        quit_ida_gracefully(process, host, port, debug=debug)


async def run_reference_generation(args, *, repo_root: Path | None = None) -> Path:
    repo_root = Path(repo_root or Path(__file__).resolve().parent)
    session_manager = (
        autostart_mcp_session(
            binary_path=args.binary,
            host=args.mcp_host,
            port=args.mcp_port,
            ida_args=args.ida_args,
            debug=args.debug,
        )
        if args.auto_start_mcp
        else attach_existing_mcp_session(
            host=args.mcp_host,
            port=args.mcp_port,
            debug=args.debug,
        )
    )
    async with session_manager as session:
        func_va = await resolve_func_va(
            session=session,
            repo_root=repo_root,
            gamever=args.gamever,
            module=args.module,
            platform=args.platform,
            func_name=args.func_name,
            debug=args.debug,
        )
        payload = await export_reference_payload_via_mcp(
            session=session,
            func_name=args.func_name,
            func_va=func_va,
            debug=args.debug,
        )
        output_path = build_reference_output_path(
            repo_root=repo_root,
            module=args.module,
            func_name=args.func_name,
            platform=args.platform,
        )
        write_reference_yaml(output_path, payload)
        return output_path


def main(argv=None) -> int:
    args = parse_args(argv)
    try:
        output_path = asyncio.run(run_reference_generation(args))
    except ReferenceGenerationError as exc:
        print(f"ERROR: {exc}")
        return 1
    print(f"Generated reference YAML: {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
```

- [ ] **Step 4: 重跑整套 `generate_reference_yaml` 定向测试并做语法检查**

Run:

```bash
uv run python -m py_compile generate_reference_yaml.py && \
uv run python -m unittest tests.test_generate_reference_yaml -v
```

Expected: PASS，attach/auto-start 两种模式、解析优先级、reference payload 导出与主流程编排全部通过。

- [ ] **Step 5: 提交 CLI 主流程实现**

```bash
git add generate_reference_yaml.py tests/test_generate_reference_yaml.py
git commit -m "feat(reference): 增加 reference yaml 生成 CLI"
```

## Task 4: 补 project-level SKILL 与 README 准备步骤文档

**Files:**
- Create: `.claude/skills/generate-reference-yaml/SKILL.md`
- Modify: `README.md`
- Modify: `README_CN.md`

- [ ] **Step 1: 新建 project-level SKILL，只保留 CLI 触发与人工检查指引**

````markdown
---
name: generate-reference-yaml
description: |
  Generate minimal reference YAML for LLM_DECOMPILE inputs by calling the project CLI.
  Use this skill when you need `ida_preprocessor_scripts/references/<module>/<func_name>.<platform>.yaml`.
  Trigger: generate reference yaml, reference yaml, LLM_DECOMPILE reference
disable-model-invocation: true
---

# Generate Reference YAML

Use the project CLI as the single backend. Do not call IDA APIs directly from this SKILL.

## Required parameters

- `gamever`
- `module`
- `platform`
- `func_name`

## Attach to existing MCP

```bash
uv run generate_reference_yaml.py \
  -gamever <gamever> \
  -module <module> \
  -platform <platform> \
  -func_name <func_name> \
  -mcp_host 127.0.0.1 \
  -mcp_port 13337
```

## Auto-start `idalib-mcp`

```bash
uv run generate_reference_yaml.py \
  -gamever <gamever> \
  -module <module> \
  -platform <platform> \
  -func_name <func_name> \
  -auto_start_mcp \
  -binary bin/<gamever>/<module>/<binary_name>
```

## After generation

1. Verify `func_name`, `func_va`, `disasm_code`, `procedure`
2. Update the target `find-*.py` script `LLM_DECOMPILE` reference path
3. Continue the unfinished reverse-engineering task
````

- [ ] **Step 2: 在 README/README_CN 新增“reference YAML 准备步骤”与命令示例**

````markdown
## Generate reference YAML for `LLM_DECOMPILE`

Reference YAML is stored under:

```text
ida_preprocessor_scripts/references/<module>/<func_name>.<platform>.yaml
```

Preparation flow:

1. Confirm the target function already has current-version YAML with `func_va`, or can be found by `config.yaml` symbol name / alias in IDA.
2. Run:

```bash
uv run generate_reference_yaml.py \
  -gamever 14141 \
  -module engine \
  -platform windows \
  -func_name CNetworkGameClient_RecordEntityBandwidth \
  -mcp_host 127.0.0.1 \
  -mcp_port 13337
```

3. Check the generated file:
   - `func_name` is correct
   - `func_va` is trusted
   - `disasm_code` is not empty
   - `procedure` is acceptable, empty string is allowed when Hex-Rays is unavailable
4. Reference the path from `LLM_DECOMPILE` in the target `find-*.py` script.
````

- [ ] **Step 3: 跑最终定向验证，并记录真实 IDA 手动验收动作**

Run:

```bash
uv run python -m py_compile generate_reference_yaml.py && \
uv run python -m unittest tests.test_generate_reference_yaml -v
```

Manual acceptance:

```bash
uv run generate_reference_yaml.py \
  -gamever 14141 \
  -module engine \
  -platform windows \
  -func_name CNetworkGameClient_RecordEntityBandwidth \
  -mcp_host 127.0.0.1 \
  -mcp_port 13337
```

Expected:

- 生成 `ida_preprocessor_scripts/references/engine/CNetworkGameClient_RecordEntityBandwidth.windows.yaml`
- YAML 只有 `func_name`、`func_va`、`disasm_code`、`procedure`
- `disasm_code` 非空
- `procedure` 允许为空字符串，但字段必须存在

- [ ] **Step 4: 提交 SKILL 与文档**

```bash
git add \
  .claude/skills/generate-reference-yaml/SKILL.md \
  README.md \
  README_CN.md
git commit -m "docs(reference): 增加 reference yaml 生成说明"
```

## Self-Review

- **Spec coverage:** 本计划覆盖了 `docs/superpowers/specs/2026-04-09-reference-yaml-generator-design.md` 的全部核心要求：独立 CLI、attach/auto-start 双模式、旧 YAML `func_va` 优先、`config.yaml` + alias fallback、最小 reference schema、路径包含 `module/platform`、project-level SKILL、README/README_CN 的 reference YAML 准备步骤与首个真实验收场景。
- **Placeholder scan:** 计划正文中没有 `TBD`、`TODO`、`implement later`、`similar to` 等占位语；每个任务都给出了明确文件路径、测试命令、期望结果与关键代码骨架。
- **Type consistency:** 计划中统一使用 `ReferenceGenerationError`、`build_reference_output_path(...)`、`load_existing_func_va(...)`、`load_symbol_aliases(...)`、`resolve_func_va(...)`、`export_reference_payload_via_mcp(...)`、`attach_existing_mcp_session(...)`、`autostart_mcp_session(...)`、`run_reference_generation(...)` 这一组函数名，后续执行时不得改名漂移。
