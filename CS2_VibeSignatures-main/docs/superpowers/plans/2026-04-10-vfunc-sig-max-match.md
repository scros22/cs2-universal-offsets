# Vfunc Sig Max Match Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为 `vfunc_sig` 增加 `vfunc_sig_max_match` 契约与 YAML 持久化能力，让 slot-only 生成和旧 YAML 复用都支持“最多 N 个匹配”。

**Architecture:** 先用 `unittest` 锁定三类行为：字段契约能把 `"vfunc_sig_max_match:10"` 解析成输出字段和生成选项、`preprocess_gen_vfunc_sig_via_mcp(...)` 能在 `<= N` 个命中时提前收敛、`preprocess_func_sig_via_mcp(...)` 能读取旧 YAML 中的 `vfunc_sig_max_match` 放宽 `vfunc_sig` 复用匹配条件。随后在 `ida_analyze_util.py` 中补齐 directive 规范化、字段顺序、slot-only 参数透传、`vfunc_sig` 限制匹配 helper 与 YAML 组装逻辑，最后回归目标脚本与相关定向测试。

**Tech Stack:** Python 3、`unittest`、`unittest.mock`、`tempfile`、`pathlib`、`yaml.safe_dump`

---

## File Structure

- Modify: `ida_analyze_util.py`
  - 扩展 `_normalize_generate_yaml_desired_fields(...)` 以区分 `desired_output_fields` 与 `generation_options`
  - 在 `func` 字段顺序与合法字段集合中加入 `vfunc_sig_max_match`
  - 调整 `_assemble_symbol_payload(...)` 只消费规范化后的输出字段
  - 为 slot-only fallback 增加 `vfunc_sig_max_match` 透传
  - 为 `preprocess_gen_vfunc_sig_via_mcp(...)` 增加 `max_match_count`
  - 为 `preprocess_func_sig_via_mcp(...)` 增加仅限 `vfunc_sig` 路径使用的受限匹配 helper
- Modify: `tests/test_ida_analyze_util.py`
  - 新增 directive 解析、非法契约、slot-only 透传、多匹配生成、旧 YAML 复用等回归测试
- Modify: `tests/test_ida_preprocessor_scripts.py`
  - 为 `find-INetworkMessages_GetLoggingChannel-windows.py` 增加转发 `"vfunc_sig_max_match:10"` 的断言
- Modify: `ida_preprocessor_scripts/find-INetworkMessages_GetLoggingChannel-windows.py`
  - 若当前分支尚未保留用户改动，则补齐 `"vfunc_sig_max_match:10"`
- Create: `docs/superpowers/plans/2026-04-10-vfunc-sig-max-match.md`
  - 当前实现计划文档

**仓库约束：**

- 实施时先跑定向 `unittest`，不要先跑全量 build
- `git commit` 消息遵循仓库约定：`<type>(scope): <中文动词开头摘要>`
- 该计划只放宽 `vfunc_sig`，不能顺手改松 `func_sig`、`gv_sig`、`patch_sig`

## Task 1: 锁定字段契约与 YAML 输出行为

**Files:**
- Modify: `tests/test_ida_analyze_util.py`
- Modify: `ida_analyze_util.py`

- [ ] **Step 1: 先写 directive 解析与非法契约的 failing tests**

在 `tests/test_ida_analyze_util.py` 的 `TestGenerateYamlDesiredFieldsContract` 中追加以下测试：

