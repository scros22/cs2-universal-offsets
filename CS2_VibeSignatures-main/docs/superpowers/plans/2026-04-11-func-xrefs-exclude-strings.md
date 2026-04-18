# Func Xrefs Exclude Strings Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为统一 `func_xrefs` 管线增加通用 `exclude_strings_list` 负向过滤能力，将全仓 `FUNC_XREFS*` schema 从 5 元组统一升级到固定 6 元组，并让 Linux 下 `CBaseEntity_SetStateChanged` 能依靠排除字符串稳定收敛到真实目标函数。

**Architecture:** 先在 `ida_analyze_util.py` 中升级 `preprocess_func_xrefs_via_mcp()`、`_try_preprocess_func_without_llm()` 与 `preprocess_common_skill()` 的统一契约，让 `exclude_strings` 与 `exclude_funcs` 一样在“正向求交后、唯一性判断前”参与负向过滤。随后分批迁移 `ida_preprocessor_scripts/` 下全部 `FUNC_XREFS*` 声明到 6 元组格式，对无需求脚本统一补空列表 `[]`，只保留 `find-CBaseEntity_SetStateChanged.py` 的 Linux 特例字符串。最后用 AST 静态检查、`py_compile` 与 monkeypatch 探针完成无 IDA 依赖的回归验证，并同步 Serena memory。

**Tech Stack:** Python 3、PyYAML、IDA MCP `py_eval`、`rg`、`uv`

---

## File Structure

- Modify: `ida_analyze_util.py`
  - `preprocess_func_xrefs_via_mcp()` 新增 `exclude_strings` 参数与字符串排除并集逻辑
  - `_try_preprocess_func_without_llm()` 透传 `exclude_strings`
  - `preprocess_common_skill()` 将 `func_xrefs` 契约升级为固定 6 元组
  - 更新 `func_xrefs` 相关 docstring、参数说明与 debug 输出
- Modify: `ida_preprocessor_scripts/find-CBaseEntity_SetStateChanged.py`
  - Windows 侧也升级到 6 元组
  - Linux 侧保留 `exclude_strings_list = ["CNetworkTransmitComponent::StateChanged(%s) @%s:%d"]`
- Modify: `ida_preprocessor_scripts/find-CBaseEntity_Spawn.py`
- Modify: `ida_preprocessor_scripts/find-CBaseEntity_SpawnRadius.py`
- Modify: `ida_preprocessor_scripts/find-CDemoRecorder_ParseMessage.py`
- Modify: `ida_preprocessor_scripts/find-CDemoRecorder_WriteSpawnGroups.py`
- Modify: `ida_preprocessor_scripts/find-CEntitySystem_Activate.py`
- Modify: `ida_preprocessor_scripts/find-CGameSceneNode_PostSpawnKeyValues.py`
- Modify: `ida_preprocessor_scripts/find-CSceneEntity_GetScriptDesc.py`
- Modify: `ida_preprocessor_scripts/find-CSteamworksGameStats_OnReceivedSessionID.py`
- Modify: `ida_preprocessor_scripts/find-GetCSceneEntityScriptDesc.py`
- Modify: `ida_preprocessor_scripts/find-RegisterSchemaTypeOverride_CEntityHandle.py`
- Modify: `ida_preprocessor_scripts/find-CLoopModeGame_LoopShutdown-linux.py`
- Modify: `ida_preprocessor_scripts/find-CLoopModeGame_LoopShutdown-windows.py`
- Modify: `ida_preprocessor_scripts/find-CLoopModeGame_ReceivedServerInfo.py`
- Modify: `ida_preprocessor_scripts/find-CLoopModeGame_RegisterEventMap.py`
- Modify: `ida_preprocessor_scripts/find-CLoopModeGame_SetGameSystemState.py`
- Modify: `ida_preprocessor_scripts/find-CLoopModeGame_SetWorldSession.py`
- Modify: `ida_preprocessor_scripts/find-CLoopModeGame_Shutdown-linux.py`
- Modify: `ida_preprocessor_scripts/find-CLoopTypeClientServerService_OnLoopActivate.py`
- Modify: `ida_preprocessor_scripts/find-CLoopTypeClientServer_BuildAndActivateLoopTypes.py`
- Modify: `ida_preprocessor_scripts/find-CNetChan_ParseMessagesDemo.py`
- Modify: `ida_preprocessor_scripts/find-CNetChan_ParseMessagesDemoInternal.py`
- Modify: `ida_preprocessor_scripts/find-CNetChan_ProcessMessages.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntities-linux.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntities-windows.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntitiesInternal-windows.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkGameClient_RecordEntityBandwidth.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkGameClient_SendMove.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkGameClient_SendMovePacket.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkServerService_Init.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkSystem_SendNetworkStats.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkTransmitComponent_StateChanged.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkUtlVectorEmbedded_NetworkStateChanged_m_vecRenderAttributes.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_AllocateAndCopyConstructNetMessageAbstract.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_AssociateNetMessageGroupIdWithChannelCategory.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_AssociateNetMessageWithChannelCategoryAbstract.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_ComputeOrderForPriority.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_ConfirmAllMessageHandlersInstalled.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_DeallocateNetMessageAbstract.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_FindNetworkMessage.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_FindOrCreateNetMessage.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_RegisterFieldChangeCallbackPriority.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_RegisterNetworkArrayFieldSerializer.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_RegisterNetworkCategory.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_RegisterNetworkFieldChangeCallbackInternal.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_RegisterNetworkFieldSerializer.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_SerializeInternal.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_SerializeMessageInternal.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_UnserializeFromStream.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_UnserializeMessageInternal.py`
- Memory: `preprocess_common_skill_func_xrefs`
  - 更新统一 `func_xrefs` 六元组契约、`exclude_strings` 语义与非硬失败规则

说明：仓库当前没有现成的 Python 单元测试目录；本计划用 `uv run python - <<'PY'` AST/monkeypatch 探针与 `python -m py_compile` 做最小充分验证，不额外引入测试框架。

### Task 1: 升级 `ida_analyze_util.py` 的统一 `func_xrefs` 契约

**Files:**
- Modify: `ida_analyze_util.py`

- [ ] **Step 1: 先写一个失败的未来态探针**

Run:

```bash
uv run python - <<'PY'
from pathlib import Path

text = Path("ida_analyze_util.py").read_text(encoding="utf-8")

assert "len(spec) != 6" in text, "func_xrefs parser still not upgraded to 6-tuple"
assert "\"exclude_strings\"" in text, "func_xrefs_map still missing exclude_strings key"
assert "exclude_strings=xref_spec[\"exclude_strings\"]" in text, "fallback call still not forwarding exclude_strings"
assert "excluded_string_func_addrs" in text, "negative string filter set still missing"
PY
```

Expected: FAIL，至少报出一个 `still missing` 或 `still not upgraded`，因为当前实现仍是 5 元组且没有 `exclude_strings` 过滤。

