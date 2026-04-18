# LLM Decompile Fallback Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为预处理链路增加 `call_llm_decompile` 回退能力，统一 `llm_*` CLI 配置，复用 `vcall_finder` 的底层 LLM 调用骨架，并先接入 `find-CNetworkMessages_FindNetworkGroup.py` 作为首个真实样例。

**Architecture:** 先在主入口中把 `vcall_finder_*` 配置统一替换为 `llm_*`，并抽出新的 `ida_llm_utils.py` 承载公共 OpenAI 调用骨架，再将 `ida_vcall_finder.py` 切到共享 helper。随后扩展 `ida_skill_preprocessor.py` 与 `ida_analyze_util.py`，让脚本可以声明 `LLM_DECOMPILE`，并把 `found_call`、`found_vcall`、`found_gv`、`found_struct_offset` 直接转换成当前版本 YAML。最后接入 `find-CNetworkMessages_FindNetworkGroup.py`、新增 prompt 文件，并用现有 `unittest` 风格补齐参数解析、helper、预处理与脚本回归测试。

**Tech Stack:** Python 3.10, `argparse`, `unittest`, `unittest.mock`, `PyYAML`, OpenAI Python SDK, MCP `py_eval`

---

## File Map

- `ida_analyze_bin.py`
  - 把 `DEFAULT_VCALL_FINDER_MODEL` 改为 `DEFAULT_LLM_MODEL`
  - 删除 `-vcall_finder_model` / `-vcall_finder_apikey` / `-vcall_finder_baseurl`
  - 新增 `-llm_model` / `-llm_apikey` / `-llm_baseurl`
  - 将统一 LLM 配置传给 `preprocess_single_skill_via_mcp(...)` 与 `aggregate_vcall_results_for_object(...)`
- `ida_llm_utils.py`
  - 新增共享 LLM helper
  - 承载 `create_openai_client(...)`、统一 response 文本提取、chat completion 调用、debug 输出
- `ida_vcall_finder.py`
  - 删除本地 `create_openai_client(...)`
  - 删除本地通用 chat completion 骨架
  - 改为复用 `ida_llm_utils.py`
  - 保留 `render_vcall_prompt(...)` 与 `parse_llm_vcall_response(...)`
- `ida_skill_preprocessor.py`
  - 扩展 `preprocess_single_skill_via_mcp(...)` 签名
  - 仅当脚本 `preprocess_skill(...)` 声明 `llm_config` 参数时才下传
- `ida_analyze_util.py`
  - 扩展 `preprocess_common_skill(...)` 参数
  - 增加 `call_llm_decompile(...)`
  - 增加 `parse_llm_decompile_response(...)`
  - 增加当前版 detail 导出 helper
  - 扩展 `preprocess_func_sig_via_mcp(...)` 以支持 `found_call` / `found_vcall` 直生
  - 新增 `preprocess_gen_struct_offset_sig_via_mcp(...)`
- `ida_preprocessor_scripts/find-CNetworkMessages_FindNetworkGroup.py`
  - 增加 `LLM_DECOMPILE`
  - 增加可选 `llm_config` 参数
  - 传入 `llm_decompile_specs` 与 `llm_config`
- `ida_preprocessor_scripts/prompt/call_llm_decompile.md`
  - 新增 prompt 模板文件
  - 内容迁移自 `docs/call_llm_decompile_prompt.md`
- `tests/test_ida_analyze_bin.py`
  - 增加 `llm_*` 参数解析与旧参数报错测试
- `tests/test_ida_vcall_finder.py`
  - 增加对共享 helper 的接线回归测试
- `tests/test_ida_preprocessor_scripts.py`
  - 增加 `find-CNetworkMessages_FindNetworkGroup.py` 对 `LLM_DECOMPILE` 与 `llm_config` 的转发测试
- `tests/test_ida_analyze_util.py`
  - 增加 `parse_llm_decompile_response(...)` 测试
  - 增加 `found_call` / `found_vcall` / `found_gv` / `found_struct_offset` 直生测试
- `tests/test_ida_llm_utils.py`
  - 新增共享 helper 测试
- `docs/superpowers/specs/2026-04-09-llm-decompile-design.md`
  - 只作为核对依据，不改内容

## Task 1: 先锁定 CLI 与共享 LLM helper 测试