```python
    def test_normalize_generate_yaml_desired_fields_parses_vfunc_sig_max_match_directive(
        self,
    ) -> None:
        result = ida_analyze_util._normalize_generate_yaml_desired_fields(
            [
                (
                    "INetworkMessages_GetLoggingChannel",
                    [
                        "func_name",
                        "vfunc_sig",
                        "vfunc_sig_max_match:10",
                        "vfunc_offset",
                        "vfunc_index",
                        "vtable_name",
                    ],
                )
            ],
            debug=True,
        )

        self.assertEqual(
            {
                "INetworkMessages_GetLoggingChannel": {
                    "desired_output_fields": [
                        "func_name",
                        "vfunc_sig",
                        "vfunc_sig_max_match",
                        "vfunc_offset",
                        "vfunc_index",
                        "vtable_name",
                    ],
                    "generation_options": {
                        "vfunc_sig_max_match": 10,
                    },
                }
            },
            result,
        )

    def test_normalize_generate_yaml_desired_fields_rejects_vfunc_sig_max_match_without_vfunc_sig(
        self,
    ) -> None:
        result = ida_analyze_util._normalize_generate_yaml_desired_fields(
            [("Foo", ["func_name", "vfunc_sig_max_match:10"])],
            debug=True,
        )

        self.assertIsNone(result)

    def test_normalize_generate_yaml_desired_fields_rejects_invalid_vfunc_sig_max_match_value(
        self,
    ) -> None:
        result = ida_analyze_util._normalize_generate_yaml_desired_fields(
            [("Foo", ["func_name", "vfunc_sig", "vfunc_sig_max_match:abc"])],
            debug=True,
        )

        self.assertIsNone(result)
```

- [ ] **Step 2: 运行新增测试，确认当前实现失败**

Run:

```bash
python -m unittest \
  tests.test_ida_analyze_util.TestGenerateYamlDesiredFieldsContract.test_normalize_generate_yaml_desired_fields_parses_vfunc_sig_max_match_directive \
  tests.test_ida_analyze_util.TestGenerateYamlDesiredFieldsContract.test_normalize_generate_yaml_desired_fields_rejects_vfunc_sig_max_match_without_vfunc_sig \
  tests.test_ida_analyze_util.TestGenerateYamlDesiredFieldsContract.test_normalize_generate_yaml_desired_fields_rejects_invalid_vfunc_sig_max_match_value \
  -v
```

Expected:

```text
FAIL: ...parses_vfunc_sig_max_match_directive
```

并且当前 `_normalize_generate_yaml_desired_fields(...)` 仍返回旧的 `dict[str, list[str]]` 结构。

- [ ] **Step 3: 实现 directive 规范化、合法字段与 payload 组装调整**

在 `ida_analyze_util.py` 中按下面的形态修改三个位置：

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
    "vfunc_sig_max_match",
]
```

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

        desired_output_fields = []
        generation_options = {}
        seen_directives = set()
        for raw_field in desired_fields:
            if not isinstance(raw_field, str) or not raw_field:
                if debug:
                    print(f"    Preprocess: invalid desired field list for {symbol_name}")
                return None

            if raw_field.startswith("vfunc_sig_max_match:"):
                if "vfunc_sig_max_match" in seen_directives:
                    if debug:
                        print(
                            f"    Preprocess: duplicated vfunc_sig_max_match for {symbol_name}"
                        )
                    return None
                value_text = raw_field.split(":", 1)[1].strip()
                try:
                    value = int(value_text, 10)
                except Exception:
                    if debug:
                        print(
                            f"    Preprocess: invalid vfunc_sig_max_match for {symbol_name}: {raw_field}"
                        )
                    return None
                if value <= 0:
                    if debug:
                        print(
                            f"    Preprocess: vfunc_sig_max_match must be > 0 for {symbol_name}"
                        )
                    return None
                desired_output_fields.append("vfunc_sig_max_match")
                generation_options["vfunc_sig_max_match"] = value
                seen_directives.add("vfunc_sig_max_match")
                continue

            desired_output_fields.append(raw_field)

        if "vfunc_sig_max_match" in generation_options and "vfunc_sig" not in desired_output_fields:
            if debug:
                print(
                    f"    Preprocess: vfunc_sig_max_match requires vfunc_sig for {symbol_name}"
                )
            return None

        normalized[symbol_name] = {
            "desired_output_fields": desired_output_fields,
            "generation_options": generation_options,
        }

    return normalized
```