- [ ] **Step 2: 修改 `ida_analyze_util.py` 的签名、解析和过滤逻辑**

将 `preprocess_func_xrefs_via_mcp()` 的签名升级为：

```python
async def preprocess_func_xrefs_via_mcp(
    session,
    func_name,
    xref_strings,
    xref_signatures,
    xref_funcs,
    exclude_funcs,
    exclude_strings,
    new_binary_dir,
    platform,
    image_base,
    vtable_class=None,
    debug=False,
):
```

把函数说明中的契约描述改成包含 `exclude_strings`：

```python
    Additionally, ``exclude_funcs`` can provide function names whose
    ``func_va`` values are loaded from current-version YAML files in
    ``new_binary_dir`` and removed from the intersection result before
    enforcing the uniqueness check.

    ``exclude_strings`` uses the same substring matching semantics as
    ``xref_strings``. For each configured string, collect the containing
    function-start set from its xrefs, union these sets, and subtract the
    result from the already-intersected positive candidate set. Empty
    exclude-string matches are treated as an empty exclusion set rather
    than a hard failure.
```

在正向交集与唯一性判断之间插入字符串排除并集逻辑：

```python
    excluded_string_func_addrs = set()
    for excluded_string in (exclude_strings or []):
        addr_set = await _collect_xref_func_starts_for_string(
            session=session,
            xref_string=excluded_string,
            debug=debug,
        )
        if debug:
            short = str(excluded_string)[:80]
            print(
                f"    Preprocess: exclude string xref '{short}' matched "
                f"{len(addr_set)} function(s)"
            )
        excluded_string_func_addrs |= set(addr_set)

    if debug and excluded_string_func_addrs:
        print(
            "    Preprocess: excluded_string_func_addrs = "
            f"{[hex(a) for a in sorted(excluded_string_func_addrs)]}"
        )

    if debug:
        print(
            "    Preprocess: common_funcs before excludes = "
            f"{[hex(a) for a in sorted(common_funcs)]}"
        )

    if excluded_func_addrs:
        common_funcs -= excluded_func_addrs
    if excluded_string_func_addrs:
        common_funcs -= excluded_string_func_addrs

    if debug:
        print(
            "    Preprocess: common_funcs after excludes = "
            f"{[hex(a) for a in sorted(common_funcs)]}"
        )
```

把 `_try_preprocess_func_without_llm()` 的透传补齐：

```python
        func_data = await preprocess_func_xrefs_via_mcp(
            session=session,
            func_name=func_name,
            xref_strings=xref_spec["xref_strings"],
            xref_signatures=xref_spec["xref_signatures"],
            xref_funcs=xref_spec["xref_funcs"],
            exclude_funcs=xref_spec["exclude_funcs"],
            exclude_strings=xref_spec["exclude_strings"],
            new_binary_dir=new_binary_dir,
            platform=platform,
            image_base=image_base,
            vtable_class=xref_vtable_class,
            debug=debug,
        )
```

把 `preprocess_common_skill()` 中的 docstring 与解析逻辑统一升级到 6 元组：

```python
    - ``func_xrefs``: locate functions via unified xref fallback through
      ``preprocess_func_xrefs_via_mcp``. Each element is a tuple of
      ``(func_name, xref_strings_list, xref_signatures_list,
      xref_funcs_list, exclude_funcs_list, exclude_strings_list)``.
```

```python
    for spec in func_xrefs:
        if not isinstance(spec, (tuple, list)) or len(spec) != 6:
            if debug:
                print(f"    Preprocess: invalid func_xrefs spec: {spec}")
            return False

        (
            func_name,
            xref_strings,
            xref_signatures,
            xref_funcs,
            exclude_funcs,
            exclude_strings,
        ) = spec
```

```python
        if not isinstance(exclude_strings, (tuple, list)):
            if debug:
                print(
                    f"    Preprocess: invalid exclude_strings type for "
                    f"{func_name}: {type(exclude_strings).__name__}"
                )
            return False

        xref_strings = list(xref_strings)
        xref_signatures = list(xref_signatures)
        xref_funcs = list(xref_funcs)
        exclude_funcs = list(exclude_funcs)
        exclude_strings = list(exclude_strings)
```

```python
        if any(not isinstance(item, str) or not item for item in exclude_strings):
            if debug:
                print(
                    f"    Preprocess: invalid exclude_strings values for {func_name}"
                )
            return False
```

```python
        func_xrefs_map[func_name] = {
            "xref_strings": xref_strings,
            "xref_signatures": xref_signatures,
            "xref_funcs": xref_funcs,
            "exclude_funcs": exclude_funcs,
            "exclude_strings": exclude_strings,
        }
```

- [ ] **Step 3: 用 monkeypatch 探针验证新语义**

Run:

```bash
uv run python - <<'PY'
import asyncio
import ida_analyze_util as util

ORIGINALS = (
    util._read_yaml_file,
    util._collect_xref_func_starts_for_string,
    util._collect_xref_func_starts_for_ea,
    util.preprocess_gen_func_sig_via_mcp,
    util._get_func_basic_info_via_mcp,
)


def fake_read_yaml(path):
    if path.endswith("Dep.windows.yaml"):
        return {"func_va": "0x5000"}
    if path.endswith("CBaseEntity_vtable.windows.yaml"):
        return {
            "vtable_entries": {
                0: "0x1000",
                1: "0x2000",
            }
        }
    raise AssertionError(path)


async def fake_collect_string(session, xref_string, debug=False):
    mapping = {
        "keep": {0x1000, 0x2000},
        "drop": {0x2000},
        "already-unique": {0x1000},
        "missing": set(),
    }
    return set(mapping.get(xref_string, set()))


async def fake_collect_ea(session, target_ea, debug=False):
    assert target_ea == 0x5000
    return {0x1000, 0x2000}


async def fake_gen_sig(session, func_va, image_base, debug=False):
    return {
        "func_va": hex(func_va),
        "func_rva": hex(func_va - image_base),
        "func_size": "0x20",
        "func_sig": "AA BB CC",
    }


async def fake_basic(session, func_va, image_base, debug=False):
    return {
        "func_va": hex(func_va),
        "func_rva": hex(func_va - image_base),
        "func_size": "0x20",
    }


async def main():
    util._read_yaml_file = fake_read_yaml
    util._collect_xref_func_starts_for_string = fake_collect_string
    util._collect_xref_func_starts_for_ea = fake_collect_ea
    util.preprocess_gen_func_sig_via_mcp = fake_gen_sig
    util._get_func_basic_info_via_mcp = fake_basic

    result = await util.preprocess_func_xrefs_via_mcp(
        session=object(),
        func_name="CBaseEntity_SetStateChanged",
        xref_strings=["keep"],
        xref_signatures=[],
        xref_funcs=["Dep"],
        exclude_funcs=[],
        exclude_strings=["drop"],
        new_binary_dir="tmp",
        platform="windows",
        image_base=0x1000,
        vtable_class="CBaseEntity",
        debug=True,
    )
    assert result["func_va"] == "0x1000", result

    result = await util.preprocess_func_xrefs_via_mcp(
        session=object(),
        func_name="AlreadyUnique",
        xref_strings=["already-unique"],
        xref_signatures=[],
        xref_funcs=["Dep"],
        exclude_funcs=[],
        exclude_strings=["missing"],
        new_binary_dir="tmp",
        platform="windows",
        image_base=0x1000,
        vtable_class="CBaseEntity",
        debug=True,
    )
    assert result["func_va"] == "0x1000", result


try:
    asyncio.run(main())
finally:
    (
        util._read_yaml_file,
        util._collect_xref_func_starts_for_string,
        util._collect_xref_func_starts_for_ea,
        util.preprocess_gen_func_sig_via_mcp,
        util._get_func_basic_info_via_mcp,
    ) = ORIGINALS

print("probe ok")
PY

python -m py_compile ida_analyze_util.py
```

