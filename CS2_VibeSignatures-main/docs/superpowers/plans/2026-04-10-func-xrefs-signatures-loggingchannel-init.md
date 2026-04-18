# Func Xrefs Signatures and LoggingChannel Init Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 让 `preprocess_common_skill(...)` 的 `FUNC_XREFS` 支持 `xref_signatures_list`，全量升级现有 `FUNC_XREFS` 脚本，并新增 Windows/Linux 双平台的 `find-LoggingChannel_Init` regular function 预处理。

**Architecture:** 先用 `unittest` 锁定 5 元组解析、`xref_signatures` 候选集求交和脚本透传行为。然后在 `ida_analyze_util.py` 中新增“字节签名命中地址 -> 所属函数起始地址”的候选集 helper，把它接入 `preprocess_func_xrefs_via_mcp(...)` 与 `preprocess_common_skill(...)` 的 `func_xrefs` 数据流。最后批量迁移 `ida_preprocessor_scripts/` 下所有已有 `FUNC_XREFS`，新增 `LoggingChannel_Init` 分平台脚本和 `config.yaml` 注册，并运行定向测试。

**Tech Stack:** Python 3、`unittest`、`unittest.mock.AsyncMock`、`pathlib`、`ast`、IDA MCP `find_bytes`/`py_eval`

---

## File Structure

- Modify: `ida_analyze_util.py`
  - 新增 `_collect_xref_func_starts_for_signature(...)`
  - 扩展 `preprocess_func_xrefs_via_mcp(...)` 参数与候选集流程
  - 扩展 `_try_preprocess_func_without_llm(...)` 对 `xref_signatures` 的透传
  - 扩展 `preprocess_common_skill(...)` 对 5 元组 `func_xrefs` 的校验和存储
- Modify: `tests/test_ida_analyze_util.py`
  - 新增 `TestFuncXrefsSignatureSupport`
  - 覆盖签名候选集求交、签名候选集为空、5 元组透传、旧 4 元组拒绝
- Create: `ida_preprocessor_scripts/find-LoggingChannel_Init-windows.py`
  - Windows 签名：`C7 44 24 40 64 FF FF FF`
- Create: `ida_preprocessor_scripts/find-LoggingChannel_Init-linux.py`
  - Linux 签名：`41 B8 64 FF FF FF`
- Modify: `config.yaml`
  - 在 `networksystem` 模块注册 `find-LoggingChannel_Init`
  - 在 `networksystem` symbols 段注册 `LoggingChannel_Init`，`category: func`
- Modify: `tests/test_ida_preprocessor_scripts.py`
  - 新增 `LoggingChannel_Init` 双平台脚本透传测试
  - 新增一个已迁移旧 `FUNC_XREFS` 脚本的 5 元组断言
- Modify: all existing files matched by `rg -l "^FUNC_XREFS\s*=" ida_preprocessor_scripts`
  - 当前应覆盖 42 个已有脚本，把 4 元组迁移为 5 元组并插入空 `xref_signatures_list`
- Create: `docs/superpowers/plans/2026-04-10-func-xrefs-signatures-loggingchannel-init.md`
  - 当前实现计划文档

**仓库约束：**

- 当前会话未获显式授权时，不执行 `git commit`；计划中的 commit 步骤仅供已授权执行者使用
- 执行阶段优先跑定向 `unittest`，不要主动扩大全仓测试或 build
- 若后续执行者使用 commit，消息格式遵循仓库约定：`<type>(scope): <中文动词开头摘要>`

## Task 1: 写公共层 failing tests

**Files:**
- Modify: `tests/test_ida_analyze_util.py`

- [ ] **Step 1: 新增 `TestFuncXrefsSignatureSupport` 测试类**

在 `tests/test_ida_analyze_util.py` 中已有 `preprocess_common_skill` 相关测试附近追加：