```python
def _assemble_symbol_payload(symbol_name, target_kind, candidate_data, desired_fields_map, debug=False):
    desired_fields_entry = desired_fields_map.get(symbol_name)
    if desired_fields_entry is None:
        if debug:
            print(f"    Preprocess: missing desired-fields entry for {symbol_name}")
        return None

    desired_fields = desired_fields_entry["desired_output_fields"]
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

- [ ] **Step 4: 回跑契约测试并补一个 payload 持久化断言**

把下面这个测试也加到 `TestGenerateYamlDesiredFieldsContract`：

```python
    async def test_preprocess_common_skill_writes_vfunc_sig_max_match_field(
        self,
    ) -> None:
        with patch.object(
            ida_analyze_util,
            "preprocess_func_sig_via_mcp",
            AsyncMock(
                return_value={
                    "func_name": "Foo",
                    "vfunc_sig": "FF 90 78 00 00 00",
                    "vfunc_sig_max_match": 10,
                    "vtable_name": "Bar",
                    "vfunc_offset": "0x78",
                    "vfunc_index": 15,
                }
            ),
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
                expected_outputs=["/tmp/Foo.windows.yaml"],
                old_yaml_map={},
                new_binary_dir="/tmp",
                platform="windows",
                image_base=0x180000000,
                func_names=["Foo"],
                generate_yaml_desired_fields=[
                    (
                        "Foo",
                        [
                            "func_name",
                            "vfunc_sig",
                            "vfunc_sig_max_match:10",
                            "vtable_name",
                            "vfunc_offset",
                            "vfunc_index",
                        ],
                    )
                ],
                debug=True,
            )

        self.assertTrue(result)
        self.assertEqual(
            {
                "func_name": "Foo",
                "vtable_name": "Bar",
                "vfunc_offset": "0x78",
                "vfunc_index": 15,
                "vfunc_sig": "FF 90 78 00 00 00",
                "vfunc_sig_max_match": 10,
            },
            mock_write_func_yaml.call_args.args[1],
        )