Expected:
- 第一段输出 `probe ok`
- 第二段无输出且退出码为 `0`

- [ ] **Step 4: 提交核心契约改动**

Run:

```bash
git add ida_analyze_util.py
git commit -m "refactor(func-xrefs): 增加exclude_strings过滤"
```

Expected: 生成 1 个 commit，提交信息为 `refactor(func-xrefs): 增加exclude_strings过滤`。

### Task 2: 迁移实体、场景与杂项脚本到 6 元组

**Files:**
- Modify: `ida_preprocessor_scripts/find-CBaseEntity_SetStateChanged.py`
- Modify: `ida_preprocessor_scripts/find-CBaseEntity_Spawn.py`
- Modify: `ida_preprocessor_scripts/find-CBaseEntity_SpawnRadius.py`
- Modify: `ida_preprocessor_scripts/find-CDemoRecorder_ParseMessage.py`
- Modify: `ida_preprocessor_scripts/find-CDemoRecorder_WriteSpawnGroups.py`
- Modify: `ida_preprocessor_scripts/find-CEntitySystem_Activate.py`
- Modify: `ida_preprocessor_scripts/find-CGameSceneNode_PostSpawnKeyValues.py`
- Modify: `ida_preprocessor_scripts/find-CSceneEntity_GetScriptDesc.py`
- Modify: `ida_preprocessor_scripts/find-CSteamworksGameStats_OnReceivedSessionID.py`
- Modify: `ida_preprocessor_scripts/find-GetCSceneEntityScriptDesc.py`
- Modify: `ida_preprocessor_scripts/find-RegisterSchemaTypeOverride_CEntityHandle.py`

- [ ] **Step 1: 先让分组 AST 校验失败**

Run:

```bash
uv run python - <<'PY'
import ast
from pathlib import Path

FILES = [
    "ida_preprocessor_scripts/find-CBaseEntity_SetStateChanged.py",
    "ida_preprocessor_scripts/find-CBaseEntity_Spawn.py",
    "ida_preprocessor_scripts/find-CBaseEntity_SpawnRadius.py",
    "ida_preprocessor_scripts/find-CDemoRecorder_ParseMessage.py",
    "ida_preprocessor_scripts/find-CDemoRecorder_WriteSpawnGroups.py",
    "ida_preprocessor_scripts/find-CEntitySystem_Activate.py",
    "ida_preprocessor_scripts/find-CGameSceneNode_PostSpawnKeyValues.py",
    "ida_preprocessor_scripts/find-CSceneEntity_GetScriptDesc.py",
    "ida_preprocessor_scripts/find-CSteamworksGameStats_OnReceivedSessionID.py",
    "ida_preprocessor_scripts/find-GetCSceneEntityScriptDesc.py",
    "ida_preprocessor_scripts/find-RegisterSchemaTypeOverride_CEntityHandle.py",
]

errors = []
for path_str in FILES:
    tree = ast.parse(Path(path_str).read_text(encoding="utf-8-sig"), filename=path_str)
    for node in ast.walk(tree):
        if not isinstance(node, ast.Assign):
            continue
        for target in node.targets:
            if isinstance(target, ast.Name) and target.id.startswith("FUNC_XREFS"):
                assert isinstance(node.value, ast.List), path_str
                for index, entry in enumerate(node.value.elts):
                    assert isinstance(entry, ast.Tuple), path_str
                    if len(entry.elts) != 6:
                        errors.append(f"{path_str}[{index}] len={len(entry.elts)}")

assert errors, "expected at least one legacy func_xrefs tuple before migration"
raise SystemExit("\n".join(errors[:20]))
PY
```

Expected: FAIL，并打印若干 `len=5` 的旧 schema 命中。

- [ ] **Step 2: 把本组所有 `FUNC_XREFS*` 统一补成 6 元组**

对所有普通脚本，把 tuple 注释与条目统一改成下面的形状；下面使用 `find-CBaseEntity_Spawn.py` 的真实条目作为模板：

```python
FUNC_XREFS = [
    # (func_name, xref_strings_list, xref_signatures_list, xref_funcs_list, exclude_funcs_list, exclude_strings_list)
    (
        "CBaseEntity_Spawn",
        [
            "hammerUniqueId",
        ],
        [
            "38 A0 63 A9"
        ],
        [],
        ["CGameSceneNode_PostSpawnKeyValues", "CBaseEntity_SpawnRadius"],
        [],
    ),
]
```

对同时声明 `FUNC_XREFS_WINDOWS` / `FUNC_XREFS_LINUX` 的脚本，两侧都改成 6 元组；下面使用 `find-CBaseEntity_SpawnRadius.py` 的真实条目作为模板：

```python
FUNC_XREFS_WINDOWS = [
    # (func_name, xref_strings_list, xref_signatures_list, xref_funcs_list, exclude_funcs_list, exclude_strings_list)
    (
        "CBaseEntity_SpawnRadius",
        [
            "radius",
            "hammerUniqueId",
        ],
        [
            "DC CD AB 6D"
        ],
        [],
        [],
        [],
    ),
]

FUNC_XREFS_LINUX = [
    # (func_name, xref_strings_list, xref_signatures_list, xref_funcs_list, exclude_funcs_list, exclude_strings_list)
    (
        "CBaseEntity_SpawnRadius",
        [
            "radius",
            "hammerUniqueId",
        ],
        [
            "20 59 41 31"
        ],
        [],
        [],
        [],
        [],
    ),
]
```

`find-CBaseEntity_SetStateChanged.py` 必须保留 Linux 特例：

```python
FUNC_XREFS_WINDOWS = [
    # (func_name, xref_strings_list, xref_signatures_list, xref_funcs_list, exclude_funcs_list, exclude_strings_list)
    (
        "CBaseEntity_SetStateChanged",
        [
            "CNetworkTransmitComponent::StateChanged(%s) @%s:%d",
        ],
        [],
        ["CNetworkTransmitComponent_StateChanged"],
        [],
        [],
    ),
]

FUNC_XREFS_LINUX = [
    # (func_name, xref_strings_list, xref_signatures_list, xref_funcs_list, exclude_funcs_list, exclude_strings_list)
    (
        "CBaseEntity_SetStateChanged",
        [],
        [],
        ["CNetworkTransmitComponent_StateChanged"],
        [],
        [
            "CNetworkTransmitComponent::StateChanged(%s) @%s:%d",
        ],
    ),
]
```