```python
class TestFuncXrefsSignatureSupport(unittest.IsolatedAsyncioTestCase):
    async def test_preprocess_func_xrefs_intersects_string_and_signature_sets(
        self,
    ) -> None:
        with patch.object(
            ida_analyze_util,
            "_collect_xref_func_starts_for_string",
            AsyncMock(return_value={0x180100000, 0x180200000}),
        ) as mock_collect_string, patch.object(
            ida_analyze_util,
            "_collect_xref_func_starts_for_signature",
            AsyncMock(return_value={0x180200000}),
        ) as mock_collect_signature, patch.object(
            ida_analyze_util,
            "preprocess_gen_func_sig_via_mcp",
            AsyncMock(
                return_value={
                    "func_va": "0x180200000",
                    "func_rva": "0x200000",
                    "func_size": "0x40",
                    "func_sig": "48 89 5C 24 08",
                }
            ),
        ) as mock_gen_sig:
            result = await ida_analyze_util.preprocess_func_xrefs_via_mcp(
                session="session",
                func_name="LoggingChannel_Init",
                xref_strings=["Networking"],
                xref_signatures=["C7 44 24 40 64 FF FF FF"],
                xref_funcs=[],
                exclude_funcs=[],
                new_binary_dir="bin_dir",
                platform="windows",
                image_base=0x180000000,
                debug=True,
            )

        self.assertEqual(
            {
                "func_name": "LoggingChannel_Init",
                "func_va": "0x180200000",
                "func_rva": "0x200000",
                "func_size": "0x40",
                "func_sig": "48 89 5C 24 08",
            },
            result,
        )
        mock_collect_string.assert_awaited_once_with(
            session="session",
            xref_string="Networking",
            debug=True,
        )
        mock_collect_signature.assert_awaited_once_with(
            session="session",
            xref_signature="C7 44 24 40 64 FF FF FF",
            debug=True,
        )
        mock_gen_sig.assert_awaited_once()

    async def test_preprocess_func_xrefs_fails_when_signature_set_is_empty(
        self,
    ) -> None:
        with patch.object(
            ida_analyze_util,
            "_collect_xref_func_starts_for_string",
            AsyncMock(return_value={0x180100000}),
        ), patch.object(
            ida_analyze_util,
            "_collect_xref_func_starts_for_signature",
            AsyncMock(return_value=set()),
        ), patch.object(
            ida_analyze_util,
            "preprocess_gen_func_sig_via_mcp",
            AsyncMock(return_value=None),
        ) as mock_gen_sig:
            result = await ida_analyze_util.preprocess_func_xrefs_via_mcp(
                session="session",
                func_name="LoggingChannel_Init",
                xref_strings=["Networking"],
                xref_signatures=["C7 44 24 40 64 FF FF FF"],
                xref_funcs=[],
                exclude_funcs=[],
                new_binary_dir="bin_dir",
                platform="windows",
                image_base=0x180000000,
                debug=True,
            )

        self.assertIsNone(result)
        mock_gen_sig.assert_not_called()
```

- [ ] **Step 2: 新增 `preprocess_common_skill` 5 元组透传测试**

继续在 `TestFuncXrefsSignatureSupport` 中追加：

```python
    async def test_preprocess_common_skill_forwards_xref_signatures(
        self,
    ) -> None:
        with patch.object(
            ida_analyze_util,
            "preprocess_func_sig_via_mcp",
            AsyncMock(return_value=None),
        ), patch.object(
            ida_analyze_util,
            "preprocess_func_xrefs_via_mcp",
            AsyncMock(
                return_value={
                    "func_name": "LoggingChannel_Init",
                    "func_va": "0x180200000",
                    "func_rva": "0x200000",
                    "func_size": "0x40",
                    "func_sig": "48 89 5C 24 08",
                }
            ),
        ) as mock_func_xrefs, patch.object(
            ida_analyze_util,
            "write_func_yaml",
        ) as mock_write_func_yaml, patch.object(
            ida_analyze_util,
            "_rename_func_in_ida",
            AsyncMock(return_value=None),
        ):
            result = await ida_analyze_util.preprocess_common_skill(
                session="session",
                expected_outputs=["/tmp/LoggingChannel_Init.windows.yaml"],
                old_yaml_map={},
                new_binary_dir="/tmp",
                platform="windows",
                image_base=0x180000000,
                func_names=["LoggingChannel_Init"],
                func_xrefs=[
                    (
                        "LoggingChannel_Init",
                        ["Networking"],
                        ["C7 44 24 40 64 FF FF FF"],
                        [],
                        [],
                    )
                ],
                generate_yaml_desired_fields=[
                    (
                        "LoggingChannel_Init",
                        ["func_name", "func_va", "func_rva", "func_size", "func_sig"],
                    )
                ],
                debug=True,
            )

        self.assertTrue(result)
        mock_func_xrefs.assert_awaited_once()
        self.assertEqual(
            ["C7 44 24 40 64 FF FF FF"],
            mock_func_xrefs.call_args.kwargs["xref_signatures"],
        )
        mock_write_func_yaml.assert_called_once()
```

- [ ] **Step 3: 新增旧 4 元组拒绝测试**

继续在 `TestFuncXrefsSignatureSupport` 中追加：

```python
    async def test_preprocess_common_skill_rejects_legacy_four_item_func_xrefs(
        self,
    ) -> None:
        result = await ida_analyze_util.preprocess_common_skill(
            session="session",
            expected_outputs=["/tmp/LoggingChannel_Init.windows.yaml"],
            old_yaml_map={},
            new_binary_dir="/tmp",
            platform="windows",
            image_base=0x180000000,
            func_names=["LoggingChannel_Init"],
            func_xrefs=[
                (
                    "LoggingChannel_Init",
                    ["Networking"],
                    [],
                    [],
                )
            ],
            generate_yaml_desired_fields=[
                (
                    "LoggingChannel_Init",
                    ["func_name", "func_va", "func_rva", "func_size", "func_sig"],
                )
            ],
            debug=True,
        )

        self.assertFalse(result)
```