**Files:**
- Create: `tests/test_ida_llm_utils.py`
- Modify: `tests/test_ida_analyze_bin.py`
- Modify: `ida_analyze_bin.py:65`
- Modify: `ida_vcall_finder.py:243`

- [x] **Step 1: 为共享 helper 写失败测试**

```python
import types
import unittest
from unittest.mock import patch

import ida_llm_utils


class _FakeMessage:
    def __init__(self, content):
        self.content = content


class _FakeChoice:
    def __init__(self, content):
        self.message = _FakeMessage(content)


class _FakeResponse:
    def __init__(self, content):
        self.choices = [_FakeChoice(content)]


class TestIdaLlmUtils(unittest.TestCase):
    def test_create_openai_client_requires_api_key(self) -> None:
        with self.assertRaises(RuntimeError):
            ida_llm_utils.create_openai_client(
                api_key=None,
                base_url=None,
                api_key_required_message="-llm_apikey is required when LLM workflow is enabled",
            )

    def test_extract_message_text_supports_string_content(self) -> None:
        response = _FakeResponse("```yaml\\nfound_vcall: []\\n```")
        self.assertEqual(
            "```yaml\\nfound_vcall: []\\n```",
            ida_llm_utils.extract_first_message_text(response),
        )

    def test_extract_message_text_rejects_empty_choices(self) -> None:
        response = types.SimpleNamespace(choices=[])
        with self.assertRaises(ValueError):
            ida_llm_utils.extract_first_message_text(response)


if __name__ == "__main__":
    unittest.main()
```

- [x] **Step 2: 为 `llm_*` 参数写失败测试**

```python
class TestParseArgsLlmOptions(unittest.TestCase):
    def test_parse_args_accepts_llm_options(self) -> None:
        with patch(
            "sys.argv",
            [
                "ida_analyze_bin.py",
                "-gamever=14141",
                "-llm_model=gpt-4o",
                "-llm_apikey=test-key",
                "-llm_baseurl=https://api.example.com/v1",
            ],
        ):
            args = ida_analyze_bin.parse_args()

        self.assertEqual("gpt-4o", args.llm_model)
        self.assertEqual("test-key", args.llm_apikey)
        self.assertEqual("https://api.example.com/v1", args.llm_baseurl)

    def test_parse_args_rejects_legacy_vcall_finder_llm_options(self) -> None:
        with patch(
            "sys.argv",
            [
                "ida_analyze_bin.py",
                "-gamever=14141",
                "-vcall_finder_model=gpt-4o",
            ],
        ), self.assertRaises(SystemExit):
            ida_analyze_bin.parse_args()
```

- [x] **Step 3: 运行新增测试，确认当前会失败**

Run:

```bash
uv run python -m unittest tests.test_ida_llm_utils tests.test_ida_analyze_bin -v
```

Expected: FAIL，因为 `ida_llm_utils.py` 不存在，且 `parse_args()` 还未提供 `llm_*`。

- [ ] **Step 4: 提交失败测试**

```bash
git add tests/test_ida_llm_utils.py tests/test_ida_analyze_bin.py
git commit -m "test(llm): 增加统一配置与共享辅助测试"
```

## Task 2: 实现统一 `llm_*` 配置并抽出共享 helper

**Files:**
- Create: `ida_llm_utils.py`
- Modify: `ida_analyze_bin.py:31-70`
- Modify: `ida_analyze_bin.py:388-405`
- Modify: `ida_analyze_bin.py:1116`
- Modify: `ida_analyze_bin.py:1331-1337`
- Modify: `ida_vcall_finder.py:12-15`
- Modify: `ida_vcall_finder.py:243-292`
- Test: `tests/test_ida_llm_utils.py`
- Test: `tests/test_ida_analyze_bin.py`

- [x] **Step 1: 在主入口中统一常量与 CLI 参数**

```python
DEFAULT_LLM_MODEL = "gpt-4o"

parser.add_argument(
    "-llm_model",
    default=DEFAULT_LLM_MODEL,
    help=f"OpenAI-compatible model for LLM workflows (default: {DEFAULT_LLM_MODEL})",
)
parser.add_argument(
    "-llm_apikey",
    default=None,
    help="OpenAI-compatible API key used by LLM workflows",
)
parser.add_argument(
    "-llm_baseurl",
    default=None,
    help="Optional OpenAI-compatible base URL used by LLM workflows",
)
```