- [ ] **Step 3: 重新运行分组校验并编译这些脚本**

Run:

```bash
uv run python - <<'PY'
import ast
from pathlib import Path

FILES = [
    "ida_preprocessor_scripts/find-CBaseEntity_SetStateChanged.py",
    "ida_preprocessor_scripts/find-CBaseEntity_Spawn.py",
    "ida_preprocessor_scripts/find-CBaseEntity_SpawnRadius.py",
    "ida_preprocessor_scripts/find-CDemoRecorder_ParseMessage.py",
    "ida_preprocessor_scripts/find-CDemoRecorder_WriteSpawnGroups.py",
    "ida_preprocessor_scripts/find-CEntitySystem_Activate.py",
    "ida_preprocessor_scripts/find-CGameSceneNode_PostSpawnKeyValues.py",
    "ida_preprocessor_scripts/find-CSceneEntity_GetScriptDesc.py",
    "ida_preprocessor_scripts/find-CSteamworksGameStats_OnReceivedSessionID.py",
    "ida_preprocessor_scripts/find-GetCSceneEntityScriptDesc.py",
    "ida_preprocessor_scripts/find-RegisterSchemaTypeOverride_CEntityHandle.py",
]

errors = []
for path_str in FILES:
    tree = ast.parse(Path(path_str).read_text(encoding="utf-8-sig"), filename=path_str)
    for node in ast.walk(tree):
        if not isinstance(node, ast.Assign):
            continue
        for target in node.targets:
            if isinstance(target, ast.Name) and target.id.startswith("FUNC_XREFS"):
                assert isinstance(node.value, ast.List), path_str
                for index, entry in enumerate(node.value.elts):
                    assert isinstance(entry, ast.Tuple), path_str
                    if len(entry.elts) != 6:
                        errors.append(f"{path_str}[{index}] len={len(entry.elts)}")

assert not errors, "\n".join(errors)
print("group ok")
PY

python -m py_compile \
  ida_preprocessor_scripts/find-CBaseEntity_SetStateChanged.py \
  ida_preprocessor_scripts/find-CBaseEntity_Spawn.py \
  ida_preprocessor_scripts/find-CBaseEntity_SpawnRadius.py \
  ida_preprocessor_scripts/find-CDemoRecorder_ParseMessage.py \
  ida_preprocessor_scripts/find-CDemoRecorder_WriteSpawnGroups.py \
  ida_preprocessor_scripts/find-CEntitySystem_Activate.py \
  ida_preprocessor_scripts/find-CGameSceneNode_PostSpawnKeyValues.py \
  ida_preprocessor_scripts/find-CSceneEntity_GetScriptDesc.py \
  ida_preprocessor_scripts/find-CSteamworksGameStats_OnReceivedSessionID.py \
  ida_preprocessor_scripts/find-GetCSceneEntityScriptDesc.py \
  ida_preprocessor_scripts/find-RegisterSchemaTypeOverride_CEntityHandle.py
```

Expected:
- 第一段输出 `group ok`
- 第二段无输出且退出码为 `0`

- [ ] **Step 4: 提交本组脚本迁移**

Run:

```bash
git add \
  ida_preprocessor_scripts/find-CBaseEntity_SetStateChanged.py \
  ida_preprocessor_scripts/find-CBaseEntity_Spawn.py \
  ida_preprocessor_scripts/find-CBaseEntity_SpawnRadius.py \
  ida_preprocessor_scripts/find-CDemoRecorder_ParseMessage.py \
  ida_preprocessor_scripts/find-CDemoRecorder_WriteSpawnGroups.py \
  ida_preprocessor_scripts/find-CEntitySystem_Activate.py \
  ida_preprocessor_scripts/find-CGameSceneNode_PostSpawnKeyValues.py \
  ida_preprocessor_scripts/find-CSceneEntity_GetScriptDesc.py \
  ida_preprocessor_scripts/find-CSteamworksGameStats_OnReceivedSessionID.py \
  ida_preprocessor_scripts/find-GetCSceneEntityScriptDesc.py \
  ida_preprocessor_scripts/find-RegisterSchemaTypeOverride_CEntityHandle.py
git commit -m "chore(ida): 迁移实体场景脚本到六元组"
```

Expected: 生成 1 个 commit，提交信息为 `chore(ida): 迁移实体场景脚本到六元组`。

### Task 3: 迁移循环、NetChan 与网络客户端脚本到 6 元组

**Files:**
- Modify: `ida_preprocessor_scripts/find-CLoopModeGame_LoopShutdown-linux.py`
- Modify: `ida_preprocessor_scripts/find-CLoopModeGame_LoopShutdown-windows.py`
- Modify: `ida_preprocessor_scripts/find-CLoopModeGame_ReceivedServerInfo.py`
- Modify: `ida_preprocessor_scripts/find-CLoopModeGame_RegisterEventMap.py`
- Modify: `ida_preprocessor_scripts/find-CLoopModeGame_SetGameSystemState.py`
- Modify: `ida_preprocessor_scripts/find-CLoopModeGame_SetWorldSession.py`
- Modify: `ida_preprocessor_scripts/find-CLoopModeGame_Shutdown-linux.py`
- Modify: `ida_preprocessor_scripts/find-CLoopTypeClientServerService_OnLoopActivate.py`
- Modify: `ida_preprocessor_scripts/find-CLoopTypeClientServer_BuildAndActivateLoopTypes.py`
- Modify: `ida_preprocessor_scripts/find-CNetChan_ParseMessagesDemo.py`
- Modify: `ida_preprocessor_scripts/find-CNetChan_ParseMessagesDemoInternal.py`
- Modify: `ida_preprocessor_scripts/find-CNetChan_ProcessMessages.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntities-linux.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntities-windows.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntitiesInternal-windows.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkGameClient_RecordEntityBandwidth.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkGameClient_SendMove.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkGameClient_SendMovePacket.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkServerService_Init.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkSystem_SendNetworkStats.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkTransmitComponent_StateChanged.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkUtlVectorEmbedded_NetworkStateChanged_m_vecRenderAttributes.py`

- [ ] **Step 1: 先让本组 AST 校验失败**

Run:

```bash
uv run python - <<'PY'
import ast
from pathlib import Path

FILES = [
    "ida_preprocessor_scripts/find-CLoopModeGame_LoopShutdown-linux.py",
    "ida_preprocessor_scripts/find-CLoopModeGame_LoopShutdown-windows.py",
    "ida_preprocessor_scripts/find-CLoopModeGame_ReceivedServerInfo.py",
    "ida_preprocessor_scripts/find-CLoopModeGame_RegisterEventMap.py",
    "ida_preprocessor_scripts/find-CLoopModeGame_SetGameSystemState.py",
    "ida_preprocessor_scripts/find-CLoopModeGame_SetWorldSession.py",
    "ida_preprocessor_scripts/find-CLoopModeGame_Shutdown-linux.py",
    "ida_preprocessor_scripts/find-CLoopTypeClientServerService_OnLoopActivate.py",
    "ida_preprocessor_scripts/find-CLoopTypeClientServer_BuildAndActivateLoopTypes.py",
    "ida_preprocessor_scripts/find-CNetChan_ParseMessagesDemo.py",
    "ida_preprocessor_scripts/find-CNetChan_ParseMessagesDemoInternal.py",
    "ida_preprocessor_scripts/find-CNetChan_ProcessMessages.py",
    "ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntities-linux.py",
    "ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntities-windows.py",
    "ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntitiesInternal-windows.py",
    "ida_preprocessor_scripts/find-CNetworkGameClient_RecordEntityBandwidth.py",
    "ida_preprocessor_scripts/find-CNetworkGameClient_SendMove.py",
    "ida_preprocessor_scripts/find-CNetworkGameClient_SendMovePacket.py",
    "ida_preprocessor_scripts/find-CNetworkServerService_Init.py",
    "ida_preprocessor_scripts/find-CNetworkSystem_SendNetworkStats.py",
    "ida_preprocessor_scripts/find-CNetworkTransmitComponent_StateChanged.py",
    "ida_preprocessor_scripts/find-CNetworkUtlVectorEmbedded_NetworkStateChanged_m_vecRenderAttributes.py",
]

errors = []
for path_str in FILES:
    tree = ast.parse(Path(path_str).read_text(encoding="utf-8-sig"), filename=path_str)
    for node in ast.walk(tree):
        if not isinstance(node, ast.Assign):
            continue
        for target in node.targets:
            if isinstance(target, ast.Name) and target.id.startswith("FUNC_XREFS"):
                assert isinstance(node.value, ast.List), path_str
                for index, entry in enumerate(node.value.elts):
                    assert isinstance(entry, ast.Tuple), path_str
                    if len(entry.elts) != 6:
                        errors.append(f"{path_str}[{index}] len={len(entry.elts)}")

assert errors, "expected at least one legacy func_xrefs tuple before migration"
raise SystemExit("\n".join(errors[:30]))
PY
```

Expected: FAIL，并打印若干 `len=5` 的旧 schema 命中。

- [ ] **Step 2: 按统一规则把本组所有 `FUNC_XREFS*` 补成 6 元组**

对本组所有脚本应用下面的标准形状，默认第 6 项填空列表；下面使用 `find-CNetworkGameClient_SendMove.py` 的真实条目作为模板：

```python
FUNC_XREFS = [
    # (func_name, xref_strings_list, xref_signatures_list, xref_funcs_list, exclude_funcs_list, exclude_strings_list)
    (
        "CNetworkGameClient_SendMove",
        [
            "CL:  CNetworkGameClient::SendMove Transmit Suppressed waiting for levelload",
        ],
        [],
        [],
        [],
        [],
    ),
]
```

对平台专用脚本也应用同一 `FUNC_XREFS` 形状；下面使用 `find-CLoopModeGame_LoopShutdown-windows.py` 的真实条目作为模板：

```python
FUNC_XREFS = [
    # (func_name, xref_strings_list, xref_signatures_list, xref_funcs_list, exclude_funcs_list, exclude_strings_list)
    (
        "CLoopModeGame_LoopShutdown",
        ["--CLoopModeGame::SetWorldSession"],
        [],
        ["CLoopModeGame_SetGameSystemState", "IGameSystem_DestroyAllGameSystems"],
        ["CLoopModeGame_ReceivedServerInfo", "CLoopModeGame_SetWorldSession"],
        [],
    ),
]
```

- [ ] **Step 3: 重新运行本组校验并编译这些脚本**

Run:

```bash
uv run python - <<'PY'
import ast
from pathlib import Path

FILES = [
    "ida_preprocessor_scripts/find-CLoopModeGame_LoopShutdown-linux.py",
    "ida_preprocessor_scripts/find-CLoopModeGame_LoopShutdown-windows.py",
    "ida_preprocessor_scripts/find-CLoopModeGame_ReceivedServerInfo.py",
    "ida_preprocessor_scripts/find-CLoopModeGame_RegisterEventMap.py",
    "ida_preprocessor_scripts/find-CLoopModeGame_SetGameSystemState.py",
    "ida_preprocessor_scripts/find-CLoopModeGame_SetWorldSession.py",
    "ida_preprocessor_scripts/find-CLoopModeGame_Shutdown-linux.py",
    "ida_preprocessor_scripts/find-CLoopTypeClientServerService_OnLoopActivate.py",
    "ida_preprocessor_scripts/find-CLoopTypeClientServer_BuildAndActivateLoopTypes.py",
    "ida_preprocessor_scripts/find-CNetChan_ParseMessagesDemo.py",
    "ida_preprocessor_scripts/find-CNetChan_ParseMessagesDemoInternal.py",
    "ida_preprocessor_scripts/find-CNetChan_ProcessMessages.py",
    "ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntities-linux.py",
    "ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntities-windows.py",
    "ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntitiesInternal-windows.py",
    "ida_preprocessor_scripts/find-CNetworkGameClient_RecordEntityBandwidth.py",
    "ida_preprocessor_scripts/find-CNetworkGameClient_SendMove.py",
    "ida_preprocessor_scripts/find-CNetworkGameClient_SendMovePacket.py",
    "ida_preprocessor_scripts/find-CNetworkServerService_Init.py",
    "ida_preprocessor_scripts/find-CNetworkSystem_SendNetworkStats.py",
    "ida_preprocessor_scripts/find-CNetworkTransmitComponent_StateChanged.py",
    "ida_preprocessor_scripts/find-CNetworkUtlVectorEmbedded_NetworkStateChanged_m_vecRenderAttributes.py",
]

errors = []
for path_str in FILES:
    tree = ast.parse(Path(path_str).read_text(encoding="utf-8-sig"), filename=path_str)
    for node in ast.walk(tree):
        if not isinstance(node, ast.Assign):
            continue
        for target in node.targets:
            if isinstance(target, ast.Name) and target.id.startswith("FUNC_XREFS"):
                assert isinstance(node.value, ast.List), path_str
                for index, entry in enumerate(node.value.elts):
                    assert isinstance(entry, ast.Tuple), path_str
                    if len(entry.elts) != 6:
                        errors.append(f"{path_str}[{index}] len={len(entry.elts)}")

assert not errors, "\n".join(errors)
print("group ok")
PY

python -m py_compile \
  ida_preprocessor_scripts/find-CLoopModeGame_LoopShutdown-linux.py \
  ida_preprocessor_scripts/find-CLoopModeGame_LoopShutdown-windows.py \
  ida_preprocessor_scripts/find-CLoopModeGame_ReceivedServerInfo.py \
  ida_preprocessor_scripts/find-CLoopModeGame_RegisterEventMap.py \
  ida_preprocessor_scripts/find-CLoopModeGame_SetGameSystemState.py \
  ida_preprocessor_scripts/find-CLoopModeGame_SetWorldSession.py \
  ida_preprocessor_scripts/find-CLoopModeGame_Shutdown-linux.py \
  ida_preprocessor_scripts/find-CLoopTypeClientServerService_OnLoopActivate.py \
  ida_preprocessor_scripts/find-CLoopTypeClientServer_BuildAndActivateLoopTypes.py \
  ida_preprocessor_scripts/find-CNetChan_ParseMessagesDemo.py \
  ida_preprocessor_scripts/find-CNetChan_ParseMessagesDemoInternal.py \
  ida_preprocessor_scripts/find-CNetChan_ProcessMessages.py \
  ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntities-linux.py \
  ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntities-windows.py \
  ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntitiesInternal-windows.py \
  ida_preprocessor_scripts/find-CNetworkGameClient_RecordEntityBandwidth.py \
  ida_preprocessor_scripts/find-CNetworkGameClient_SendMove.py \
  ida_preprocessor_scripts/find-CNetworkGameClient_SendMovePacket.py \
  ida_preprocessor_scripts/find-CNetworkServerService_Init.py \
  ida_preprocessor_scripts/find-CNetworkSystem_SendNetworkStats.py \
  ida_preprocessor_scripts/find-CNetworkTransmitComponent_StateChanged.py \
  ida_preprocessor_scripts/find-CNetworkUtlVectorEmbedded_NetworkStateChanged_m_vecRenderAttributes.py
```