```

Run:

```bash
python -m unittest tests.test_ida_analyze_util.TestGenerateYamlDesiredFieldsContract -v
```

Expected:

```text
OK
```

- [ ] **Step 5: 提交契约与 payload 调整**

```bash
git add ida_analyze_util.py tests/test_ida_analyze_util.py
git commit -m "test(preprocess): 补齐vfunc多匹配契约测试"
```

## Task 2: 锁定 `preprocess_gen_vfunc_sig_via_mcp(...)` 的多匹配收敛行为

**Files:**
- Modify: `tests/test_ida_analyze_util.py`
- Modify: `ida_analyze_util.py`

- [ ] **Step 1: 先写 `<= N` 成功与 `> N` 继续扩展的 failing tests**

在 `tests/test_ida_analyze_util.py` 的 `test_preprocess_gen_vfunc_sig_via_mcp_generates_current_version_sig` 后面追加：

```python
    async def test_preprocess_gen_vfunc_sig_via_mcp_accepts_match_count_within_limit(
        self,
    ) -> None:
        session = AsyncMock()

        def _fake_call_tool(*, name: str, arguments: dict[str, object]):
            if name == "py_eval":
                return _py_eval_payload(
                    {
                        "vfunc_sig_va": "0x18004abc3",
                        "vfunc_inst_length": 6,
                        "vfunc_disp_offset": 2,
                        "vfunc_disp_size": 4,
                        "insts": [
                            {
                                "ea": "0x18004abc3",
                                "size": 6,
                                "bytes": "ff9078000000",
                                "wild": [],
                            },
                            {
                                "ea": "0x18004abc9",
                                "size": 3,
                                "bytes": "4885c0",
                                "wild": [],
                            },
                        ],
                    }
                )
            if name == "find_bytes":
                self.assertEqual(11, arguments["limit"])
                return _FakeCallToolResult(
                    [
                        {
                            "matches": ["0x18004abc3", "0x18004abd0"],
                            "n": 2,
                        }
                    ]
                )
            raise AssertionError(f"unexpected MCP tool: {name}")

        session.call_tool.side_effect = _fake_call_tool

        result = await ida_analyze_util.preprocess_gen_vfunc_sig_via_mcp(
            session=session,
            inst_va="0x18004ABC3",
            vfunc_offset="0x78",
            max_match_count=10,
            debug=True,
        )

        self.assertEqual("FF 90 78 00 00 00", result["vfunc_sig"])
        self.assertEqual(10, result["vfunc_sig_max_match"])

    async def test_preprocess_gen_vfunc_sig_via_mcp_rejects_match_count_over_limit(
        self,
    ) -> None:
        session = AsyncMock()

        def _fake_call_tool(*, name: str, arguments: dict[str, object]):
            if name == "py_eval":
                return _py_eval_payload(
                    {
                        "vfunc_sig_va": "0x18004abc3",
                        "vfunc_inst_length": 6,
                        "vfunc_disp_offset": 2,
                        "vfunc_disp_size": 4,
                        "insts": [
                            {
                                "ea": "0x18004abc3",
                                "size": 6,
                                "bytes": "ff9078000000",
                                "wild": [],
                            },
                            {
                                "ea": "0x18004abc9",
                                "size": 3,
                                "bytes": "4885c0",
                                "wild": [],
                            },
                        ],
                    }
                )
            if name == "find_bytes":
                return _FakeCallToolResult(
                    [
                        {
                            "matches": [
                                "0x18004abc3",
                                "0x18004abd0",
                                "0x18004abe0",
                            ],
                            "n": 11,
                        }
                    ]
                )
            raise AssertionError(f"unexpected MCP tool: {name}")

        session.call_tool.side_effect = _fake_call_tool

        result = await ida_analyze_util.preprocess_gen_vfunc_sig_via_mcp(
            session=session,
            inst_va="0x18004ABC3",
            vfunc_offset="0x78",
            max_match_count=10,
            debug=True,
        )

        self.assertIsNone(result)
```

- [ ] **Step 2: 运行这两条测试，确认当前实现仍按唯一匹配失败**

Run:

```bash
python -m unittest \
  tests.test_ida_analyze_util.TestLlmDecompileSupport.test_preprocess_gen_vfunc_sig_via_mcp_accepts_match_count_within_limit \
  tests.test_ida_analyze_util.TestLlmDecompileSupport.test_preprocess_gen_vfunc_sig_via_mcp_rejects_match_count_over_limit \
  -v
```

Expected:

```text
FAIL: ...accepts_match_count_within_limit
```

- [ ] **Step 3: 实现 `max_match_count` 参数与 `limit = N + 1` 逻辑**

在 `ida_analyze_util.py` 的 `preprocess_gen_vfunc_sig_via_mcp(...)` 中按下面形态修改：

```python
async def preprocess_gen_vfunc_sig_via_mcp(
    session,
    inst_va,
    vfunc_offset,
    min_sig_bytes=6,
    max_sig_bytes=96,
    max_instructions=64,
    max_match_count=1,
    extra_wildcard_offsets=None,
    debug=False,
):
    try:
        max_match_count = max(1, int(max_match_count))
    except Exception:
        if debug:
            print("    Preprocess: invalid max_match_count for vfunc_sig")
        return None
```

并把 `find_bytes` 调用和判定改成：

```python
            fb_result = await session.call_tool(
                name="find_bytes",
                arguments={
                    "patterns": [candidate_sig],
                    "limit": max_match_count + 1,
                },
            )
```

```python
        matches = entry.get("matches", [])
        match_count = entry.get("n", len(matches))

        if match_count < 1 or match_count > max_match_count:
            continue

        normalized_matches = {str(match).lower() for match in matches}
        if hex(inst_va_int).lower() not in normalized_matches:
            continue

        best_sig = candidate_sig
        best_sig_len = prefix_len
        break