- [x] **Step 2: 新增共享 helper 模块**

```python
#!/usr/bin/env python3

from __future__ import annotations

import time
from typing import Any

from openai import OpenAI


def require_nonempty_text(value: Any, name: str) -> str:
    text = "" if value is None else str(value).strip()
    if not text:
        raise ValueError(f"{name} cannot be empty")
    return text


def create_openai_client(api_key, base_url=None, *, api_key_required_message: str) -> OpenAI:
    if api_key is None or not str(api_key).strip():
        raise RuntimeError(api_key_required_message)

    client_kwargs = {"api_key": require_nonempty_text(api_key, "api_key")}
    if base_url is not None:
        client_kwargs["base_url"] = require_nonempty_text(base_url, "base_url")
    return OpenAI(**client_kwargs)


def extract_first_message_text(response: Any) -> str:
    choices = getattr(response, "choices", None) or []
    if not choices:
        raise ValueError("OpenAI response missing choices")
    message = getattr(choices[0], "message", None)
    content = getattr(message, "content", "") if message is not None else ""
    return content if isinstance(content, str) else str(content)


def call_llm_text(client, *, model, messages, temperature=0.1):
    return extract_first_message_text(
        client.chat.completions.create(
            model=require_nonempty_text(model, "model"),
            messages=messages,
            temperature=temperature,
        )
    )
```

- [x] **Step 3: 让 `ida_vcall_finder.py` 复用共享 helper**

```python
from ida_llm_utils import call_llm_text, create_openai_client


def call_openai_for_vcalls(client, detail, model, *, debug=False, request_label=""):
    if debug:
        _print_vcall_debug(
            f"LLM request start {request_label} model='{model}'".rstrip(),
            debug,
        )

    started_at = time.monotonic()
    content = call_llm_text(
        client,
        model=model,
        messages=[
            {"role": "system", "content": "You are a reverse engineering expert."},
            {"role": "user", "content": render_vcall_prompt(detail)},
        ],
        temperature=0.1,
    )
    found_vcall = parse_llm_vcall_response(content)["found_vcall"]
    if debug:
        elapsed_seconds = time.monotonic() - started_at
        _print_vcall_debug(
            "LLM request done "
            f"{request_label} elapsed={elapsed_seconds:.2f}s "
            f"response_chars={len(content)} found_vcall={len(found_vcall)}",
            debug,
        )
    return found_vcall
```

- [x] **Step 4: 主流程改为向预处理与聚合都传统一配置**

```python
preprocess_ok = asyncio.run(
    preprocess_single_skill_via_mcp(
        host,
        port,
        skill_name,
        expected_outputs,
        old_yaml_map,
        binary_dir,
        platform,
        llm_model=args.llm_model,
        llm_apikey=args.llm_apikey,
        llm_baseurl=args.llm_baseurl,
        debug=debug,
    )
)

stats = aggregate_vcall_results_for_object(
    base_dir="vcall_finder",
    gamever=gamever,
    object_name=object_name,
    model=args.llm_model,
    api_key=args.llm_apikey,
    base_url=args.llm_baseurl,
    debug=debug,
)
```

- [x] **Step 5: 运行测试，确认共享 helper 与参数解析通过**

Run:

```bash
uv run python -m unittest tests.test_ida_llm_utils tests.test_ida_analyze_bin tests.test_ida_vcall_finder -v
```

Expected: PASS，新参数解析通过，旧参数继续报错，`vcall_finder` 测试不再依赖本地 `OpenAI` 构造逻辑。

- [ ] **Step 6: 提交共享配置与 helper**

```bash
git add ida_analyze_bin.py ida_llm_utils.py ida_vcall_finder.py tests/test_ida_llm_utils.py tests/test_ida_analyze_bin.py tests/test_ida_vcall_finder.py
git commit -m "feat(llm): 统一预处理与聚合配置"
```

## Task 3: 让脚本调度层支持 `llm_config`

**Files:**
- Modify: `ida_skill_preprocessor.py:30-147`
- Modify: `tests/test_ida_preprocessor_scripts.py`
- Test: `tests/test_ida_preprocessor_scripts.py`

- [x] **Step 1: 为脚本转发 `llm_config` 写失败测试**