Expected:
- 第一段输出 `group ok`
- 第二段无输出且退出码为 `0`

- [ ] **Step 4: 提交本组脚本迁移**

Run:

```bash
git add \
  ida_preprocessor_scripts/find-CLoopModeGame_LoopShutdown-linux.py \
  ida_preprocessor_scripts/find-CLoopModeGame_LoopShutdown-windows.py \
  ida_preprocessor_scripts/find-CLoopModeGame_ReceivedServerInfo.py \
  ida_preprocessor_scripts/find-CLoopModeGame_RegisterEventMap.py \
  ida_preprocessor_scripts/find-CLoopModeGame_SetGameSystemState.py \
  ida_preprocessor_scripts/find-CLoopModeGame_SetWorldSession.py \
  ida_preprocessor_scripts/find-CLoopModeGame_Shutdown-linux.py \
  ida_preprocessor_scripts/find-CLoopTypeClientServerService_OnLoopActivate.py \
  ida_preprocessor_scripts/find-CLoopTypeClientServer_BuildAndActivateLoopTypes.py \
  ida_preprocessor_scripts/find-CNetChan_ParseMessagesDemo.py \
  ida_preprocessor_scripts/find-CNetChan_ParseMessagesDemoInternal.py \
  ida_preprocessor_scripts/find-CNetChan_ProcessMessages.py \
  ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntities-linux.py \
  ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntities-windows.py \
  ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntitiesInternal-windows.py \
  ida_preprocessor_scripts/find-CNetworkGameClient_RecordEntityBandwidth.py \
  ida_preprocessor_scripts/find-CNetworkGameClient_SendMove.py \
  ida_preprocessor_scripts/find-CNetworkGameClient_SendMovePacket.py \
  ida_preprocessor_scripts/find-CNetworkServerService_Init.py \
  ida_preprocessor_scripts/find-CNetworkSystem_SendNetworkStats.py \
  ida_preprocessor_scripts/find-CNetworkTransmitComponent_StateChanged.py \
  ida_preprocessor_scripts/find-CNetworkUtlVectorEmbedded_NetworkStateChanged_m_vecRenderAttributes.py
git commit -m "chore(ida): 迁移循环网络脚本到六元组"
```

Expected: 生成 1 个 commit，提交信息为 `chore(ida): 迁移循环网络脚本到六元组`。

### Task 4: 迁移 `CNetworkMessages_*` 脚本到 6 元组

**Files:**
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_AllocateAndCopyConstructNetMessageAbstract.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_AssociateNetMessageGroupIdWithChannelCategory.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_AssociateNetMessageWithChannelCategoryAbstract.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_ComputeOrderForPriority.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_ConfirmAllMessageHandlersInstalled.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_DeallocateNetMessageAbstract.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_FindNetworkMessage.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_FindOrCreateNetMessage.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_RegisterFieldChangeCallbackPriority.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_RegisterNetworkArrayFieldSerializer.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_RegisterNetworkCategory.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_RegisterNetworkFieldChangeCallbackInternal.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_RegisterNetworkFieldSerializer.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_SerializeInternal.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_SerializeMessageInternal.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_UnserializeFromStream.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_UnserializeMessageInternal.py`

- [ ] **Step 1: 先让消息组 AST 校验失败**

Run:

```bash
uv run python - <<'PY'
import ast
from pathlib import Path

FILES = [
    "ida_preprocessor_scripts/find-CNetworkMessages_AllocateAndCopyConstructNetMessageAbstract.py",
    "ida_preprocessor_scripts/find-CNetworkMessages_AssociateNetMessageGroupIdWithChannelCategory.py",
    "ida_preprocessor_scripts/find-CNetworkMessages_AssociateNetMessageWithChannelCategoryAbstract.py",
    "ida_preprocessor_scripts/find-CNetworkMessages_ComputeOrderForPriority.py",
    "ida_preprocessor_scripts/find-CNetworkMessages_ConfirmAllMessageHandlersInstalled.py",
    "ida_preprocessor_scripts/find-CNetworkMessages_DeallocateNetMessageAbstract.py",
    "ida_preprocessor_scripts/find-CNetworkMessages_FindNetworkMessage.py",
    "ida_preprocessor_scripts/find-CNetworkMessages_FindOrCreateNetMessage.py",
    "ida_preprocessor_scripts/find-CNetworkMessages_RegisterFieldChangeCallbackPriority.py",
    "ida_preprocessor_scripts/find-CNetworkMessages_RegisterNetworkArrayFieldSerializer.py",
    "ida_preprocessor_scripts/find-CNetworkMessages_RegisterNetworkCategory.py",
    "ida_preprocessor_scripts/find-CNetworkMessages_RegisterNetworkFieldChangeCallbackInternal.py",
    "ida_preprocessor_scripts/find-CNetworkMessages_RegisterNetworkFieldSerializer.py",
    "ida_preprocessor_scripts/find-CNetworkMessages_SerializeInternal.py",
    "ida_preprocessor_scripts/find-CNetworkMessages_SerializeMessageInternal.py",
    "ida_preprocessor_scripts/find-CNetworkMessages_UnserializeFromStream.py",
    "ida_preprocessor_scripts/find-CNetworkMessages_UnserializeMessageInternal.py",
]

