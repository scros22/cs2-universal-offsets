# LLM Decompile Batch Request Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 让 `preprocess_common_skill(...)` 在多个 `LLM_DECOMPILE` 条目共享同一 `prompt_path` 与 `reference_yaml_path` 时，只对仍需 LLM fallback 的目标发起一次合并请求，并在 `symbol_name_list` 中携带整组 unresolved symbol。

**Architecture:** 保持 `call_llm_decompile(...)` 与 YAML 解析结构不变，只改 `ida_analyze_util.py` 的调用侧。先给 `_prepare_llm_decompile_request(...)` 增加稳定的分组身份，再在 `preprocess_common_skill(...)` 中引入“fast path 结果缓存 + LLM 分组结果缓存”，使同组 unresolved 目标共享一次 `call_llm_decompile(...)` 结果，同时继续按 `entry["func_name"]` 为每个 symbol 过滤自身条目。

**Tech Stack:** Python 3.10, `unittest`, `unittest.mock.AsyncMock`, `tempfile`, `PyYAML`

---

## File Map

- `ida_analyze_util.py`
  - 扩展 `_prepare_llm_decompile_request(...)` 的返回值，暴露已解析的 `prompt_path` 与 `reference_yaml_path`
  - 新增 `_build_llm_decompile_request_cache_key(...)`，统一生成同组请求缓存键
  - 新增 `_try_preprocess_func_without_llm(...)`，抽离 `func_sig` 与 `func_xrefs` 两段 fast path
  - 在 `preprocess_common_skill(...)` 中维护：
    - `fast_path_results`
    - `fast_path_attempted`
    - `llm_request_cache`
    - `llm_result_cache`
  - 让同组 unresolved 目标共享一次 `call_llm_decompile(...)`
- `tests/test_ida_analyze_util.py`
  - 新增“同组 unresolved 目标只请求一次”的回归测试
  - 新增“fast path 已命中目标不得进入 `symbol_name_list`”的回归测试
  - 给现有单目标 LLM fallback 用例补一个 `symbol_name_list == [func_name]` 的断言，锁定单目标行为

## Task 1: 合并同组 unresolved LLM 请求

**Files:**
- Modify: `tests/test_ida_analyze_util.py`
- Modify: `ida_analyze_util.py`

- [ ] **Step 1: 先写“同组 unresolved 目标只请求一次”的失败测试**