- [ ] **Step 4: 新增三类正向约束全空测试**

继续在 `TestFuncXrefsSignatureSupport` 中追加：

```python
    async def test_preprocess_common_skill_rejects_empty_positive_xref_sources(
        self,
    ) -> None:
        result = await ida_analyze_util.preprocess_common_skill(
            session="session",
            expected_outputs=["/tmp/LoggingChannel_Init.windows.yaml"],
            old_yaml_map={},
            new_binary_dir="/tmp",
            platform="windows",
            image_base=0x180000000,
            func_names=["LoggingChannel_Init"],
            func_xrefs=[
                (
                    "LoggingChannel_Init",
                    [],
                    [],
                    [],
                    [],
                )
            ],
            generate_yaml_desired_fields=[
                (
                    "LoggingChannel_Init",
                    ["func_name", "func_va", "func_rva", "func_size", "func_sig"],
                )
            ],
            debug=True,
        )

        self.assertFalse(result)
```

- [ ] **Step 5: 运行测试确认当前失败**

Run:

```bash
python -m unittest tests.test_ida_analyze_util.TestFuncXrefsSignatureSupport -v
```

Expected:

```text
FAILED
```

失败原因应包含至少一个：

- `preprocess_func_xrefs_via_mcp()` 不接受 `xref_signatures`
- `ida_analyze_util` 还不存在 `_collect_xref_func_starts_for_signature`
- `preprocess_common_skill(...)` 仍按旧 4 元组解析

- [ ] **Step 6: 授权时提交测试变更**

仅当用户明确允许 git commit 时执行：

```bash
git add tests/test_ida_analyze_util.py
git commit -m "test(preprocess): 增加FUNC_XREFS签名约束测试"
```

## Task 2: 实现公共层 `xref_signatures` 支持

**Files:**
- Modify: `ida_analyze_util.py`

- [ ] **Step 1: 新增签名候选集 helper**

在 `_collect_xref_func_starts_for_ea(...)` 后、`_get_func_basic_info_via_mcp(...)` 前新增：

```python
async def _collect_xref_func_starts_for_signature(
    session, xref_signature, debug=False
):
    """
    Collect function-start addresses that contain bytes matched by a signature.

    Returns:
        Set[int]: Function start addresses.
    """
    if not isinstance(xref_signature, str) or not xref_signature:
        return set()

    try:
        find_result = await session.call_tool(
            name="find_bytes",
            arguments={"patterns": [xref_signature]},
        )
        find_data = parse_mcp_result(find_result)
    except Exception as e:
        if debug:
            print(f"    Preprocess: find_bytes error for xref signature: {e}")
        return set()

    if not isinstance(find_data, list) or not find_data:
        return set()

    matches = find_data[0].get("matches", [])
    if not isinstance(matches, list) or not matches:
        return set()

    match_addrs = []
    for match in matches:
        try:
            match_addrs.append(_parse_int_value(match))
        except Exception:
            continue
    if not match_addrs:
        return set()

    py_code = (
        "import idaapi, json\n"
        f"match_addrs = {match_addrs!r}\n"
        "func_starts = set()\n"
        "for match_ea in match_addrs:\n"
        "    f = idaapi.get_func(match_ea)\n"
        "    if f:\n"
        "        func_starts.add(f.start_ea)\n"
        "result = json.dumps([hex(ea) for ea in sorted(func_starts)])\n"
    )

    try:
        eval_result = await session.call_tool(
            name="py_eval",
            arguments={"code": py_code},
        )
        eval_data = parse_mcp_result(eval_result)
    except Exception as e:
        if debug:
            print(f"    Preprocess: py_eval error for xref signature search: {e}")
        return set()

    return _parse_func_start_set_from_py_eval(eval_data, debug=debug)
```

- [ ] **Step 2: 扩展 `preprocess_func_xrefs_via_mcp(...)` 签名与文档**

把函数签名改为：

```python
async def preprocess_func_xrefs_via_mcp(
    session,
    func_name,
    xref_strings,
    xref_signatures,
    xref_funcs,
    exclude_funcs,
    new_binary_dir,
    platform,
    image_base,
    vtable_class=None,
    debug=False,
):
```

把 docstring 中候选来源扩展为：

```python
    Resolve target function by intersecting candidate sets from:
    - string xrefs
    - byte signatures mapped to containing function starts
    - xrefs to dependency function addresses from current-version YAML files.
    - vtable entries (when ``vtable_class`` is specified)
```