errors = []
for path_str in FILES:
    tree = ast.parse(Path(path_str).read_text(encoding="utf-8-sig"), filename=path_str)
    for node in ast.walk(tree):
        if not isinstance(node, ast.Assign):
            continue
        for target in node.targets:
            if isinstance(target, ast.Name) and target.id.startswith("FUNC_XREFS"):
                assert isinstance(node.value, ast.List), path_str
                for index, entry in enumerate(node.value.elts):
                    assert isinstance(entry, ast.Tuple), path_str
                    if len(entry.elts) != 6:
                        errors.append(f"{path_str}[{index}] len={len(entry.elts)}")

assert errors, "expected at least one legacy func_xrefs tuple before migration"
raise SystemExit("\n".join(errors[:30]))
PY
```

Expected: FAIL，并打印若干 `len=5` 的旧 schema 命中。

- [ ] **Step 2: 把消息组所有 `FUNC_XREFS*` 统一补成 6 元组**

对普通消息脚本应用下面的标准形状；下面使用 `find-CNetworkMessages_ComputeOrderForPriority.py` 的真实条目作为模板：

```python
FUNC_XREFS = [
    # (func_name, xref_strings_list, xref_signatures_list, xref_funcs_list, exclude_funcs_list, exclude_strings_list)
    (
        "CNetworkMessages_ComputeOrderForPriority",
        [
            "Network field tried to use a priority that has not been registered!",
        ],
        [],
        [],
        [],
        [],
    ),
]
```

对 `find-CNetworkMessages_FindNetworkMessage.py` 这种同时声明 `FUNC_XREFS_WINDOWS` / `FUNC_XREFS_LINUX` 的脚本，Windows 与 Linux 两侧都补第 6 项空列表：

```python
FUNC_XREFS_WINDOWS = [
    # (func_name, xref_strings_list, xref_signatures_list, xref_funcs_list, exclude_funcs_list, exclude_strings_list)
    (
        "CNetworkMessages_FindNetworkMessage",
        [
            "unknown",
        ],
        [
            "41 ?? FF FF 00 00",
            "41 ?? FF 7F 00 00"
        ],
        [],
        ["CNetworkMessages_FindNetworkMessagePartial", "CNetworkMessages_ConfirmAllMessageHandlersInstalled"],
        [],
    ),
]

FUNC_XREFS_LINUX = [
    # (func_name, xref_strings_list, xref_signatures_list, xref_funcs_list, exclude_funcs_list, exclude_strings_list)
    (
        "CNetworkMessages_FindNetworkMessage",
        [
            "unknown",
        ],
        [
            "81 FB FF FF 00 00",
            "66 81 E2 FF 7F"
        ],
        [],
        ["CNetworkMessages_FindNetworkMessagePartial", "CNetworkMessages_ConfirmAllMessageHandlersInstalled"],
        [],
    ),
]
```

- [ ] **Step 3: 重新运行消息组校验并编译这些脚本**

Run:

```bash
uv run python - <<'PY'
import ast
from pathlib import Path

FILES = [
    "ida_preprocessor_scripts/find-CNetworkMessages_AllocateAndCopyConstructNetMessageAbstract.py",
    "ida_preprocessor_scripts/find-CNetworkMessages_AssociateNetMessageGroupIdWithChannelCategory.py",
    "ida_preprocessor_scripts/find-CNetworkMessages_AssociateNetMessageWithChannelCategoryAbstract.py",
    "ida_preprocessor_scripts/find-CNetworkMessages_ComputeOrderForPriority.py",
    "ida_preprocessor_scripts/find-CNetworkMessages_ConfirmAllMessageHandlersInstalled.py",
    "ida_preprocessor_scripts/find-CNetworkMessages_DeallocateNetMessageAbstract.py",
    "ida_preprocessor_scripts/find-CNetworkMessages_FindNetworkMessage.py",
    "ida_preprocessor_scripts/find-CNetworkMessages_FindOrCreateNetMessage.py",
    "ida_preprocessor_scripts/find-CNetworkMessages_RegisterFieldChangeCallbackPriority.py",
    "ida_preprocessor_scripts/find-CNetworkMessages_RegisterNetworkArrayFieldSerializer.py",
    "ida_preprocessor_scripts/find-CNetworkMessages_RegisterNetworkCategory.py",
    "ida_preprocessor_scripts/find-CNetworkMessages_RegisterNetworkFieldChangeCallbackInternal.py",
    "ida_preprocessor_scripts/find-CNetworkMessages_RegisterNetworkFieldSerializer.py",
    "ida_preprocessor_scripts/find-CNetworkMessages_SerializeInternal.py",
    "ida_preprocessor_scripts/find-CNetworkMessages_SerializeMessageInternal.py",
    "ida_preprocessor_scripts/find-CNetworkMessages_UnserializeFromStream.py",
    "ida_preprocessor_scripts/find-CNetworkMessages_UnserializeMessageInternal.py",
]

errors = []
for path_str in FILES:
    tree = ast.parse(Path(path_str).read_text(encoding="utf-8-sig"), filename=path_str)
    for node in ast.walk(tree):
        if not isinstance(node, ast.Assign):
            continue
        for target in node.targets:
            if isinstance(target, ast.Name) and target.id.startswith("FUNC_XREFS"):
                assert isinstance(node.value, ast.List), path_str
                for index, entry in enumerate(node.value.elts):
                    assert isinstance(entry, ast.Tuple), path_str
                    if len(entry.elts) != 6:
                        errors.append(f"{path_str}[{index}] len={len(entry.elts)}")

assert not errors, "\n".join(errors)
print("group ok")
PY

python -m py_compile \
  ida_preprocessor_scripts/find-CNetworkMessages_AllocateAndCopyConstructNetMessageAbstract.py \
  ida_preprocessor_scripts/find-CNetworkMessages_AssociateNetMessageGroupIdWithChannelCategory.py \
  ida_preprocessor_scripts/find-CNetworkMessages_AssociateNetMessageWithChannelCategoryAbstract.py \
  ida_preprocessor_scripts/find-CNetworkMessages_ComputeOrderForPriority.py \
  ida_preprocessor_scripts/find-CNetworkMessages_ConfirmAllMessageHandlersInstalled.py \
  ida_preprocessor_scripts/find-CNetworkMessages_DeallocateNetMessageAbstract.py \
  ida_preprocessor_scripts/find-CNetworkMessages_FindNetworkMessage.py \
  ida_preprocessor_scripts/find-CNetworkMessages_FindOrCreateNetMessage.py \
  ida_preprocessor_scripts/find-CNetworkMessages_RegisterFieldChangeCallbackPriority.py \
  ida_preprocessor_scripts/find-CNetworkMessages_RegisterNetworkArrayFieldSerializer.py \
  ida_preprocessor_scripts/find-CNetworkMessages_RegisterNetworkCategory.py \
  ida_preprocessor_scripts/find-CNetworkMessages_RegisterNetworkFieldChangeCallbackInternal.py \
  ida_preprocessor_scripts/find-CNetworkMessages_RegisterNetworkFieldSerializer.py \
  ida_preprocessor_scripts/find-CNetworkMessages_SerializeInternal.py \
  ida_preprocessor_scripts/find-CNetworkMessages_SerializeMessageInternal.py \
  ida_preprocessor_scripts/find-CNetworkMessages_UnserializeFromStream.py \
  ida_preprocessor_scripts/find-CNetworkMessages_UnserializeMessageInternal.py
