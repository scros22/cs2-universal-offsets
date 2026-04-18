# llm_decompile found_funcptr Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 扩展共享 `llm_decompile` 流水线以解析并消费 `found_funcptr`，让函数指针装载指令也能生成函数 YAML 与 `func_sig`

**Architecture:** 变更全部收敛在 `ida_analyze_util.py` 的公共 LLM fallback 链路。先补齐空结果与解析器 schema，再新增一个基于 `insn_va` 的 MCP resolver 把函数指针引用恢复为唯一 `direct_func_va`，最后把 `found_funcptr` 插入 `found_call` 与 `found_vcall` 之间，并用定向测试锁住优先级、失败语义与文档一致性。

**Tech Stack:** Python 3、`unittest`/`pytest`、PyYAML、IDA MCP `py_eval`

---

## File Map

- Modify: `ida_analyze_util.py`
  - 责任：扩展 `llm_decompile` 空结果 schema、YAML 解析、函数指针地址恢复、函数目标消费顺序
- Modify: `tests/test_ida_analyze_util.py`
  - 责任：覆盖 `found_funcptr` 的解析、happy path、优先级和失败语义
- Modify: `docs/call_llm_decompile_prompt.md`
  - 责任：把公开文档补齐到与实际 prompt/解析协议一致

## Task 1: 扩展 llm_decompile schema 与解析器

**Files:**
- Modify: `ida_analyze_util.py`
- Modify: `tests/test_ida_analyze_util.py`

- [ ] **Step 1: 写解析层失败测试**

在 `tests/test_ida_analyze_util.py` 的 `TestLlmDecompileSupport` 中新增一个独立测试，先锁住 `found_funcptr` 的标准化行为：

```python
def test_parse_llm_decompile_response_normalizes_found_funcptr(self) -> None:
    response_text = """
```yaml
found_funcptr:
  - insn_va: 0x180666600
    insn_disasm: " lea     rdx, sub_15BC910 "
    funcptr_name: " CLoopModeGame_OnClientPollNetworking "
  - insn_va: 0x180666601
```
""".strip()

    parsed = ida_analyze_util.parse_llm_decompile_response(response_text)

    self.assertEqual(
        [
            {
                "insn_va": "0x180666600",
                "insn_disasm": "lea     rdx, sub_15BC910",
                "funcptr_name": "CLoopModeGame_OnClientPollNetworking",
            }
        ],
        parsed["found_funcptr"],
    )
    self.assertEqual([], parsed["found_call"])
    self.assertEqual([], parsed["found_vcall"])
```

- [ ] **Step 2: 运行该测试并确认失败**

Run:

```bash
python -m pytest tests/test_ida_analyze_util.py::TestLlmDecompileSupport::test_parse_llm_decompile_response_normalizes_found_funcptr -v
```

Expected:

```text
FAIL because parse_llm_decompile_response() does not return a found_funcptr section yet
```

- [ ] **Step 3: 实现 schema 与解析器最小改动**

在 `ida_analyze_util.py` 中同时补齐 `_empty_llm_decompile_result()` 和 `parse_llm_decompile_response()`：

```python
def _empty_llm_decompile_result():
    return {
        "found_vcall": [],
        "found_call": [],
        "found_funcptr": [],
        "found_gv": [],
        "found_struct_offset": [],
    }
```

```python
return {
    "found_vcall": _normalize_llm_entries(
        parsed.get("found_vcall", []),
        ("insn_va", "insn_disasm", "vfunc_offset", "func_name"),
    ),
    "found_call": _normalize_llm_entries(
        parsed.get("found_call", []),
        ("insn_va", "insn_disasm", "func_name"),
    ),
    "found_funcptr": _normalize_llm_entries(
        parsed.get("found_funcptr", []),
        ("insn_va", "insn_disasm", "funcptr_name"),
    ),
    "found_gv": _normalize_llm_entries(
        parsed.get("found_gv", []),
        ("insn_va", "insn_disasm", "gv_name"),
    ),
    "found_struct_offset": _normalize_llm_struct_offset_entries(
        parsed.get("found_struct_offset", []),
    ),
}
```

- [ ] **Step 4: 重新运行解析测试并确认通过**