```python
FIND_NETWORK_GROUP_SCRIPT_PATH = Path(
    "ida_preprocessor_scripts/find-CNetworkMessages_FindNetworkGroup.py"
)


class TestFindCNetworkMessagesFindNetworkGroup(unittest.IsolatedAsyncioTestCase):
    async def test_preprocess_skill_forwards_llm_specs_and_config(self) -> None:
        module = _load_module(
            FIND_NETWORK_GROUP_SCRIPT_PATH,
            "find_CNetworkMessages_FindNetworkGroup",
        )
        mock_preprocess_common_skill = AsyncMock(return_value=True)
        llm_config = {"model": "gpt-4o", "api_key": "k", "base_url": "https://api.example.com/v1"}

        with patch.object(module, "preprocess_common_skill", mock_preprocess_common_skill):
            result = await module.preprocess_skill(
                session="session",
                skill_name="skill",
                expected_outputs=["out.yaml"],
                old_yaml_map={"k": "v"},
                new_binary_dir="bin_dir",
                platform="windows",
                image_base=0x180000000,
                llm_config=llm_config,
                debug=True,
            )

        self.assertTrue(result)
        mock_preprocess_common_skill.assert_awaited_once()
        self.assertEqual(llm_config, mock_preprocess_common_skill.await_args.kwargs["llm_config"])
        self.assertIn("llm_decompile_specs", mock_preprocess_common_skill.await_args.kwargs)
```

- [x] **Step 2: 扩展 `preprocess_single_skill_via_mcp(...)`，按脚本签名传递 `llm_config`**

```python
async def preprocess_single_skill_via_mcp(
    host,
    port,
    skill_name,
    expected_outputs,
    old_yaml_map,
    new_binary_dir,
    platform,
    llm_model=None,
    llm_apikey=None,
    llm_baseurl=None,
    debug=False,
):
    preprocess_func = _get_preprocess_entry(skill_name, debug=debug)
    if preprocess_func is None:
        return False

    server_url = f"http://{host}:{port}/mcp"
    llm_config = {
        "model": llm_model,
        "api_key": llm_apikey,
        "base_url": llm_baseurl,
    }
    call_kwargs = dict(
        skill_name=skill_name,
        expected_outputs=expected_outputs,
        old_yaml_map=old_yaml_map,
        new_binary_dir=new_binary_dir,
        platform=platform,
        debug=debug,
    )
    if "llm_config" in inspect.signature(preprocess_func).parameters:
        call_kwargs["llm_config"] = llm_config
    async with httpx.AsyncClient(
        follow_redirects=True,
        timeout=httpx.Timeout(30.0, read=300.0),
        trust_env=False,
    ) as http_client:
        async with streamable_http_client(server_url, http_client=http_client) as (read_stream, write_stream, _):
            async with ClientSession(read_stream, write_stream) as session:
                await session.initialize()
                ib_result = await session.call_tool(
                    name="py_eval",
                    arguments={"code": "hex(idaapi.get_imagebase())"},
                )
                ib_data = parse_mcp_result(ib_result)
                image_base = int(ib_data.get("result", "0x0"), 16) if isinstance(ib_data, dict) else int(str(ib_data), 16)
                call_kwargs["session"] = session
                call_kwargs["image_base"] = image_base
                result = preprocess_func(**call_kwargs)
                if inspect.isawaitable(result):
                    result = await result
                return bool(result)
```

- [x] **Step 3: 运行脚本转发测试**

Run:

```bash
uv run python -m unittest tests.test_ida_preprocessor_scripts -v
```

Expected: PASS，现有脚本继续工作，`find-CNetworkMessages_FindNetworkGroup.py` 可选择性接收 `llm_config`。

- [ ] **Step 4: 提交调度层改动**

```bash
git add ida_skill_preprocessor.py tests/test_ida_preprocessor_scripts.py
git commit -m "feat(preprocess): 下传统一 llm 配置"
```

## Task 4: 先写 `ida_analyze_util.py` 的 LLM decompile 失败测试

**Files:**
- Modify: `tests/test_ida_analyze_util.py`
- Modify: `ida_analyze_util.py:461-520`
- Modify: `ida_analyze_util.py:2825-3265`

- [x] **Step 1: 为响应解析写失败测试**