```

返回值补齐：

```python
    return {
        "vfunc_sig": best_sig,
        "vfunc_sig_va": hex(inst_va_int),
        "vfunc_sig_disp": 0,
        "vfunc_inst_length": first_len,
        "vfunc_disp_offset": disp_off,
        "vfunc_disp_size": disp_size,
        "vfunc_offset": hex(vfunc_offset_int),
        "vfunc_sig_max_match": max_match_count,
    }
```

- [ ] **Step 4: 回跑 `vfunc_sig` 生成测试**

Run:

```bash
python -m unittest \
  tests.test_ida_analyze_util.TestLlmDecompileSupport.test_preprocess_gen_vfunc_sig_via_mcp_generates_current_version_sig \
  tests.test_ida_analyze_util.TestLlmDecompileSupport.test_preprocess_gen_vfunc_sig_via_mcp_accepts_match_count_within_limit \
  tests.test_ida_analyze_util.TestLlmDecompileSupport.test_preprocess_gen_vfunc_sig_via_mcp_rejects_match_count_over_limit \
  -v
```

Expected:

```text
OK
```

- [ ] **Step 5: 提交 `vfunc_sig` 生成器改动**

```bash
git add ida_analyze_util.py tests/test_ida_analyze_util.py
git commit -m "fix(preprocess): 支持vfunc签名多匹配收敛"
```

## Task 3: 打通 slot-only fallback 与目标脚本转发

**Files:**
- Modify: `tests/test_ida_analyze_util.py`
- Modify: `tests/test_ida_preprocessor_scripts.py`
- Modify: `ida_analyze_util.py`
- Modify: `ida_preprocessor_scripts/find-INetworkMessages_GetLoggingChannel-windows.py`

- [ ] **Step 1: 先写 slot-only 透传与脚本转发的 failing tests**

在 `tests/test_ida_analyze_util.py` 的 slot-only 测试旁追加：

```python
        mock_preprocess_gen_vfunc_sig.assert_awaited_once_with(
            session=session,
            inst_va="0x18004abc3",
            vfunc_offset="0x78",
            max_match_count=10,
            debug=True,
        )
        self.assertEqual(10, written_payload["vfunc_sig_max_match"])
```

同时在 `tests/test_ida_preprocessor_scripts.py` 新增：

```python
class TestFindINetworkMessagesGetLoggingChannelWindows(unittest.IsolatedAsyncioTestCase):
    async def test_preprocess_skill_forwards_vfunc_sig_max_match_directive(self) -> None:
        module = _load_module(
            "ida_preprocessor_scripts/find-INetworkMessages_GetLoggingChannel-windows.py",
            "find_INetworkMessages_GetLoggingChannel_windows",
        )
        mock_preprocess_common_skill = AsyncMock(return_value=True)
        llm_config = {
            "model": "gpt-4.1-mini",
            "api_key": "test-api-key",
        }

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
        mock_preprocess_common_skill.assert_awaited_once_with(
            session="session",
            expected_outputs=["out.yaml"],
            old_yaml_map={"k": "v"},
            new_binary_dir="bin_dir",
            platform="windows",
            image_base=0x180000000,
            func_names=["INetworkMessages_GetLoggingChannel"],
            func_vtable_relations=[
                ("INetworkMessages_GetLoggingChannel", "INetworkMessages")
            ],
            llm_decompile_specs=[
                (
                    "INetworkMessages_GetLoggingChannel",
                    "prompt/call_llm_decompile.md",
                    "references/server/CNetworkUtlVectorEmbedded_TryLateResolve_m_vecRenderAttributes.{platform}.yaml",
                )
            ],
            generate_yaml_desired_fields=[
                (
                    "INetworkMessages_GetLoggingChannel",
                    [
                        "func_name",
                        "vfunc_sig",
                        "vfunc_sig_max_match:10",
                        "vfunc_offset",
                        "vfunc_index",
                        "vtable_name",
                    ],
                )
            ],
            llm_config=llm_config,
            debug=True,
        )