- [ ] **Step 3: 在 `preprocess_func_xrefs_via_mcp(...)` 中加入签名候选集**

在字符串候选集循环之后、`xref_funcs` 循环之前加入：

```python
    for xref_signature in (xref_signatures or []):
        addr_set = await _collect_xref_func_starts_for_signature(
            session=session,
            xref_signature=xref_signature,
            debug=debug,
        )
        if not addr_set:
            if debug:
                short = str(xref_signature)[:80]
                print(
                    "    Preprocess: empty candidate set for signature xref: "
                    f"{short}"
                )
            return None
        candidate_sets.append(addr_set)
```

- [ ] **Step 4: 扩展 `_try_preprocess_func_without_llm(...)` 透传**

在调用 `preprocess_func_xrefs_via_mcp(...)` 时新增：

```python
            xref_signatures=xref_spec["xref_signatures"],
```

最终参数段应包含：

```python
        func_data = await preprocess_func_xrefs_via_mcp(
            session=session,
            func_name=func_name,
            xref_strings=xref_spec["xref_strings"],
            xref_signatures=xref_spec["xref_signatures"],
            xref_funcs=xref_spec["xref_funcs"],
            exclude_funcs=xref_spec["exclude_funcs"],
            new_binary_dir=new_binary_dir,
            platform=platform,
            image_base=image_base,
            vtable_class=xref_vtable_class,
            debug=debug,
        )
```

- [ ] **Step 5: 扩展 `preprocess_common_skill(...)` 的 `func_xrefs` 解析**

把格式校验从 4 元组改成 5 元组：

```python
    for spec in func_xrefs:
        if not isinstance(spec, (tuple, list)) or len(spec) != 5:
            if debug:
                print(f"    Preprocess: invalid func_xrefs spec: {spec}")
            return False

        func_name, xref_strings, xref_signatures, xref_funcs, exclude_funcs = spec
```

在 `xref_strings` 类型校验之后增加 `xref_signatures` 类型校验：

```python
        if not isinstance(xref_signatures, (tuple, list)):
            if debug:
                print(
                    f"    Preprocess: invalid xref_signatures type for "
                    f"{func_name}: {type(xref_signatures).__name__}"
                )
            return False
```

把列表转换段改为：

```python
        xref_strings = list(xref_strings)
        xref_signatures = list(xref_signatures)
        xref_funcs = list(xref_funcs)
        exclude_funcs = list(exclude_funcs)
```

在 `xref_strings` 值校验之后增加：

```python
        if any(not isinstance(item, str) or not item for item in xref_signatures):
            if debug:
                print(f"    Preprocess: invalid xref_signatures values for {func_name}")
            return False
```

把空正向约束判断改为：

```python
        if not xref_strings and not xref_signatures and not xref_funcs:
            if debug:
                print(f"    Preprocess: empty func_xrefs spec for {func_name}")
            return False
```

把 `func_xrefs_map[func_name]` 改为：

```python
        func_xrefs_map[func_name] = {
            "xref_strings": xref_strings,
            "xref_signatures": xref_signatures,
            "xref_funcs": xref_funcs,
            "exclude_funcs": exclude_funcs,
        }
```

- [ ] **Step 6: 更新 `preprocess_common_skill(...)` docstring**

把 `func_xrefs` 说明改为 5 元组：

```python
    - ``func_xrefs``: locate functions via unified xref fallback through
      ``preprocess_func_xrefs_via_mcp``. Each element is a tuple of
      ``(func_name, xref_strings_list, xref_signatures_list, xref_funcs_list, exclude_funcs_list)``.
      ``xref_signatures`` provides byte signatures whose matches are mapped
      to containing function starts before intersecting with other candidate
      sets.
```

- [ ] **Step 7: 跑 Task 1 定向测试确认通过**

Run:

```bash
python -m unittest tests.test_ida_analyze_util.TestFuncXrefsSignatureSupport -v
```

Expected:

```text
OK
```

- [ ] **Step 8: 授权时提交公共层实现**

仅当用户明确允许 git commit 时执行：

```bash
git add ida_analyze_util.py tests/test_ida_analyze_util.py
git commit -m "feat(preprocess): 支持FUNC_XREFS签名约束"
```

## Task 3: 新增 `LoggingChannel_Init` 脚本与配置

**Files:**
- Create: `ida_preprocessor_scripts/find-LoggingChannel_Init-windows.py`
- Create: `ida_preprocessor_scripts/find-LoggingChannel_Init-linux.py`
- Modify: `config.yaml`

- [ ] **Step 1: 新增 Windows 预处理脚本**

创建 `ida_preprocessor_scripts/find-LoggingChannel_Init-windows.py`：