```python
class TestParseLlmDecompileResponse(unittest.TestCase):
    def test_parse_llm_decompile_response_normalizes_all_sections(self) -> None:
        payload = """
```yaml
found_vcall:
  - insn_va: '0x180777700'
    insn_disasm: call    [rax+68h]
    vfunc_offset: '0x68'
    func_name: ILoopMode_OnLoopActivate
found_call:
  - insn_va: '0x180888800'
    insn_disasm: call    sub_180999900
    func_name: CLoopModeGame_RegisterEventMapInternal
found_gv:
  - insn_va: '0x180444400'
    insn_disasm: mov rcx, cs:qword_180666600
    gv_name: s_GameEventManager
found_struct_offset:
  - insn_va: '0x1801BA12A'
    insn_disasm: mov rcx, [r14+58h]
    offset: '0x58'
    struct_name: CGameResourceService
    member_name: m_pEntitySystem
```
"""
        parsed = ida_analyze_util.parse_llm_decompile_response(payload)
        self.assertEqual("0x68", parsed["found_vcall"][0]["vfunc_offset"])
        self.assertEqual(
            "CLoopModeGame_RegisterEventMapInternal",
            parsed["found_call"][0]["func_name"],
        )
        self.assertEqual("CGameResourceService", parsed["found_struct_offset"][0]["struct_name"])
```

- [x] **Step 2: 为 `found_vcall` / `found_gv` / `found_struct_offset` 直生写失败测试**

```python
class TestApplyLlmDecompileResults(unittest.IsolatedAsyncioTestCase):
    async def test_preprocess_common_skill_uses_found_vcall_to_generate_func_yaml(self) -> None:
        with patch.object(
            ida_analyze_util,
            "preprocess_func_sig_via_mcp",
            AsyncMock(side_effect=[None, {
                "func_name": "CNetworkMessages_FindNetworkGroup",
                "func_va": "0x180010780",
                "func_rva": "0x10780",
                "func_size": "0x40",
                "func_sig": "AA BB CC DD",
                "vtable_name": "CNetworkMessages",
                "vfunc_offset": "0x78",
                "vfunc_index": 15,
            }]),
        ), patch.object(
            ida_analyze_util,
            "call_llm_decompile",
            AsyncMock(return_value={
                "found_vcall": [{
                    "insn_va": "0x180020000",
                    "insn_disasm": "call    [rax+78h]",
                    "vfunc_offset": "0x78",
                    "func_name": "CNetworkMessages_FindNetworkGroup",
                }],
                "found_call": [],
                "found_gv": [],
                "found_struct_offset": [],
            }),
        ), patch.object(
            ida_analyze_util,
            "write_func_yaml",
        ) as mock_write_func_yaml, patch.object(
            ida_analyze_util,
            "_rename_func_in_ida",
            AsyncMock(return_value=None),
        ):
            result = await ida_analyze_util.preprocess_common_skill(
                session="session",
                expected_outputs=["/tmp/CNetworkMessages_FindNetworkGroup.windows.yaml"],
                old_yaml_map={"/tmp/CNetworkMessages_FindNetworkGroup.windows.yaml": "/tmp/old.yaml"},
                new_binary_dir="/tmp",
                platform="windows",
                image_base=0x180000000,
                func_names=["CNetworkMessages_FindNetworkGroup"],
                func_vtable_relations=[("CNetworkMessages_FindNetworkGroup", "CNetworkMessages", True)],
                llm_decompile_specs=[("CNetworkMessages_FindNetworkGroup", "prompt/call_llm_decompile.md", "references/CNetworkMessages_FindNetworkGroup.reference.yaml")],
                llm_config={"model": "gpt-4o", "api_key": "k", "base_url": None},
                debug=True,
            )

        self.assertTrue(result)
        written_payload = mock_write_func_yaml.call_args.args[1]
        self.assertEqual("0x78", written_payload["vfunc_offset"])
```

- [x] **Step 3: 运行测试确认当前失败**

Run:

```bash
uv run python -m unittest tests.test_ida_analyze_util -v
```

Expected: FAIL，因为 `parse_llm_decompile_response(...)`、`call_llm_decompile(...)` 和直生路径还不存在。

- [ ] **Step 4: 提交失败测试**

```bash
git add tests/test_ida_analyze_util.py
git commit -m "test(preprocess): 增加 llm 反编译回退测试"
```

## Task 5: 实现 `call_llm_decompile`、四类直生与 struct-offset 生成