```

- [ ] **Step 2: 运行两组测试，确认当前透传尚未打通**

Run:

```bash
python -m unittest \
  tests.test_ida_analyze_util.TestLlmDecompileSupport.test_preprocess_common_skill_uses_slot_only_fallback_when_vtable_unavailable \
  tests.test_ida_preprocessor_scripts.TestFindINetworkMessagesGetLoggingChannelWindows.test_preprocess_skill_forwards_vfunc_sig_max_match_directive \
  -v
```

Expected:

```text
FAIL: expected await not found
```

因为当前 `preprocess_gen_vfunc_sig_via_mcp(...)` 还没有收到 `max_match_count=10`。

- [ ] **Step 3: 在公共层与 helper 中透传 `vfunc_sig_max_match`**

在 `ida_analyze_util.py` 中调整：

```python
async def _build_enriched_slot_only_vfunc_payload_via_mcp(
    session,
    func_name,
    llm_result,
    *,
    vtable_name=None,
    require_vfunc_sig=False,
    require_vtable_name=False,
    vfunc_sig_max_match=1,
    debug=False,
):
```

并在生成成功后补齐：

```python
        payload["vfunc_sig"] = str(vfunc_sig)
        payload["vfunc_sig_max_match"] = int(
            sig_data.get("vfunc_sig_max_match", vfunc_sig_max_match)
        )
```

`preprocess_common_skill(...)` 中读取规范化结果并透传：

```python
        desired_fields_entry = desired_fields_map.get(func_name)
        desired_fields = desired_fields_entry["desired_output_fields"]
        generation_options = desired_fields_entry["generation_options"]
        desired_fields_set = set(desired_fields)
```

```python
                func_data = await _build_enriched_slot_only_vfunc_payload_via_mcp(
                    session=session,
                    func_name=func_name,
                    llm_result=llm_result,
                    vtable_name=fallback_vtable_name,
                    require_vfunc_sig="vfunc_sig" in desired_fields_set,
                    require_vtable_name="vtable_name" in desired_fields_set,
                    vfunc_sig_max_match=generation_options.get("vfunc_sig_max_match", 1),
                    debug=debug,
                )
```

如果目标脚本里还没有该 directive，则保持为：

```python
            "func_name",
            "vfunc_sig",
            "vfunc_sig_max_match:10",
            "vfunc_offset",
            "vfunc_index",
            "vtable_name",
```

- [ ] **Step 4: 回跑 slot-only 与脚本转发测试**

Run:

```bash
python -m unittest \
  tests.test_ida_analyze_util.TestLlmDecompileSupport.test_preprocess_common_skill_uses_slot_only_fallback_when_vtable_unavailable \
  tests.test_ida_preprocessor_scripts.TestFindINetworkMessagesGetLoggingChannelWindows.test_preprocess_skill_forwards_vfunc_sig_max_match_directive \
  -v
```

Expected:

```text
OK
```

- [ ] **Step 5: 提交 slot-only 透传改动**

```bash
git add \
  ida_analyze_util.py \
  tests/test_ida_analyze_util.py \
  tests/test_ida_preprocessor_scripts.py \
  ida_preprocessor_scripts/find-INetworkMessages_GetLoggingChannel-windows.py