Run:

```bash
python -m pytest tests/test_ida_analyze_util.py::TestLlmDecompileSupport::test_parse_llm_decompile_response_normalizes_found_funcptr -v
```

Expected:

```text
PASS
```

- [ ] **Step 5: 提交本任务**

```bash
git add ida_analyze_util.py tests/test_ida_analyze_util.py
git commit -m "test(ida): 补充 found_funcptr 解析测试"
```

## Task 2: 新增函数指针 resolver 并接入 happy path

**Files:**
- Modify: `ida_analyze_util.py`
- Modify: `tests/test_ida_analyze_util.py`

- [ ] **Step 1: 写函数 happy path 失败测试**

在 `tests/test_ida_analyze_util.py` 中新增一个专门覆盖 `found_funcptr` 直生函数 YAML 的测试：

```python
async def test_preprocess_common_skill_uses_found_funcptr_to_generate_func_yaml(
    self,
) -> None:
    func_name = "CLoopModeGame_OnClientPollNetworking"
    output_paths = [f"/tmp/{func_name}.windows.yaml"]
    target_detail_payload = {
        "func_name": "CLoopModeGame_RegisterEventMapInternal",
        "func_va": "0x180555500",
        "disasm_code": "lea     rdx, sub_15BC910",
        "procedure": "v40 = sub_15BC910;",
    }
    normalized_payload = {
        "found_vcall": [],
        "found_call": [],
        "found_funcptr": [
            {
                "insn_va": "0x180666600",
                "insn_disasm": "lea     rdx, sub_15BC910",
                "funcptr_name": func_name,
            }
        ],
        "found_gv": [],
        "found_struct_offset": [],
    }

    async def _fake_preprocess_direct_func_sig_via_mcp(**kwargs):
        return {
            "func_name": kwargs["func_name"],
            "func_va": str(kwargs["direct_func_va"]).strip().lower(),
        }

    with tempfile.TemporaryDirectory() as temp_dir:
        preprocessor_dir = Path(temp_dir) / "ida_preprocessor_scripts"
        (preprocessor_dir / "prompt").mkdir(parents=True, exist_ok=True)
        (preprocessor_dir / "prompt" / "call_llm_decompile.md").write_text(
            "{symbol_name_list}",
            encoding="utf-8",
        )
        _write_yaml(
            preprocessor_dir / "references" / "reference.yaml",
            {
                "func_name": target_detail_payload["func_name"],
                "disasm_code": target_detail_payload["disasm_code"],
                "procedure": target_detail_payload["procedure"],
            },
        )

        with patch.object(
            ida_analyze_util,
            "_get_preprocessor_scripts_dir",
            return_value=preprocessor_dir,
        ), patch.object(
            ida_analyze_util,
            "create_openai_client",
            return_value=object(),
            create=True,
        ), patch.object(
            ida_analyze_util,
            "preprocess_func_sig_via_mcp",
            AsyncMock(return_value=None),
        ), patch.object(
            ida_analyze_util,
            "_load_llm_decompile_target_detail_via_mcp",
            AsyncMock(return_value=target_detail_payload),
        ), patch.object(
            ida_analyze_util,
            "_resolve_direct_funcptr_target_via_mcp",
            AsyncMock(return_value="0x180123450"),
            create=True,
        ) as mock_resolve_direct_funcptr_target, patch.object(
            ida_analyze_util,
            "_preprocess_direct_func_sig_via_mcp",
            AsyncMock(side_effect=_fake_preprocess_direct_func_sig_via_mcp),
        ), patch.object(
            ida_analyze_util,
            "call_llm_decompile",
            create=True,
            new_callable=AsyncMock,
            return_value=normalized_payload,
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
                expected_outputs=output_paths,
                old_yaml_map={},
                new_binary_dir="/tmp",
                platform="windows",
                image_base=0x180000000,
                func_names=[func_name],
                generate_yaml_desired_fields=[
                    (func_name, ["func_name", "func_va"]),
                ],
                llm_decompile_specs=[
                    (
                        func_name,
                        "prompt/call_llm_decompile.md",
                        "references/reference.yaml",
                    ),
                ],
                llm_config={
                    "model": "gpt-4.1-mini",
                    "api_key": "test-api-key",
                },
                debug=True,
            )

    self.assertTrue(result)
    mock_resolve_direct_funcptr_target.assert_awaited_once_with(
        "session",
        "0x180666600",
        debug=True,
    )
    self.assertEqual("0x180123450", mock_write_func_yaml.call_args.args[1]["func_va"])
```