**Files:**
- Modify: `ida_analyze_util.py:461-843`
- Modify: `ida_analyze_util.py:1147-1275`
- Modify: `ida_analyze_util.py:1835-2095`
- Modify: `ida_analyze_util.py:2825-3265`
- Test: `tests/test_ida_analyze_util.py`

- [x] **Step 1: 给 `preprocess_func_sig_via_mcp(...)` 增加直接地址与 vfunc 偏移入口**

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
    direct_func_va=None,
    direct_vtable_class=None,
    direct_vfunc_offset=None,
):
    if direct_func_va is not None:
        generated = await preprocess_gen_func_sig_via_mcp(
            session=session,
            func_va=direct_func_va,
            image_base=image_base,
            debug=debug,
        )
        if generated is None:
            return None
        new_data = {"func_name": func_name, **generated}
        if direct_vtable_class is not None and direct_vfunc_offset is not None:
            offset_value = int(str(direct_vfunc_offset), 0)
            new_data["vtable_name"] = direct_vtable_class
            new_data["vfunc_offset"] = hex(offset_value)
            new_data["vfunc_index"] = offset_value // 8
        return new_data
    if yaml is None:
        if debug:
            print("    Preprocess: PyYAML is required for func_sig preprocessing")
        return None

    if not old_path or not os.path.exists(old_path):
        if debug:
            print(f"    Preprocess: no old YAML for {os.path.basename(new_path)}")
        return None

    with open(old_path, "r", encoding="utf-8") as handle:
        old_data = yaml.safe_load(handle)
    if not old_data or not isinstance(old_data, dict):
        return None
    # 其余旧版 `func_sig` / `vfunc_sig` 复用逻辑保持原样
```

- [x] **Step 2: 增加 `parse_llm_decompile_response(...)` 与 `call_llm_decompile(...)`**

```python
def _parse_yaml_mapping_from_response(response_text: str | None) -> dict[str, object]:
    text = (response_text or "").strip()
    if not text:
        return {}
    matches = re.findall(r"```(?:yaml|yml)?\s*(.*?)```", text, re.IGNORECASE | re.DOTALL)
    candidates = matches or [text]
    for candidate in candidates:
        try:
            parsed = yaml.load(candidate.strip(), Loader=yaml.BaseLoader)
        except yaml.YAMLError:
            continue
        if isinstance(parsed, dict):
            return parsed
        if parsed is None:
            return {}
    return {}


def _normalize_llm_entries(entries, required_keys):
    normalized = []
    for entry in entries or []:
        if not isinstance(entry, dict):
            continue
        item = {key: str(entry.get(key, "")).strip() for key in required_keys}
        if all(item.values()):
            normalized.append(item)
    return normalized


def _collect_symbol_names(reference_detail, target_detail):
    names = set()
    for payload in (reference_detail, target_detail):
        if isinstance(payload, dict):
            for key in ("func_name", "gv_name", "struct_name", "member_name"):
                value = str(payload.get(key, "")).strip()
                if value:
                    names.add(value)
    return names


def parse_llm_decompile_response(response_text: str | None) -> dict[str, list[dict[str, str]]]:
    parsed = _parse_yaml_mapping_from_response(response_text)
    return {
        "found_vcall": _normalize_llm_entries(parsed.get("found_vcall", []), ("insn_va", "insn_disasm", "vfunc_offset", "func_name")),
        "found_call": _normalize_llm_entries(parsed.get("found_call", []), ("insn_va", "insn_disasm", "func_name")),
        "found_gv": _normalize_llm_entries(parsed.get("found_gv", []), ("insn_va", "insn_disasm", "gv_name")),
        "found_struct_offset": _normalize_llm_entries(parsed.get("found_struct_offset", []), ("insn_va", "insn_disasm", "offset", "struct_name", "member_name")),
    }


def call_llm_decompile(client, target_detail, reference_detail, prompt_text, model, *, debug=False, request_label=""):
    content = call_llm_text(
        client,
        model=model,
        messages=[
            {"role": "system", "content": "You are a reverse engineering expert."},
            {"role": "user", "content": prompt_text.format(
                symbol_name_list=", ".join(sorted(_collect_symbol_names(reference_detail, target_detail))),
                disasm_for_reference=reference_detail.get("disasm_code", ""),
                procedure_for_reference=reference_detail.get("procedure", ""),
                disasm_code=target_detail.get("disasm_code", ""),
                procedure=target_detail.get("procedure", ""),
            )},
        ],
        temperature=0.1,
    )
    return parse_llm_decompile_response(content)