git commit -m "fix(preprocess): 透传vfunc多匹配上限"
```

## Task 4: 打通旧 YAML 的 `vfunc_sig` 复用路径

**Files:**
- Modify: `tests/test_ida_analyze_util.py`
- Modify: `ida_analyze_util.py`

- [ ] **Step 1: 先写旧 YAML 复用的 failing tests**

在 `tests/test_ida_analyze_util.py` 中追加一个新测试类：

```python
class TestPreprocessFuncSigViaMcpVfuncSigMaxMatch(unittest.IsolatedAsyncioTestCase):
    async def test_preprocess_func_sig_via_mcp_allows_vfunc_sig_match_count_within_limit(
        self,
    ) -> None:
        session = AsyncMock()

        with tempfile.TemporaryDirectory() as temp_dir:
            old_path = Path(temp_dir) / "old.yaml"
            new_path = Path(temp_dir) / "new.yaml"
            vtable_path = Path(temp_dir) / "INetworkMessages_vtable.windows.yaml"
            old_path.write_text(
                yaml.safe_dump(
                    {
                        "func_name": "INetworkMessages_GetLoggingChannel",
                        "vfunc_sig": "FF 90 20 01 00 00",
                        "vfunc_sig_max_match": 10,
                        "vtable_name": "INetworkMessages",
                        "vfunc_offset": "0x120",
                        "vfunc_index": 36,
                        "func_va": "0x180111111",
                    },
                    sort_keys=False,
                ),
                encoding="utf-8",
            )
            vtable_path.write_text(
                yaml.safe_dump(
                    {
                        "vtable_class": "INetworkMessages",
                        "vtable_entries": {36: "0x180222222"},
                    },
                    sort_keys=False,
                ),
                encoding="utf-8",
            )

            async def _fake_call_tool(*, name, arguments):
                if name == "find_bytes":
                    return _FakeCallToolResult(
                        [
                            {
                                "matches": ["0x1805f0173", "0x1805f01ad"],
                                "n": 2,
                            }
                        ]
                    )
                if name == "py_eval":
                    return _py_eval_payload(
                        {"func_va": "0x180222222", "func_size": "0x40"}
                    )
                raise AssertionError(f"unexpected MCP tool: {name}")

            session.call_tool.side_effect = _fake_call_tool

            result = await ida_analyze_util.preprocess_func_sig_via_mcp(
                session=session,
                new_path=str(new_path),
                old_path=str(old_path),
                image_base=0x180000000,
                new_binary_dir=temp_dir,
                platform="windows",
                func_name="INetworkMessages_GetLoggingChannel",
                debug=True,
            )

        self.assertEqual(10, result["vfunc_sig_max_match"])
        self.assertEqual("FF 90 20 01 00 00", result["vfunc_sig"])
        self.assertEqual(36, result["vfunc_index"])
```

- [ ] **Step 2: 运行旧 YAML 复用测试，确认当前实现仍然要求唯一匹配**

Run:

```bash
python -m unittest \
  tests.test_ida_analyze_util.TestPreprocessFuncSigViaMcpVfuncSigMaxMatch \
  -v
```

Expected:

```text
FAIL: ...matched 2 (need 1)
```

- [ ] **Step 3: 新增仅限 `vfunc_sig` 使用的受限匹配 helper，并写回字段**

在 `ida_analyze_util.py` 的 `preprocess_func_sig_via_mcp(...)` 内保留 `_find_unique_match(...)` 不动，新增：

```python
    async def _find_match_with_limit(signature, label, max_match_count):
        try:
            max_match_count = max(1, int(max_match_count))
        except Exception:
            if debug:
                print(f"    Preprocess: invalid max match count for {label}")
            return None

        try:
            fb_result = await session.call_tool(
                name="find_bytes",
                arguments={
                    "patterns": [signature],
                    "limit": max_match_count + 1,
                },
            )
            fb_data = parse_mcp_result(fb_result)
        except Exception as e:
            if debug:
                print(f"    Preprocess: find_bytes error: {e}")
            return None

        if not isinstance(fb_data, list) or len(fb_data) == 0:
            return None

        entry = fb_data[0]
        if not isinstance(entry, dict):
            return None

        matches = entry.get("matches", [])
        match_count = entry.get("n", len(matches))
        if match_count < 1 or match_count > max_match_count:
            if debug:
                print(
                    f"    Preprocess: {label} matched {match_count} "
                    f"(need <= {max_match_count})"
                )
            return None

        return matches[0]