- [ ] **Step 2: 运行该测试并确认失败**

Run:

```bash
python -m pytest tests/test_ida_analyze_util.py::TestLlmDecompileSupport::test_preprocess_common_skill_uses_found_funcptr_to_generate_func_yaml -v
```

Expected:

```text
FAIL because preprocess_common_skill() never consumes found_funcptr entries yet
```

- [ ] **Step 3: 实现 resolver 与 happy path 消费逻辑**

先在 `ida_analyze_util.py` 的 resolver 区域新增函数指针解析器：

```python
async def _resolve_direct_funcptr_target_via_mcp(session, insn_va, debug=False):
    try:
        insn_va_int = _parse_int_value(insn_va)
    except Exception:
        return None

    py_code = (
        "import ida_funcs, idautils, json\n"
        f"insn_ea = {insn_va_int}\n"
        "matches = []\n"
        "seen_addrs = set()\n"
        "for target_ea in idautils.DataRefsFrom(insn_ea):\n"
        "    func = ida_funcs.get_func(target_ea)\n"
        "    if func is None:\n"
        "        continue\n"
        "    func_start = int(func.start_ea)\n"
        "    func_va = hex(func_start)\n"
        "    if func_va in seen_addrs:\n"
        "        continue\n"
        "    seen_addrs.add(func_va)\n"
        "    matches.append({'func_va': func_va})\n"
        "result = json.dumps(matches)\n"
    )

    try:
        eval_result = await session.call_tool(
            name="py_eval",
            arguments={"code": py_code},
        )
    except Exception as exc:
        if debug:
            print(
                "    Preprocess: py_eval error while resolving direct funcptr target: "
                f"{exc}"
            )
        return None

    match_payload = _parse_py_eval_json_result(
        eval_result,
        debug=debug,
        context="llm_decompile direct funcptr target lookup",
    )
    if not isinstance(match_payload, list):
        return None

    resolved_matches = []
    seen_func_vas = set()
    for item in match_payload:
        if not isinstance(item, dict):
            continue
        func_va = str(item.get("func_va", "")).strip()
        if not func_va or func_va in seen_func_vas:
            continue
        try:
            int(func_va, 0)
        except (TypeError, ValueError):
            continue
        seen_func_vas.add(func_va)
        resolved_matches.append(func_va)

    if len(resolved_matches) != 1:
        if debug:
            print(
                "    Preprocess: llm_decompile direct funcptr target lookup returned "
                f"{len(resolved_matches)} matches: {resolved_matches}"
            )
        return None

    return resolved_matches[0]
```

再把 `preprocess_common_skill()` 的函数 fallback 顺序改为 `found_call -> found_funcptr -> found_vcall`：

```python
for entry in llm_result.get("found_funcptr", []):
    if entry.get("funcptr_name") != func_name:
        continue
    direct_func_va = await _resolve_direct_funcptr_target_via_mcp(
        session,
        entry.get("insn_va"),
        debug=debug,
    )
    if direct_func_va is None:
        continue
    func_data = await _preprocess_direct_func_sig_via_mcp(
        session=session,
        new_path=target_output,
        image_base=image_base,
        platform=platform,
        func_name=func_name,
        direct_func_va=direct_func_va,
        require_func_sig="func_sig" in desired_fields_set,
        normalized_mangled_class_names=normalized_mangled_class_names,
        debug=debug,
    )
    if func_data is not None:
        break
```

- [ ] **Step 4: 重新运行 happy path 测试并确认通过**

Run:

```bash
python -m pytest tests/test_ida_analyze_util.py::TestLlmDecompileSupport::test_preprocess_common_skill_uses_found_funcptr_to_generate_func_yaml -v
```

Expected:

```text
PASS
```

- [ ] **Step 5: 提交本任务**