```

- [x] **Step 3: 新增 `preprocess_gen_struct_offset_sig_via_mcp(...)`**

```python
async def _generate_instruction_signature(
    session,
    *,
    inst_va,
    func_va,
    image_base,
    debug=False,
):
    # 将 `preprocess_gen_gv_sig_via_mcp()` 中“从已知指令位置收集指令字节并搜索最短唯一签名”的逻辑提取到这里，
    # 返回统一结构：{"sig": "...", "sig_disp": 0}
    return await _generate_signature_from_known_instruction(
        session=session,
        inst_va=inst_va,
        func_va=func_va,
        image_base=image_base,
        debug=debug,
    )


async def preprocess_gen_struct_offset_sig_via_mcp(
    session,
    struct_name,
    member_name,
    offset,
    image_base,
    access_inst_va=None,
    access_func_va=None,
    member_size=None,
    debug=False,
):
    generated = await _generate_instruction_signature(
        session=session,
        inst_va=access_inst_va,
        func_va=access_func_va,
        image_base=image_base,
        debug=debug,
    )
    if generated is None:
        return None
    payload = {
        "struct_name": str(struct_name),
        "member_name": str(member_name),
        "offset": hex(int(str(offset), 0)),
        "offset_sig": generated["sig"],
        "offset_sig_disp": generated["sig_disp"],
    }
    if member_size is not None:
        payload["size"] = int(member_size)
    return payload
```

- [x] **Step 4: 在 `preprocess_common_skill(...)` 中接入 LLM 回退与四类直生**

```python
if func_data is None and func_name in llm_decompile_map and llm_config and llm_config.get("api_key"):
    decompile_result = await _run_llm_decompile_for_target(
        session=session,
        func_name=func_name,
        target_output=target_output,
        llm_spec=llm_decompile_map[func_name],
        llm_config=llm_config,
        image_base=image_base,
        platform=platform,
        debug=debug,
    )
    func_data = await _apply_llm_decompile_results(
        session=session,
        func_name=func_name,
        target_output=target_output,
        expected_outputs=expected_outputs,
        matched_func_outputs=matched_func_outputs,
        matched_gv_outputs=matched_gv_outputs,
        matched_struct_outputs=matched_struct_outputs,
        decompile_result=decompile_result,
        vtable_relations_map=vtable_relations_map,
        image_base=image_base,
        new_binary_dir=new_binary_dir,
        platform=platform,
        debug=debug,
    )
```

- [x] **Step 5: 运行 util 测试，确认四类直生通过**

Run:

```bash
uv run python -m unittest tests.test_ida_analyze_util -v
```

Expected: PASS，`found_vcall` 能直生 `CNetworkMessages_FindNetworkGroup` 的 vfunc YAML，`found_gv` 与 `found_struct_offset` 路径具备当前版生成能力。

- [ ] **Step 6: 提交 util 实现**

```bash
git add ida_analyze_util.py tests/test_ida_analyze_util.py
git commit -m "feat(preprocess): 增加 llm 反编译回退"
```

## Task 6: 接入 `find-CNetworkMessages_FindNetworkGroup.py` 与 prompt 文件

**Files:**
- Create: `ida_preprocessor_scripts/prompt/call_llm_decompile.md`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_FindNetworkGroup.py`
- Modify: `tests/test_ida_preprocessor_scripts.py`
- Check: `.claude/skills/find-CNetworkMessages_FindNetworkGroup/SKILL.md`

- [x] **Step 1: 新增 prompt 文件**

```markdown
You are a reverse engineering expert. I have disassembly outputs and procedure code of the same function.

This is the function for reference:

**Disassembly for Reference**

```c
{disasm_for_reference}
```

**Procedure code for Reference**

```c
{procedure_for_reference}
```

This is the function you need to reverse-engineering:

**Disassembly to reverse-engineering**

```c
{disasm_code}
```

**Procedure code to reverse-engineering**

```c
{procedure}
```

Please collect all references for "{symbol_name_list}" in the function you need to reverse-engineering and output those references as YAML.
`found_vcall` is for indirect call to virtual function.
`found_call` is for direct call to regular function.
`found_gv` is for reference to global variable.
`found_struct_offset` is for reference to struct offset.

If nothing found, output an empty YAML.
```