```

Expected:
- 第一段输出 `group ok`
- 第二段无输出且退出码为 `0`

- [ ] **Step 4: 提交消息组脚本迁移**

Run:

```bash
git add \
  ida_preprocessor_scripts/find-CNetworkMessages_AllocateAndCopyConstructNetMessageAbstract.py \
  ida_preprocessor_scripts/find-CNetworkMessages_AssociateNetMessageGroupIdWithChannelCategory.py \
  ida_preprocessor_scripts/find-CNetworkMessages_AssociateNetMessageWithChannelCategoryAbstract.py \
  ida_preprocessor_scripts/find-CNetworkMessages_ComputeOrderForPriority.py \
  ida_preprocessor_scripts/find-CNetworkMessages_ConfirmAllMessageHandlersInstalled.py \
  ida_preprocessor_scripts/find-CNetworkMessages_DeallocateNetMessageAbstract.py \
  ida_preprocessor_scripts/find-CNetworkMessages_FindNetworkMessage.py \
  ida_preprocessor_scripts/find-CNetworkMessages_FindOrCreateNetMessage.py \
  ida_preprocessor_scripts/find-CNetworkMessages_RegisterFieldChangeCallbackPriority.py \
  ida_preprocessor_scripts/find-CNetworkMessages_RegisterNetworkArrayFieldSerializer.py \
  ida_preprocessor_scripts/find-CNetworkMessages_RegisterNetworkCategory.py \
  ida_preprocessor_scripts/find-CNetworkMessages_RegisterNetworkFieldChangeCallbackInternal.py \
  ida_preprocessor_scripts/find-CNetworkMessages_RegisterNetworkFieldSerializer.py \
  ida_preprocessor_scripts/find-CNetworkMessages_SerializeInternal.py \
  ida_preprocessor_scripts/find-CNetworkMessages_SerializeMessageInternal.py \
  ida_preprocessor_scripts/find-CNetworkMessages_UnserializeFromStream.py \
  ida_preprocessor_scripts/find-CNetworkMessages_UnserializeMessageInternal.py
git commit -m "chore(ida): 迁移消息脚本到六元组"
```

Expected: 生成 1 个 commit，提交信息为 `chore(ida): 迁移消息脚本到六元组`。

### Task 5: 做全仓回归验证并同步 Serena memory

**Files:**
- Verify: `ida_analyze_util.py`
- Verify: `ida_preprocessor_scripts/*.py`
- Memory: `preprocess_common_skill_func_xrefs`

- [ ] **Step 1: 做全仓 `FUNC_XREFS*` 六元组静态校验**

Run:

```bash
uv run python - <<'PY'
import ast
from pathlib import Path

files = sorted(Path("ida_preprocessor_scripts").glob("find-*.py"))
errors = []
checked = 0

for path in files:
    tree = ast.parse(path.read_text(encoding="utf-8-sig"), filename=str(path))
    for node in ast.walk(tree):
        if not isinstance(node, ast.Assign):
            continue
        for target in node.targets:
            if isinstance(target, ast.Name) and target.id.startswith("FUNC_XREFS"):
                checked += 1
                assert isinstance(node.value, ast.List), str(path)
                for index, entry in enumerate(node.value.elts):
                    assert isinstance(entry, ast.Tuple), str(path)
                    if len(entry.elts) != 6:
                        errors.append(f"{path}[{index}] len={len(entry.elts)}")

assert checked > 0, "no FUNC_XREFS assignments found"
assert not errors, "\n".join(errors)
print(f"validated {checked} FUNC_XREFS assignments")
PY
```

Expected: 输出 `validated N FUNC_XREFS assignments`，且退出码为 `0`。

- [ ] **Step 2: 编译核心文件与全部受影响脚本**

Run:

```bash
python -m py_compile ida_analyze_util.py $(rg -l "^FUNC_XREFS(_WINDOWS|_LINUX)?\\s*=" ida_preprocessor_scripts -g '*.py' | sort)
```

Expected: 无输出且退出码为 `0`。

- [ ] **Step 3: 更新 Serena memory `preprocess_common_skill_func_xrefs`**

把 memory 更新为下面的内容要点：

```md
# preprocess_common_skill func_xrefs

## Summary
- `preprocess_common_skill` 只接受统一的 `func_xrefs`
- `func_xrefs` 现固定为六元组：
  `(func_name, xref_strings_list, xref_signatures_list, xref_funcs_list, exclude_funcs_list, exclude_strings_list)`
- `exclude_strings_list` 与 `xref_strings_list` 一样使用子串匹配

## Contract
- `xref_strings_list`、`xref_signatures_list`、`xref_funcs_list` 不能同时为空
- `exclude_funcs_list` 与 `exclude_strings_list` 可为空
- 旧 5 元组不再支持，命中后应直接视为非法配置

## Operational notes
- `exclude_funcs_list` 在正向交集后按 `func_va` 做差集
- `exclude_strings_list` 在正向交集后，按字符串 xref 所属函数集合并集做差集
- `exclude_strings_list` 若某个字符串没有命中任何函数，不视为失败，只视为空排除集
- `find-CBaseEntity_SetStateChanged.py` 的 Linux 路径使用
  `CNetworkTransmitComponent::StateChanged(%s) @%s:%d`
  作为内联 vcall 排除字符串
```

完成方式：
- 若 memory 内容较短且结构变化大，直接全量覆盖
- 若只需替换旧 5 元组描述，使用精确编辑替换对应段落

- [ ] **Step 4: 提交验证与记忆同步**

Run:

```bash
git add ida_analyze_util.py ida_preprocessor_scripts
git commit -m "docs(memory): 更新func_xrefs流程记忆"
```

Expected: 生成 1 个 commit，提交信息为 `docs(memory): 更新func_xrefs流程记忆`。

## Self-Review Notes

- 覆盖性：Task 1 覆盖统一契约、解析、透传和负向过滤语义；Task 2 到 Task 4 覆盖全部 `FUNC_XREFS*` 脚本迁移；Task 5 覆盖全仓静态验证与 Serena memory 更新。
- 占位符：计划内避免未定事项、延期实现和省略性执行描述。
- 一致性：全计划统一使用 6 元组 schema 与 `exclude_strings` 命名；所有回归命令均围绕同一契约验证，不再混用旧 5 元组描述。