```bash
git add ida_analyze_util.py tests/test_ida_analyze_util.py
git commit -m "feat(ida): 支持 found_funcptr 地址恢复"
```

## Task 3: 锁住优先级、失败语义并同步文档

**Files:**
- Modify: `ida_analyze_util.py`
- Modify: `tests/test_ida_analyze_util.py`
- Modify: `docs/call_llm_decompile_prompt.md`

- [ ] **Step 1: 写回归测试覆盖优先级与失败语义**

在 `tests/test_ida_analyze_util.py` 中新增两个完整回归测试：

```python
async def test_preprocess_common_skill_prefers_found_call_over_found_funcptr(
    self,
) -> None:
    func_name = "CLoopModeGame_OnClientPollNetworking"
    output_paths = [f"/tmp/{func_name}.windows.yaml"]
    target_detail_payload = {
        "func_name": "CLoopModeGame_RegisterEventMapInternal",
        "func_va": "0x180555500",
        "disasm_code": "call    sub_180111111\nlea     rdx, sub_15BC910",
        "procedure": "RegisterEventListener_Abstract(...); v40 = sub_15BC910;",
    }
    normalized_payload = {
        "found_vcall": [],
        "found_call": [
            {
                "insn_va": "0x180777700",
                "insn_disasm": "call    sub_180111111",
                "func_name": func_name,
            }
        ],
        "found_funcptr": [
            {
                "insn_va": "0x180666600",
                "insn_disasm": "lea     rdx, sub_15BC910",
                "funcptr_name": func_name,
            }
        ],
        "found_gv": [],
        "found_struct_offset": [],
    }

    async def _fake_preprocess_direct_func_sig_via_mcp(**kwargs):
        return {
            "func_name": kwargs["func_name"],
            "func_va": str(kwargs["direct_func_va"]).strip().lower(),
        }

    with tempfile.TemporaryDirectory() as temp_dir:
        preprocessor_dir = Path(temp_dir) / "ida_preprocessor_scripts"
        (preprocessor_dir / "prompt").mkdir(parents=True, exist_ok=True)
        (preprocessor_dir / "prompt" / "call_llm_decompile.md").write_text(
            "{symbol_name_list}",
            encoding="utf-8",
        )
        _write_yaml(
            preprocessor_dir / "references" / "reference.yaml",
            {
                "func_name": target_detail_payload["func_name"],
                "disasm_code": target_detail_payload["disasm_code"],
                "procedure": target_detail_payload["procedure"],
            },
        )

        with patch.object(
            ida_analyze_util,
            "_get_preprocessor_scripts_dir",
            return_value=preprocessor_dir,
        ), patch.object(
            ida_analyze_util,
            "create_openai_client",
            return_value=object(),
            create=True,
        ), patch.object(
            ida_analyze_util,
            "preprocess_func_sig_via_mcp",
            AsyncMock(return_value=None),
        ), patch.object(
            ida_analyze_util,
            "_load_llm_decompile_target_detail_via_mcp",
            AsyncMock(return_value=target_detail_payload),
        ), patch.object(
            ida_analyze_util,
            "_resolve_direct_call_target_via_mcp",
            AsyncMock(return_value="0x180111111"),
        ) as mock_resolve_call, patch.object(
            ida_analyze_util,
            "_resolve_direct_funcptr_target_via_mcp",
            AsyncMock(return_value="0x180222222"),
            create=True,
        ) as mock_resolve_funcptr, patch.object(
            ida_analyze_util,
            "_preprocess_direct_func_sig_via_mcp",
            AsyncMock(side_effect=_fake_preprocess_direct_func_sig_via_mcp),
        ), patch.object(
            ida_analyze_util,
            "call_llm_decompile",
            create=True,
            new_callable=AsyncMock,
            return_value=normalized_payload,
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
                expected_outputs=output_paths,
                old_yaml_map={},
                new_binary_dir="/tmp",
                platform="windows",
                image_base=0x180000000,
                func_names=[func_name],
                generate_yaml_desired_fields=[
                    (func_name, ["func_name", "func_va"]),
                ],
                llm_decompile_specs=[
                    (
                        func_name,
                        "prompt/call_llm_decompile.md",
                        "references/reference.yaml",
                    ),
                ],
                llm_config={
                    "model": "gpt-4.1-mini",
                    "api_key": "test-api-key",
                },
                debug=True,
            )

    self.assertTrue(result)
    mock_resolve_call.assert_awaited_once_with("session", "0x180777700", debug=True)
    mock_resolve_funcptr.assert_not_awaited()
    self.assertEqual("0x180111111", mock_write_func_yaml.call_args.args[1]["func_va"])
```
```python
async def test_preprocess_common_skill_skips_found_funcptr_when_resolver_is_non_unique(
    self,
) -> None:
    func_name = "CLoopModeGame_OnClientPollNetworking"
    output_paths = [f"/tmp/{func_name}.windows.yaml"]
    target_detail_payload = {
        "func_name": "CLoopModeGame_RegisterEventMapInternal",
        "func_va": "0x180555500",
        "disasm_code": "lea     rdx, sub_15BC910",
        "procedure": "v40 = sub_15BC910;",
    }
    normalized_payload = {
        "found_vcall": [],
        "found_call": [],
        "found_funcptr": [
            {
                "insn_va": "0x180666600",
                "insn_disasm": "lea     rdx, sub_15BC910",
                "funcptr_name": func_name,
            }
        ],
        "found_gv": [],
        "found_struct_offset": [],
    }

    with tempfile.TemporaryDirectory() as temp_dir:
        preprocessor_dir = Path(temp_dir) / "ida_preprocessor_scripts"
        (preprocessor_dir / "prompt").mkdir(parents=True, exist_ok=True)
        (preprocessor_dir / "prompt" / "call_llm_decompile.md").write_text(
            "{symbol_name_list}",
            encoding="utf-8",
        )
        _write_yaml(
            preprocessor_dir / "references" / "reference.yaml",
            {
                "func_name": target_detail_payload["func_name"],
                "disasm_code": target_detail_payload["disasm_code"],
                "procedure": target_detail_payload["procedure"],
            },
        )

        with patch.object(
            ida_analyze_util,
            "_get_preprocessor_scripts_dir",
            return_value=preprocessor_dir,
        ), patch.object(
            ida_analyze_util,
            "create_openai_client",
            return_value=object(),
            create=True,
        ), patch.object(
            ida_analyze_util,
            "preprocess_func_sig_via_mcp",
            AsyncMock(return_value=None),
        ), patch.object(
            ida_analyze_util,
            "_load_llm_decompile_target_detail_via_mcp",
            AsyncMock(return_value=target_detail_payload),
        ), patch.object(
            ida_analyze_util,
            "_resolve_direct_funcptr_target_via_mcp",
            AsyncMock(return_value=None),
            create=True,
        ), patch.object(
            ida_analyze_util,
            "_preprocess_direct_func_sig_via_mcp",
            AsyncMock(),
        ) as mock_preprocess_direct_func_sig, patch.object(
            ida_analyze_util,
            "call_llm_decompile",
            create=True,
            new_callable=AsyncMock,
            return_value=normalized_payload,
        ), patch.object(
            ida_analyze_util,
            "_rename_func_in_ida",
            AsyncMock(return_value=None),
        ):
            result = await ida_analyze_util.preprocess_common_skill(
                session="session",
                expected_outputs=output_paths,
                old_yaml_map={},
                new_binary_dir="/tmp",
                platform="windows",
                image_base=0x180000000,
                func_names=[func_name],
                generate_yaml_desired_fields=[
                    (func_name, ["func_name", "func_va"]),
                ],
                llm_decompile_specs=[
                    (
                        func_name,
                        "prompt/call_llm_decompile.md",
                        "references/reference.yaml",
                    ),
                ],
                llm_config={
                    "model": "gpt-4.1-mini",
                    "api_key": "test-api-key",
                },
                debug=True,
            )

    self.assertFalse(result)
    mock_preprocess_direct_func_sig.assert_not_awaited()
```