- [x] **Step 2: 在 `find-CNetworkMessages_FindNetworkGroup.py` 中声明 `LLM_DECOMPILE`**

```python
LLM_DECOMPILE = [
    (
        "CNetworkMessages_FindNetworkGroup",
        "prompt/call_llm_decompile.md",
        "references/CNetworkMessages_FindNetworkGroup.from-CNetworkGameClient_RecordEntityBandwidth.yaml",
    ),
]

FUNC_VTABLE_RELATIONS = [
    ("CNetworkMessages_FindNetworkGroup", "CNetworkMessages", True),
]


async def preprocess_skill(
    session,
    skill_name,
    expected_outputs,
    old_yaml_map,
    new_binary_dir,
    platform,
    image_base,
    llm_config=None,
    debug=False,
):
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
        llm_config=llm_config,
        debug=debug,
    )
```

- [x] **Step 3: 运行脚本测试，确认新样例配置通过**

Run:

```bash
uv run python -m unittest tests.test_ida_preprocessor_scripts -v
```

Expected: PASS，脚本能把 `LLM_DECOMPILE`、`func_vtable_relations` 与 `llm_config` 一起传给 `preprocess_common_skill(...)`。

- [ ] **Step 4: 提交首个样例接入**

```bash
git add ida_preprocessor_scripts/find-CNetworkMessages_FindNetworkGroup.py ida_preprocessor_scripts/prompt/call_llm_decompile.md tests/test_ida_preprocessor_scripts.py
git commit -m "feat(skill): 接入 FindNetworkGroup 反编译回退"
```

## Task 7: 运行定向回归并更新计划状态

**Files:**
- Modify: `docs/superpowers/plans/2026-04-09-llm-decompile.md`
- Check: `tests/test_ida_analyze_bin.py`
- Check: `tests/test_ida_llm_utils.py`
- Check: `tests/test_ida_vcall_finder.py`
- Check: `tests/test_ida_preprocessor_scripts.py`
- Check: `tests/test_ida_analyze_util.py`

- [x] **Step 1: 运行全部定向测试**

Run:

```bash
uv run python -m unittest \
  tests.test_ida_analyze_bin \
  tests.test_ida_llm_utils \
  tests.test_ida_vcall_finder \
  tests.test_ida_preprocessor_scripts \
  tests.test_ida_analyze_util \
  -v
```

Observed: Ran 50 tests in 0.047s, OK.

- [x] **Step 2: 记录手动验收说明**

- 本次代码不自动生成 `ida_preprocessor_scripts/references/CNetworkMessages_FindNetworkGroup.from-CNetworkGameClient_RecordEntityBandwidth.yaml`
- 该样本 YAML 由用户在实现完成后手动准备
- 用户使用真实 IDA 数据验证 `CNetworkMessages_FindNetworkGroup.{platform}.yaml` 是否能由 `found_vcall` 直生

- [x] **Step 3: 更新计划勾选状态并整理交付说明**

- [x] Task 1
- [x] Task 2
- [x] Task 3
- [x] Task 4
- [x] Task 5
- [x] Task 6
- [x] Task 7

- [ ] **Step 4: 提交最终收尾**

```bash
git add docs/superpowers/plans/2026-04-09-llm-decompile.md
git commit -m "docs(plan): 完成 llm 反编译实施计划"
```

## Self-Review

- **Spec coverage:** 本计划覆盖了 spec 中的四大块：统一 `llm_*` CLI、共享 LLM helper、`call_llm_decompile` 与四类直生、`find-CNetworkMessages_FindNetworkGroup.py` 首个真实样例接入与人工验收说明。
- **Placeholder scan:** 计划中没有 `TODO` / `TBD` / “后续实现”一类占位语句；所有测试、命令、文件路径与代码草案均已写明。
- **Type consistency:** 统一使用 `llm_config = {"model", "api_key", "base_url"}`；统一使用 `LLM_DECOMPILE = [(func_name, prompt_path, reference_yaml_path)]`；`preprocess_func_sig_via_mcp(...)` 的新增参数名固定为 `direct_func_va`、`direct_vtable_class`、`direct_vfunc_offset`。