```python
    async def test_preprocess_common_skill_batches_same_llm_request_for_multiple_unresolved_targets(
        self,
    ) -> None:
        func_names = [
            "INetworkMessages_GetFieldChangeCallbackOrderCount",
            "INetworkMessages_GetFieldChangeCallbackPriorities",
        ]
        expected_outputs = [f"/tmp/{name}.windows.yaml" for name in func_names]
        llm_payload = {
            "found_vcall": [
                {
                    "insn_va": "0x180700010",
                    "insn_disasm": "call    qword ptr [rax+68h]",
                    "vfunc_offset": "0x68",
                    "func_name": func_names[0],
                },
                {
                    "insn_va": "0x180700018",
                    "insn_disasm": "call    qword ptr [rax+70h]",
                    "vfunc_offset": "0x70",
                    "func_name": func_names[1],
                },
            ],
            "found_call": [],
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
                    "func_name": "CFlattenedSerializers_CreateFieldChangedEventQueue",
                    "disasm_code": "mov     rax, [rcx]",
                    "procedure": "return this->vfptr[13](this);",
                },
            )

            async def _fake_direct_func_sig(**kwargs):
                func_name = kwargs["func_name"]
                return {
                    "func_name": func_name,
                    "func_va": (
                        "0x180123450"
                        if func_name == func_names[0]
                        else "0x180123460"
                    ),
                    "func_rva": (
                        "0x123450"
                        if func_name == func_names[0]
                        else "0x123460"
                    ),
                    "func_size": "0x40",
                }

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
                "call_llm_decompile",
                new_callable=AsyncMock,
                return_value=llm_payload,
                create=True,
            ) as mock_call_llm_decompile, patch.object(
                ida_analyze_util,
                "_load_llm_decompile_target_detail_via_mcp",
                AsyncMock(
                    return_value={
                        "func_name": "CFlattenedSerializers_CreateFieldChangedEventQueue",
                        "func_va": "0x180555500",
                        "disasm_code": "call    qword ptr [rax+68h]",
                        "procedure": "return this->vfptr[13](this);",
                    }
                ),
            ), patch.object(
                ida_analyze_util,
                "_preprocess_direct_func_sig_via_mcp",
                AsyncMock(side_effect=_fake_direct_func_sig),
            ), patch.object(
                ida_analyze_util,
                "write_func_yaml",
            ) as mock_write_func_yaml, patch.object(
                ida_analyze_util,
                "_rename_func_in_ida",
                AsyncMock(return_value=None),
            ):
                result = await ida_analyze_util.preprocess_common_skill(
                    session=AsyncMock(),
                    expected_outputs=expected_outputs,
                    old_yaml_map={},
                    new_binary_dir="/tmp",
                    platform="windows",
                    image_base=0x180000000,
                    func_names=func_names,
                    func_vtable_relations=[
                        (func_names[0], "INetworkMessages"),
                        (func_names[1], "INetworkMessages"),
                    ],
                    generate_yaml_desired_fields=[
                        (func_names[0], ["func_name", "func_va"]),
                        (func_names[1], ["func_name", "func_va"]),
                    ],
                    llm_decompile_specs=[
                        (
                            func_names[0],
                            "prompt/call_llm_decompile.md",
                            "references/reference.yaml",
                        ),
                        (
                            func_names[1],
                            "prompt/call_llm_decompile.md",
                            "references/reference.yaml",
                        ),
                    ],
                    llm_config={"model": "gpt-4.1-mini", "api_key": "test-key"},
                    debug=True,
                )

        self.assertTrue(result)
        mock_call_llm_decompile.assert_awaited_once()
        self.assertEqual(
            func_names,
            mock_call_llm_decompile.call_args.kwargs["symbol_name_list"],
        )
        self.assertEqual(2, mock_write_func_yaml.call_count)
```

- [ ] **Step 2: 运行单测，确认当前实现会失败**

Run:

```bash
uv run python -m unittest \
  tests.test_ida_analyze_util.TestLlmDecompileSupport.test_preprocess_common_skill_batches_same_llm_request_for_multiple_unresolved_targets \
  -v
```

Expected: FAIL，典型报错是 `Expected 'call_llm_decompile' to have been awaited once. Awaited 2 times.`。

- [ ] **Step 3: 给 LLM 请求增加稳定分组键**

```python
def _build_llm_decompile_request_cache_key(llm_request):
    if not isinstance(llm_request, dict):
        return None

    prompt_path = str(llm_request.get("prompt_path", "")).strip()
    reference_yaml_path = str(llm_request.get("reference_yaml_path", "")).strip()
    if not prompt_path or not reference_yaml_path:
        return None

    return (prompt_path, reference_yaml_path)
```

```python
# replace the return block at the end of _prepare_llm_decompile_request(...)
return {
    "client": client,
    "model": model,
    "prompt_path": str(prompt_path),
    "reference_yaml_path": str(reference_yaml_path),
    "prompt_template": prompt_template,
    "target_func_name": target_func_name,
    "disasm_for_reference": str(reference_data.get("disasm_code", "") or ""),
    "procedure_for_reference": str(reference_data.get("procedure", "") or ""),
}
```

- [ ] **Step 4: 在 `preprocess_common_skill(...)` 中缓存同组 LLM 结果**

```python
# insert these locals before the `for func_name in all_func_names:` loop
    llm_request_cache = {}
    llm_result_cache = {}

    def _get_llm_request(func_name):
        if func_name not in llm_request_cache:
            llm_request_cache[func_name] = _prepare_llm_decompile_request(
                func_name,
                llm_decompile_specs_map,
                llm_config,
                platform=platform,
                debug=debug,
            )
        return llm_request_cache[func_name]
```