- [ ] **Step 2: 运行回归测试并确认失败**

Run:

```bash
python -m pytest \
  tests/test_ida_analyze_util.py::TestLlmDecompileSupport::test_preprocess_common_skill_prefers_found_call_over_found_funcptr \
  tests/test_ida_analyze_util.py::TestLlmDecompileSupport::test_preprocess_common_skill_skips_found_funcptr_when_resolver_is_non_unique \
  -v
```

Expected:

```text
At least one test FAILS until found_funcptr ordering and resolver-failure behavior are locked down
```

- [ ] **Step 3: 收紧控制流并同步公开文档**

在 `ida_analyze_util.py` 中把函数目标的 LLM fallback 顺序写成显式的三级分支，先 `found_call`，再 `found_funcptr`，最后 `found_vcall`：

```python
for entry in llm_result.get("found_call", []):
    if entry.get("func_name") != func_name:
        continue
    direct_func_va = await _resolve_direct_call_target_via_mcp(
        session,
        entry.get("insn_va"),
        debug=debug,
    )
    if direct_func_va is None:
        continue
    func_data = await _preprocess_direct_func_sig_via_mcp(
        session=session,
        new_path=target_output,
        image_base=image_base,
        platform=platform,
        func_name=func_name,
        direct_func_va=direct_func_va,
        require_func_sig="func_sig" in desired_fields_set,
        normalized_mangled_class_names=normalized_mangled_class_names,
        debug=debug,
    )
    if func_data is not None:
        break

if func_data is None:
    for entry in llm_result.get("found_funcptr", []):
        if entry.get("funcptr_name") != func_name:
            continue
        direct_func_va = await _resolve_direct_funcptr_target_via_mcp(
            session,
            entry.get("insn_va"),
            debug=debug,
        )
        if direct_func_va is None:
            continue
        func_data = await _preprocess_direct_func_sig_via_mcp(
            session=session,
            new_path=target_output,
            image_base=image_base,
            platform=platform,
            func_name=func_name,
            direct_func_va=direct_func_va,
            require_func_sig="func_sig" in desired_fields_set,
            normalized_mangled_class_names=normalized_mangled_class_names,
            debug=debug,
        )
        if func_data is not None:
            break

if func_data is None and func_name in vtable_relations_map:
    vtable_class = vtable_relations_map[func_name]
```