```python
#!/usr/bin/env python3
"""Preprocess script for find-LoggingChannel_Init skill."""

from ida_analyze_util import preprocess_common_skill

TARGET_FUNCTION_NAMES = [
    "LoggingChannel_Init",
]

FUNC_XREFS = [
    # (func_name, xref_strings_list, xref_signatures_list, xref_funcs_list, exclude_funcs_list)
    (
        "LoggingChannel_Init",
        ["Networking"],
        ["C7 44 24 40 64 FF FF FF"],
        [],
        [],
    ),
]

GENERATE_YAML_DESIRED_FIELDS = [
    # (symbol_name, generate_yaml_fields)
    (
        "LoggingChannel_Init",
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
    """Reuse previous gamever func_sig to locate target function(s) and write YAML."""
    return await preprocess_common_skill(
        session=session,
        expected_outputs=expected_outputs,
        old_yaml_map=old_yaml_map,
        new_binary_dir=new_binary_dir,
        platform=platform,
        image_base=image_base,
        func_names=TARGET_FUNCTION_NAMES,
        func_xrefs=FUNC_XREFS,
        generate_yaml_desired_fields=GENERATE_YAML_DESIRED_FIELDS,
        debug=debug,
    )
```

- [ ] **Step 2: 新增 Linux 预处理脚本**

创建 `ida_preprocessor_scripts/find-LoggingChannel_Init-linux.py`：

```python
#!/usr/bin/env python3
"""Preprocess script for find-LoggingChannel_Init skill."""

from ida_analyze_util import preprocess_common_skill

TARGET_FUNCTION_NAMES = [
    "LoggingChannel_Init",
]

FUNC_XREFS = [
    # (func_name, xref_strings_list, xref_signatures_list, xref_funcs_list, exclude_funcs_list)
    (
        "LoggingChannel_Init",
        ["Networking"],
        ["41 B8 64 FF FF FF"],
        [],
        [],
    ),
]

GENERATE_YAML_DESIRED_FIELDS = [
    # (symbol_name, generate_yaml_fields)
    (
        "LoggingChannel_Init",
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
    """Reuse previous gamever func_sig to locate target function(s) and write YAML."""
    return await preprocess_common_skill(
        session=session,
        expected_outputs=expected_outputs,
        old_yaml_map=old_yaml_map,
        new_binary_dir=new_binary_dir,
        platform=platform,
        image_base=image_base,
        func_names=TARGET_FUNCTION_NAMES,
        func_xrefs=FUNC_XREFS,
        generate_yaml_desired_fields=GENERATE_YAML_DESIRED_FIELDS,
        debug=debug,
    )
```

- [ ] **Step 3: 注册 `find-LoggingChannel_Init` skill**

在 `config.yaml` 的第一个 `networksystem` 模块 `skills` 段中，建议插入在 `find-CNetworkMessages_GetLoggingChannel` 附近：

```yaml
      - name: find-LoggingChannel_Init
        expected_output:
          - LoggingChannel_Init.{platform}.yaml

      - name: find-CNetworkMessages_GetLoggingChannel
        expected_output:
          - CNetworkMessages_GetLoggingChannel.{platform}.yaml
        expected_input:
          - CNetworkMessages_vtable.{platform}.yaml
```

- [ ] **Step 4: 注册 `LoggingChannel_Init` symbol**

在 `config.yaml` 的 `networksystem` symbols 段中，建议插入在 `CNetworkMessages_GetLoggingChannel` 附近：

```yaml
      - name: LoggingChannel_Init
        category: func
        shared: true

      - name: CNetworkMessages_GetLoggingChannel
        category: vfunc
        alias:
          - CNetworkMessages::GetLoggingChannel
        shared: true
```

- [ ] **Step 5: 授权时提交脚本和配置**

仅当用户明确允许 git commit 时执行：

```bash
git add \
  ida_preprocessor_scripts/find-LoggingChannel_Init-windows.py \
  ida_preprocessor_scripts/find-LoggingChannel_Init-linux.py \
  config.yaml
git commit -m "feat(networksystem): 增加LoggingChannel_Init预处理"
```

## Task 4: 迁移所有已有 `FUNC_XREFS` 脚本

**Files:**
- Modify: all files returned by `rg -l "^FUNC_XREFS\s*=" ida_preprocessor_scripts`

- [ ] **Step 1: 记录迁移前脚本清单**

Run:

```bash
rg -l "^FUNC_XREFS\s*=" ida_preprocessor_scripts | sort
```

Expected:

```text
ida_preprocessor_scripts/find-CDemoRecorder_ParseMessage.py
ida_preprocessor_scripts/find-CDemoRecorder_WriteSpawnGroups.py
ida_preprocessor_scripts/find-CEntitySystem_Activate.py
ida_preprocessor_scripts/find-CLoopModeGame_LoopShutdown-linux.py
ida_preprocessor_scripts/find-CLoopModeGame_LoopShutdown-windows.py
ida_preprocessor_scripts/find-CLoopModeGame_ReceivedServerInfo.py
ida_preprocessor_scripts/find-CLoopModeGame_RegisterEventMap.py
ida_preprocessor_scripts/find-CLoopModeGame_SetGameSystemState.py
ida_preprocessor_scripts/find-CLoopModeGame_SetWorldSession.py
ida_preprocessor_scripts/find-CLoopModeGame_Shutdown-linux.py
ida_preprocessor_scripts/find-CLoopTypeClientServerService_OnLoopActivate.py
ida_preprocessor_scripts/find-CLoopTypeClientServer_BuildAndActivateLoopTypes.py
ida_preprocessor_scripts/find-CNetChan_ParseMessagesDemo.py
ida_preprocessor_scripts/find-CNetChan_ParseMessagesDemoInternal.py
ida_preprocessor_scripts/find-CNetChan_ProcessMessages.py
ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntities-linux.py
ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntities-windows.py
ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntitiesInternal-windows.py
ida_preprocessor_scripts/find-CNetworkGameClient_RecordEntityBandwidth.py
ida_preprocessor_scripts/find-CNetworkGameClient_SendMove.py
ida_preprocessor_scripts/find-CNetworkGameClient_SendMovePacket.py
ida_preprocessor_scripts/find-CNetworkMessages_AllocateAndCopyConstructNetMessageAbstract.py
ida_preprocessor_scripts/find-CNetworkMessages_AssociateNetMessageGroupIdWithChannelCategory.py
ida_preprocessor_scripts/find-CNetworkMessages_AssociateNetMessageWithChannelCategoryAbstract.py
ida_preprocessor_scripts/find-CNetworkMessages_ComputeOrderForPriority.py
ida_preprocessor_scripts/find-CNetworkMessages_DeallocateNetMessageAbstract.py
ida_preprocessor_scripts/find-CNetworkMessages_FindOrCreateNetMessage.py
ida_preprocessor_scripts/find-CNetworkMessages_RegisterFieldChangeCallbackPriority.py
ida_preprocessor_scripts/find-CNetworkMessages_RegisterNetworkArrayFieldSerializer.py
ida_preprocessor_scripts/find-CNetworkMessages_RegisterNetworkCategory.py
ida_preprocessor_scripts/find-CNetworkMessages_RegisterNetworkFieldChangeCallbackInternal.py
ida_preprocessor_scripts/find-CNetworkMessages_RegisterNetworkFieldSerializer.py
ida_preprocessor_scripts/find-CNetworkMessages_SerializeInternal.py
ida_preprocessor_scripts/find-CNetworkMessages_SerializeMessageInternal.py
ida_preprocessor_scripts/find-CNetworkMessages_UnserializeFromStream.py
ida_preprocessor_scripts/find-CNetworkMessages_UnserializeMessageInternal.py
ida_preprocessor_scripts/find-CNetworkServerService_Init.py
ida_preprocessor_scripts/find-CNetworkSystem_SendNetworkStats.py
ida_preprocessor_scripts/find-CSceneEntity_GetScriptDesc.py
ida_preprocessor_scripts/find-CSteamworksGameStats_OnReceivedSessionID.py
ida_preprocessor_scripts/find-GetCSceneEntityScriptDesc.py
ida_preprocessor_scripts/find-RegisterSchemaTypeOverride_CEntityHandle.py
```

- [ ] **Step 2: 对每个已有脚本执行同一迁移规则**

将所有旧注释：

```python
# (func_name, xref_strings_list, xref_funcs_list, exclude_funcs_list)
```

改为：

```python
# (func_name, xref_strings_list, xref_signatures_list, xref_funcs_list, exclude_funcs_list)
```

将每个旧 4 元组：

```python
(
    "SomeFunc",
    [...],
    [...],
    [...],
),
```

改为 5 元组：

```python
(
    "SomeFunc",
    [...],
    [],
    [...],
    [...],
),
```

这个空列表就是该旧脚本的 `xref_signatures_list`，旧语义保持不变。

- [ ] **Step 3: 运行静态校验确认没有旧注释**

Run:

```bash
rg -n "# \(func_name, xref_strings_list, xref_funcs_list, exclude_funcs_list\)" ida_preprocessor_scripts
```

Expected:

```text
no matches
```

- [ ] **Step 4: 运行静态校验确认没有旧 4 元组脚本残留**

Run:

```bash
python - <<'PY'
import ast
from pathlib import Path

bad = []
for path in sorted(Path("ida_preprocessor_scripts").glob("*.py")):
    text = path.read_text(encoding="utf-8")
    if "FUNC_XREFS" not in text:
        continue
    tree = ast.parse(text, filename=str(path))
    for node in ast.walk(tree):
        if not isinstance(node, ast.Assign):
            continue
        for target in node.targets:
            if isinstance(target, ast.Name) and target.id == "FUNC_XREFS":
                if not isinstance(node.value, ast.List):
                    bad.append((str(path), "FUNC_XREFS is not a list"))
                    continue
                for index, item in enumerate(node.value.elts):
                    if not isinstance(item, ast.Tuple) or len(item.elts) != 5:
                        bad.append((str(path), f"entry {index} is not 5-tuple"))

if bad:
    for path, msg in bad:
        print(f"{path}: {msg}")
    raise SystemExit(1)
PY
```

Expected:

```text
exit code 0
```

- [ ] **Step 5: 授权时提交脚本迁移**

仅当用户明确允许 git commit 时执行：

```bash
git add ida_preprocessor_scripts
git commit -m "refactor(preprocess): 升级FUNC_XREFS为五元组"
```

## Task 5: 补脚本层测试

**Files:**
- Modify: `tests/test_ida_preprocessor_scripts.py`

- [ ] **Step 1: 新增脚本路径常量**

在现有脚本路径常量附近加入：

```python
LOGGING_CHANNEL_INIT_WINDOWS_SCRIPT_PATH = Path(
    "ida_preprocessor_scripts/find-LoggingChannel_Init-windows.py"
)
LOGGING_CHANNEL_INIT_LINUX_SCRIPT_PATH = Path(
    "ida_preprocessor_scripts/find-LoggingChannel_Init-linux.py"
)
CNETWORK_SERVER_SERVICE_INIT_SCRIPT_PATH = Path(
    "ida_preprocessor_scripts/find-CNetworkServerService_Init.py"
)
```

- [ ] **Step 2: 新增 `LoggingChannel_Init` 双平台脚本透传测试**

在文件末尾 `if __name__ == "__main__":` 前加入：

```python
class TestFindLoggingChannelInit(unittest.IsolatedAsyncioTestCase):
    async def _assert_preprocess_skill_forwards_signature_xrefs(
        self,
        script_path: Path,
        module_name: str,
        expected_signature: str,
        platform: str,
    ) -> None:
        module = _load_module(script_path, module_name)
        mock_preprocess_common_skill = AsyncMock(return_value=True)
        expected_func_xrefs = [
            (
                "LoggingChannel_Init",
                ["Networking"],
                [expected_signature],
                [],
                [],
            )
        ]
        expected_generate_yaml_desired_fields = [
            (
                "LoggingChannel_Init",
                ["func_name", "func_va", "func_rva", "func_size", "func_sig"],
            )
        ]

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
                platform=platform,
                image_base=0x180000000,
                debug=True,
            )

        self.assertTrue(result)
        mock_preprocess_common_skill.assert_awaited_once_with(
            session="session",
            expected_outputs=["out.yaml"],
            old_yaml_map={"k": "v"},
            new_binary_dir="bin_dir",
            platform=platform,
            image_base=0x180000000,
            func_names=["LoggingChannel_Init"],
            func_xrefs=expected_func_xrefs,
            generate_yaml_desired_fields=expected_generate_yaml_desired_fields,
            debug=True,
        )

    async def test_windows_script_forwards_logging_channel_init_xrefs(self) -> None:
        await self._assert_preprocess_skill_forwards_signature_xrefs(
            script_path=LOGGING_CHANNEL_INIT_WINDOWS_SCRIPT_PATH,
            module_name="find_LoggingChannel_Init_windows",
            expected_signature="C7 44 24 40 64 FF FF FF",
            platform="windows",
        )

    async def test_linux_script_forwards_logging_channel_init_xrefs(self) -> None:
        await self._assert_preprocess_skill_forwards_signature_xrefs(
            script_path=LOGGING_CHANNEL_INIT_LINUX_SCRIPT_PATH,
            module_name="find_LoggingChannel_Init_linux",
            expected_signature="41 B8 64 FF FF FF",
            platform="linux",
        )
```

- [ ] **Step 3: 新增已迁移旧脚本的 5 元组断言**

继续在 `if __name__ == "__main__":` 前加入：