```python
# replace the current `if func_data is None and func_name in llm_decompile_specs_map:` block with:
        if func_data is None and func_name in llm_decompile_specs_map:
            llm_request = _get_llm_request(func_name)
            cache_key = _build_llm_decompile_request_cache_key(llm_request)
            llm_result = llm_result_cache.get(cache_key)
            if llm_result is None:
                if llm_request is None or cache_key is None:
                    llm_result = _empty_llm_decompile_result()
                else:
                    batch_symbol_name_list = []
                    for candidate_name in all_func_names:
                        candidate_request = _get_llm_request(candidate_name)
                        candidate_key = _build_llm_decompile_request_cache_key(
                            candidate_request
                        )
                        if candidate_key != cache_key:
                            continue
                        batch_symbol_name_list.append(candidate_name)

                    try:
                        llm_target_detail = await _load_llm_decompile_target_detail_via_mcp(
                            session,
                            llm_request["target_func_name"],
                            debug=debug,
                        )
                        if llm_target_detail is None:
                            llm_result = _empty_llm_decompile_result()
                        else:
                            llm_result = await call_llm_decompile(
                                client=llm_request["client"],
                                model=llm_request["model"],
                                symbol_name_list=batch_symbol_name_list,
                                disasm_code=llm_target_detail["disasm_code"],
                                procedure=llm_target_detail["procedure"],
                                disasm_for_reference=llm_request["disasm_for_reference"],
                                procedure_for_reference=llm_request["procedure_for_reference"],
                                prompt_template=llm_request["prompt_template"],
                                platform=platform,
                                debug=debug,
                            )
                    except Exception:
                        llm_result = _empty_llm_decompile_result()

                llm_result_cache[cache_key] = llm_result
```

- [ ] **Step 5: 重新运行单测，确认 batching 生效**

Run:

```bash
uv run python -m unittest \
  tests.test_ida_analyze_util.TestLlmDecompileSupport.test_preprocess_common_skill_batches_same_llm_request_for_multiple_unresolved_targets \
  -v
```

Expected: PASS，且断言 `symbol_name_list` 等于两个目标名称。

- [ ] **Step 6: 提交这部分实现**

```bash
git add ida_analyze_util.py tests/test_ida_analyze_util.py
git commit -m "fix(ida): 合并同组 llm decompile 回退请求"
```

## Task 2: 排除 fast path 已命中的目标

**Files:**
- Modify: `tests/test_ida_analyze_util.py`
- Modify: `ida_analyze_util.py`

- [ ] **Step 1: 写“fast path 命中的目标不得进入 batch”的失败测试**