然后更新 `docs/call_llm_decompile_prompt.md`，把公开文档补齐为与真实 prompt 一致：

```markdown
`found_vcall` is for indirect call to virtual function.
`found_call` is for direct call to regular non-virtual function.
`found_funcptr` is for reference to function pointer.
`found_gv` is for reference to global variable.
`found_struct_offset` is for reference to struct offset.
```

并补上示例段：

```yaml
found_funcptr:
  - insn_va: '0x180666600'
    insn_disasm: lea     rdx, sub_15BC910
    funcptr_name: CLoopModeGame_OnClientPollNetworking
```

- [ ] **Step 4: 运行定向回归测试并确认通过**

Run:

```bash
python -m pytest \
  tests/test_ida_analyze_util.py::TestLlmDecompileSupport::test_parse_llm_decompile_response_normalizes_found_funcptr \
  tests/test_ida_analyze_util.py::TestLlmDecompileSupport::test_preprocess_common_skill_uses_found_funcptr_to_generate_func_yaml \
  tests/test_ida_analyze_util.py::TestLlmDecompileSupport::test_preprocess_common_skill_prefers_found_call_over_found_funcptr \
  tests/test_ida_analyze_util.py::TestLlmDecompileSupport::test_preprocess_common_skill_skips_found_funcptr_when_resolver_is_non_unique \
  -v
```

Expected:

```text
4 passed
```

- [ ] **Step 5: 提交本任务**

```bash
git add ida_analyze_util.py tests/test_ida_analyze_util.py docs/call_llm_decompile_prompt.md
git commit -m "fix(llm): 保持 found_funcptr 回退顺序"
```

## Self-Review Checklist

- `docs/superpowers/specs/2026-04-12-llm-decompile-found-funcptr-design.md` 中的每项要求都能映射到 Task 1-3
- 计划中没有 `TODO`、`TBD`、`implement later`、`similar to` 之类占位语
- 新增名字保持一致：`found_funcptr`、`funcptr_name`、`_resolve_direct_funcptr_target_via_mcp`
- `found_funcptr` 的消费顺序明确位于 `found_call` 与 `found_vcall` 之间