```python
class TestFindCNetworkServerServiceInit(unittest.IsolatedAsyncioTestCase):
    async def test_preprocess_skill_forwards_five_item_func_xrefs(self) -> None:
        module = _load_module(
            CNETWORK_SERVER_SERVICE_INIT_SCRIPT_PATH,
            "find_CNetworkServerService_Init",
        )
        mock_preprocess_common_skill = AsyncMock(return_value=True)
        expected_func_xrefs = [
            (
                "CNetworkServerService_Init",
                [
                    "ServerToClient",
                    "Entities",
                    "Local Player",
                    "Other Players",
                ],
                [],
                [],
                [],
            )
        ]
        expected_func_vtable_relations = [
            ("CNetworkServerService_Init", "CNetworkServerService"),
        ]
        expected_generate_yaml_desired_fields = [
            (
                "CNetworkServerService_Init",
                [
                    "func_name",
                    "func_va",
                    "func_rva",
                    "func_size",
                    "func_sig",
                    "vtable_name",
                    "vfunc_offset",
                    "vfunc_index",
                ],
            )
        ]

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
            old_yaml_map={"k": "v"},
            new_binary_dir="bin_dir",
            platform="windows",
            image_base=0x180000000,
            func_names=["CNetworkServerService_Init"],
            func_xrefs=expected_func_xrefs,
            func_vtable_relations=expected_func_vtable_relations,
            generate_yaml_desired_fields=expected_generate_yaml_desired_fields,
            debug=True,
        )
```

- [ ] **Step 4: 运行脚本层定向测试**

Run:

```bash
python -m unittest \
  tests.test_ida_preprocessor_scripts.TestFindLoggingChannelInit \
  tests.test_ida_preprocessor_scripts.TestFindCNetworkServerServiceInit \
  -v
```

Expected:

```text
OK
```

- [ ] **Step 5: 授权时提交脚本测试**

仅当用户明确允许 git commit 时执行：

```bash
git add tests/test_ida_preprocessor_scripts.py
git commit -m "test(preprocess): 覆盖LoggingChannel_Init脚本"
```

## Task 6: 最终定向验证与收尾

**Files:**
- Read: `ida_analyze_util.py`
- Read: `ida_preprocessor_scripts/find-LoggingChannel_Init-windows.py`
- Read: `ida_preprocessor_scripts/find-LoggingChannel_Init-linux.py`
- Read: `config.yaml`
- Read: `tests/test_ida_analyze_util.py`
- Read: `tests/test_ida_preprocessor_scripts.py`

- [ ] **Step 1: 跑公共层和脚本层定向测试**

Run:

```bash
python -m unittest \
  tests.test_ida_analyze_util.TestFuncXrefsSignatureSupport \
  tests.test_ida_preprocessor_scripts.TestFindLoggingChannelInit \
  tests.test_ida_preprocessor_scripts.TestFindCNetworkServerServiceInit \
  -v
```

Expected:

```text
OK
```

- [ ] **Step 2: 确认没有旧 `FUNC_XREFS` 元组残留**

Run:

```bash
python - <<'PY'
import ast
from pathlib import Path

bad = []
for path in sorted(Path("ida_preprocessor_scripts").glob("*.py")):
    text = path.read_text(encoding="utf-8")
    if "FUNC_XREFS" not in text:
        continue
    tree = ast.parse(text, filename=str(path))
    for node in ast.walk(tree):
        if not isinstance(node, ast.Assign):
            continue
        for target in node.targets:
            if isinstance(target, ast.Name) and target.id == "FUNC_XREFS":
                for index, item in enumerate(getattr(node.value, "elts", [])):
                    if not isinstance(item, ast.Tuple) or len(item.elts) != 5:
                        bad.append((str(path), index))

if bad:
    for path, index in bad:
        print(f"{path}: FUNC_XREFS entry {index} is not 5-tuple")
    raise SystemExit(1)
PY
```

Expected:

```text
exit code 0
```

- [ ] **Step 3: 确认 `LoggingChannel_Init` 注册存在**

Run:

```bash
rg -n "find-LoggingChannel_Init|LoggingChannel_Init" config.yaml ida_preprocessor_scripts tests/test_ida_preprocessor_scripts.py
```

Expected:

```text
config.yaml contains find-LoggingChannel_Init
config.yaml contains LoggingChannel_Init
ida_preprocessor_scripts/find-LoggingChannel_Init-windows.py contains LoggingChannel_Init
ida_preprocessor_scripts/find-LoggingChannel_Init-linux.py contains LoggingChannel_Init
tests/test_ida_preprocessor_scripts.py contains TestFindLoggingChannelInit
```

- [ ] **Step 4: 检查工作区差异**

Run:

```bash
git status --short
git diff --stat
```

Expected:

```text
Only files listed in this plan are modified or created.
```

- [ ] **Step 5: 授权时提交最终收尾**

仅当用户明确允许 git commit 时执行：

```bash
git add \
  ida_analyze_util.py \
  config.yaml \
  ida_preprocessor_scripts \
  tests/test_ida_analyze_util.py \
  tests/test_ida_preprocessor_scripts.py \
  docs/superpowers/plans/2026-04-10-func-xrefs-signatures-loggingchannel-init.md \
  docs/superpowers/specs/2026-04-10-preprocess-common-skill-func-xrefs-signatures-design.md
git commit -m "feat(preprocess): 增加FUNC_XREFS签名约束"
```