```

在 `vfunc_sig` fallback 分支读取旧 YAML：

```python
        vfunc_sig_max_match = old_data.get("vfunc_sig_max_match", 1)
        try:
            vfunc_sig_max_match = _parse_int_field(
                vfunc_sig_max_match,
                "vfunc_sig_max_match",
            )
        except Exception:
            if debug:
                print(
                    "    Preprocess: invalid vfunc_sig_max_match in "
                    f"{os.path.basename(old_path)}"
                )
            return None
        if vfunc_sig_max_match <= 0:
            if debug:
                print(
                    "    Preprocess: vfunc_sig_max_match must be > 0 in "
                    f"{os.path.basename(old_path)}"
                )
            return None
```

把匹配调用改成：

```python
        vfunc_match_addr = await _find_match_with_limit(
            vfunc_sig,
            f"{os.path.basename(old_path)} vfunc_sig",
            vfunc_sig_max_match,
        )
```

并在两个 `used_vfunc_fallback` 返回分支都补上：

```python
        new_data["vfunc_sig_max_match"] = vfunc_sig_max_match
```

- [ ] **Step 4: 回跑旧 YAML 复用与 slot-only 相关测试**

Run:

```bash
python -m unittest \
  tests.test_ida_analyze_util.TestPreprocessFuncSigViaMcpVfuncSigMaxMatch \
  tests.test_ida_analyze_util.TestLlmDecompileSupport.test_preprocess_common_skill_fails_when_slot_only_vfunc_sig_generation_fails \
  -v
```

Expected:

```text
OK
```

- [ ] **Step 5: 提交旧 YAML 复用改动**

```bash
git add ida_analyze_util.py tests/test_ida_analyze_util.py
git commit -m "fix(preprocess): 复用vfunc多匹配上限"
```

## Task 5: 跑最终定向回归并整理交付说明

**Files:**
- Modify: `ida_analyze_util.py`
- Modify: `tests/test_ida_analyze_util.py`
- Modify: `tests/test_ida_preprocessor_scripts.py`
- Modify: `ida_preprocessor_scripts/find-INetworkMessages_GetLoggingChannel-windows.py`

- [ ] **Step 1: 跑 `ida_analyze_util` 相关定向回归**

Run:

```bash
python -m unittest tests.test_ida_analyze_util -v
```

Expected:

```text
OK
```

- [ ] **Step 2: 跑脚本转发回归**

Run:

```bash
python -m unittest tests.test_ida_preprocessor_scripts -v
```

Expected:

```text
OK
```

- [ ] **Step 3: 人工检查最终目标脚本与字段顺序**

Run:

```bash
rg -n "vfunc_sig_max_match|FUNC_YAML_ORDER|_normalize_generate_yaml_desired_fields|_find_match_with_limit" \
  ida_analyze_util.py \
  ida_preprocessor_scripts/find-INetworkMessages_GetLoggingChannel-windows.py
```

Expected:

```text
ida_analyze_util.py: ... "vfunc_sig_max_match"
ida_preprocessor_scripts/find-INetworkMessages_GetLoggingChannel-windows.py: ... "vfunc_sig_max_match:10"
```

- [ ] **Step 4: 整理最终变更摘要**

在提交说明或交付说明中明确写出：

```text
1. vfunc_sig 生成支持 <= N 个匹配
2. 旧 YAML 复用支持读取 vfunc_sig_max_match
3. 最终 YAML 会持久化 vfunc_sig_max_match
4. 未声明该字段的旧路径仍保持唯一匹配
```

- [ ] **Step 5: 提交最终回归通过的实现**

```bash
git add \
  ida_analyze_util.py \
  tests/test_ida_analyze_util.py \
  tests/test_ida_preprocessor_scripts.py \
  ida_preprocessor_scripts/find-INetworkMessages_GetLoggingChannel-windows.py
git commit -m "fix(preprocess): 支持vfunc签名多匹配上限"
```