```python
    async def test_preprocess_common_skill_llm_batch_excludes_fast_path_targets(
        self,
    ) -> None:
        func_names = [
            "INetworkMessages_GetFieldChangeCallbackOrderCount",
            "INetworkMessages_GetFieldChangeCallbackPriorities",
        ]
        expected_outputs = [f"/tmp/{name}.windows.yaml" for name in func_names]

        async def _fake_preprocess_func_sig(**kwargs):
            func_name = kwargs["func_name"]
            if func_name == func_names[0]:
                return {
                    "func_name": func_name,
                    "func_va": "0x180123450",
                    "func_rva": "0x123450",
                    "func_size": "0x40",
                }
            return None

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
                    "func_name": "CFlattenedSerializers_CreateFieldChangedEventQueue",
                    "disasm_code": "call    sub_180222200",
                    "procedure": "return this->vfptr[13](this);",
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
                AsyncMock(side_effect=_fake_preprocess_func_sig),
            ), patch.object(
                ida_analyze_util,
                "call_llm_decompile",
                new_callable=AsyncMock,
                create=True,
                return_value={
                    "found_vcall": [],
                    "found_call": [
                        {
                            "insn_va": "0x180700020",
                            "insn_disasm": "call    sub_180222200",
                            "func_name": func_names[1],
                        }
                    ],
                    "found_gv": [],
                    "found_struct_offset": [],
                },
            ) as mock_call_llm_decompile, patch.object(
                ida_analyze_util,
                "_load_llm_decompile_target_detail_via_mcp",
                AsyncMock(
                    return_value={
                        "func_name": "CFlattenedSerializers_CreateFieldChangedEventQueue",
                        "func_va": "0x180555500",
                        "disasm_code": "call    sub_180222200",
                        "procedure": "return sub_180222200(this);",
                    }
                ),
            ), patch.object(
                ida_analyze_util,
                "_resolve_direct_call_target_via_mcp",
                AsyncMock(return_value="0x180123460"),
            ), patch.object(
                ida_analyze_util,
                "_preprocess_direct_func_sig_via_mcp",
                AsyncMock(
                    return_value={
                        "func_name": func_names[1],
                        "func_va": "0x180123460",
                        "func_rva": "0x123460",
                        "func_size": "0x40",
                    }
                ),
            ), patch.object(
                ida_analyze_util,
                "write_func_yaml",
            ), patch.object(
                ida_analyze_util,
                "_rename_func_in_ida",
                AsyncMock(return_value=None),
            ):
                result = await ida_analyze_util.preprocess_common_skill(
                    session=AsyncMock(),
                    expected_outputs=expected_outputs,
                    old_yaml_map={},
                    new_binary_dir="/tmp",
                    platform="windows",
                    image_base=0x180000000,
                    func_names=func_names,
                    generate_yaml_desired_fields=[
                        (func_names[0], ["func_name", "func_va"]),
                        (func_names[1], ["func_name", "func_va"]),
                    ],
                    llm_decompile_specs=[
                        (
                            func_names[0],
                            "prompt/call_llm_decompile.md",
                            "references/reference.yaml",
                        ),
                        (
                            func_names[1],
                            "prompt/call_llm_decompile.md",
                            "references/reference.yaml",
                        ),
                    ],
                    llm_config={"model": "gpt-4.1-mini", "api_key": "test-key"},
                    debug=True,
                )

        self.assertTrue(result)
        self.assertEqual(
            [func_names[1]],
            mock_call_llm_decompile.call_args.kwargs["symbol_name_list"],
        )
```

- [ ] **Step 2: 给现有单目标用例补一个 `symbol_name_list` 断言**

```python
        self.assertEqual(
            [func_name],
            mock_call_llm_decompile.call_args.kwargs["symbol_name_list"],
        )
```

- [ ] **Step 3: 运行两个回归测试，确认第二个测试当前会失败**

Run:

```bash
uv run python -m unittest \
  tests.test_ida_analyze_util.TestLlmDecompileSupport.test_preprocess_common_skill_batches_same_llm_request_for_multiple_unresolved_targets \
  tests.test_ida_analyze_util.TestLlmDecompileSupport.test_preprocess_common_skill_llm_batch_excludes_fast_path_targets \
  -v
```

Expected: 第一个测试 PASS，第二个测试 FAIL，典型报错是 `symbol_name_list` 里仍包含已由 fast path 命中的目标。

- [ ] **Step 4: 抽出 `func_sig` / `func_xrefs` fast path，并在 batch 收集前先判定 unresolved**

```python
async def _try_preprocess_func_without_llm(
    *,
    session,
    func_name,
    target_output,
    old_yaml_map,
    new_binary_dir,
    platform,
    image_base,
    normalized_mangled_class_names,
    func_xrefs_map,
    vtable_relations_map,
    debug=False,
):
    old_path = (old_yaml_map or {}).get(target_output)
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

    if func_data is None and func_name in func_xrefs_map:
        xref_spec = func_xrefs_map[func_name]
        func_data = await preprocess_func_xrefs_via_mcp(
            session=session,
            func_name=func_name,
            xref_strings=xref_spec["xref_strings"],
            xref_funcs=xref_spec["xref_funcs"],
            exclude_funcs=xref_spec["exclude_funcs"],
            new_binary_dir=new_binary_dir,
            platform=platform,
            image_base=image_base,
            vtable_class=vtable_relations_map.get(func_name),
            debug=debug,
        )

    return func_data
```

```python
    fast_path_attempted = set()
    fast_path_results = {}

    async def _ensure_fast_path(func_name):
        if func_name in fast_path_attempted:
            return fast_path_results.get(func_name)

        fast_path_attempted.add(func_name)
        fast_path_results[func_name] = await _try_preprocess_func_without_llm(
            session=session,
            func_name=func_name,
            target_output=matched_func_outputs[func_name],
            old_yaml_map=old_yaml_map,
            new_binary_dir=new_binary_dir,
            platform=platform,
            image_base=image_base,
            normalized_mangled_class_names=normalized_mangled_class_names,
            func_xrefs_map=func_xrefs_map,
            vtable_relations_map=vtable_relations_map,
            debug=debug,
        )
        return fast_path_results[func_name]
```

```python
# replace the func loop entry and candidate collection with:
        func_data = await _ensure_fast_path(func_name)

        if func_data is None and func_name in llm_decompile_specs_map:
            llm_request = _get_llm_request(func_name)
            cache_key = _build_llm_decompile_request_cache_key(llm_request)
            llm_result = llm_result_cache.get(cache_key)
            if llm_result is None:
                if llm_request is None or cache_key is None:
                    llm_result = _empty_llm_decompile_result()
                else:
                    batch_symbol_name_list = []
                    for candidate_name in all_func_names:
                        candidate_request = _get_llm_request(candidate_name)
                        candidate_key = _build_llm_decompile_request_cache_key(
                            candidate_request
                        )
                        if candidate_key != cache_key:
                            continue
                        if await _ensure_fast_path(candidate_name) is not None:
                            continue
                        batch_symbol_name_list.append(candidate_name)
```

- [ ] **Step 5: 重新运行两个回归测试，确认都通过**

Run:

```bash
uv run python -m unittest \
  tests.test_ida_analyze_util.TestLlmDecompileSupport.test_preprocess_common_skill_batches_same_llm_request_for_multiple_unresolved_targets \
  tests.test_ida_analyze_util.TestLlmDecompileSupport.test_preprocess_common_skill_llm_batch_excludes_fast_path_targets \
  -v
```

Expected: PASS。

- [ ] **Step 6: 提交 fast-path 过滤修复**

```bash
git add ida_analyze_util.py tests/test_ida_analyze_util.py
git commit -m "fix(ida): 排除已命中的 llm 回退分组目标"
```

## Task 3: 做与本次改动直接相关的回归验证

**Files:**
- Modify: `tests/test_ida_analyze_util.py`
- Verify: `ida_analyze_util.py`

- [ ] **Step 1: 运行完整的 `TestLlmDecompileSupport`**

Run:

```bash
uv run python -m unittest tests.test_ida_analyze_util.TestLlmDecompileSupport -v
```

Expected: PASS，包含：
- 单目标 LLM fallback
- direct call fallback
- slot-only fallback
- 新增的 batch 与 fast-path exclusion 回归

- [ ] **Step 2: 运行整个 `tests.test_ida_analyze_util` 文件**

Run:

```bash
uv run python -m unittest tests.test_ida_analyze_util -v
```

Expected: PASS，没有新增的 rename/write 顺序回归。

- [ ] **Step 3: 检查最终 diff 仅限本次需求**

Run:

```bash
git diff -- ida_analyze_util.py tests/test_ida_analyze_util.py
```

Expected: 只看到：
- LLM request key / cache helper
- fast path helper
- `preprocess_common_skill(...)` batching 逻辑
- 两个新回归测试与一个单目标断言

## Self-Review Checklist

- Spec coverage
  - “同 prompt/reference 只发一次请求”由 Task 1 覆盖
  - “只合并仍需 LLM fallback 的目标”由 Task 2 覆盖
  - “单目标行为不变”由 Task 2 的单目标断言与 Task 3 回归覆盖
- Placeholder scan
  - 无 `TODO`、`TBD`、`later`、`similar to`
  - 每个代码步骤都给出具体代码
  - 每个验证步骤都给出精确命令与预期
- Type consistency
  - 统一使用 `prompt_path` / `reference_yaml_path` / `symbol_name_list`
  - fast path helper 的输入输出保持与现有 `preprocess_func_sig_via_mcp(...)` / `preprocess_func_xrefs_via_mcp(...)` 一致
