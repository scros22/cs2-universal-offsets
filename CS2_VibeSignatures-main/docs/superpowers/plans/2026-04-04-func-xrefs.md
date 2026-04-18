# Func Xrefs Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将 `preprocess_common_skill` 的字符串交叉引用回退能力升级为统一的 `func_xrefs`，支持字符串与函数交叉引用联合求交，完成所有旧脚本迁移，并把 `CNetworkGameClient_ProcessPacketEntities` 以依赖当前版本 YAML 的方式接入配置与预处理流程。

**Architecture:** 先在 `ida_analyze_util.py` 中拆出可测试的小型 xref 收集辅助函数，再实现统一的 `preprocess_func_xrefs_via_mcp()`，让它只从 `new_binary_dir` 读取当前版本依赖函数 YAML 的 `func_va`。随后把 `preprocess_common_skill()` 的入口、文档和回退逻辑整体切到 `func_xrefs`，并批量迁移 `ida_preprocessor_scripts/` 中的旧调用；最后在 `config.yaml` 中新增 `find-CNetworkGameClient_ProcessPacketEntities` 并调整符号别名，同时补写 Serena memory 和记号级静态验证。

**Tech Stack:** Python 3、PyYAML、IDA MCP `py_eval`、`rg`、Serena memory tools

---

## File Structure

- Modify: `ida_analyze_util.py`
  - 新增小型 YAML/地址解析与 xref 收集辅助函数
  - 将 `preprocess_func_xref_strings_via_mcp()` 升级为 `preprocess_func_xrefs_via_mcp()`
  - 将 `preprocess_common_skill()` 参数、文档与回退逻辑统一迁移到 `func_xrefs`
- Modify: `config.yaml`
  - 在 `networksystem` 模块中新增 `find-CNetworkGameClient_ProcessPacketEntities`
  - 通过 `expected_input` 强化当前版本 YAML 依赖顺序
  - 新增 `CNetworkGameClient_ProcessPacketEntities` symbol
  - 从 `CNetworkGameClient_ProcessPacketEntitiesInternal` 的 alias 中移除错误的 `ProcessPacketEntities`
- Create: `ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntities.py`
  - 使用 `func_xrefs` 的“字符串 + 函数”联合约束来定位外层函数
- Modify: `ida_preprocessor_scripts/find-CDemoRecorder_ParseMessage.py`
- Modify: `ida_preprocessor_scripts/find-CNetChan_ParseMessagesDemo.py`
- Modify: `ida_preprocessor_scripts/find-CNetChan_ProcessMessages.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntitiesInternal.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkGameClient_SendMove.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkGameClient_SendMovePacket.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_AllocateAndCopyConstructNetMessageAbstract.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_AssociateNetMessageGroupIdWithChannelCategory.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_AssociateNetMessageWithChannelCategoryAbstract.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_ComputeOrderForPriority.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_DeallocateNetMessageAbstract.py`
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
  - 记录 `func_xrefs` 三元组契约、当前版本 YAML 依赖规则和迁移结论

说明：仓库当前没有现成的 Python 单元测试目录，本计划使用内联 `uv run python - <<'PY'` 探针和静态 `rg`/`py_compile` 作为最小 TDD 与回归验证，不额外引入新的测试框架。

### Task 1: 在 `ida_analyze_util.py` 建立可测试的 `func_xrefs` 核心辅助函数

**Files:**
- Modify: `ida_analyze_util.py`
- Verify: 本任务 Step 1 / Step 3 的 `uv run python - <<'PY'`

- [ ] **Step 1: 先写失败的 API 探针**

Run:

```bash
uv run python - <<'PY'
import ida_analyze_util as util

assert hasattr(util, "_read_yaml_file"), "missing _read_yaml_file"
assert hasattr(util, "_collect_xref_func_starts_for_string"), "missing _collect_xref_func_starts_for_string"
assert hasattr(util, "_collect_xref_func_starts_for_ea"), "missing _collect_xref_func_starts_for_ea"
assert hasattr(util, "preprocess_func_xrefs_via_mcp"), "missing preprocess_func_xrefs_via_mcp"
PY
```

Expected: FAIL，至少报出一个 `missing ...`，因为这些辅助函数和统一 helper 还不存在。

- [ ] **Step 2: 新增小型辅助函数并实现 `preprocess_func_xrefs_via_mcp()`**

在 `ida_analyze_util.py` 中、`preprocess_common_skill()` 之前加入以下实现，保持函数短小、职责单一：

```python
def _read_yaml_file(path):
    try:
        with open(path, "r", encoding="utf-8") as f:
            return yaml.safe_load(f)
    except Exception:
        return None


def _parse_int_value(value):
    if isinstance(value, int):
        return value
    if isinstance(value, str):
        raw = value.strip()
        if not raw:
            raise ValueError("empty integer string")
        return int(raw, 0)
    return int(value)


async def _collect_xref_func_starts_for_string(session, xref_string, debug=False):
    py_code = r'''
import idautils, idaapi, json

search_str = SEARCH_STR_PLACEHOLDER
func_starts = set()

for s in idautils.Strings():
    if search_str in str(s):
        for xref in idautils.XrefsTo(s.ea, 0):
            func = idaapi.get_func(xref.frm)
            if func:
                func_starts.add(func.start_ea)

result = json.dumps([hex(ea) for ea in sorted(func_starts)])
'''.replace(
        "SEARCH_STR_PLACEHOLDER",
        json.dumps(xref_string),
    )

    eval_result = await session.call_tool(
        name="py_eval",
        arguments={"code": py_code},
    )
    eval_data = parse_mcp_result(eval_result)
    if not isinstance(eval_data, dict):
        return set()

    result_str = eval_data.get("result", "")
    try:
        return {int(addr, 16) for addr in json.loads(result_str or "[]")}
    except Exception:
        return set()


async def _collect_xref_func_starts_for_ea(session, target_ea, debug=False):
    py_code = f'''
import idautils, idaapi, json

target_ea = {target_ea}
func_starts = set()

for xref in idautils.XrefsTo(target_ea, 0):
    func = idaapi.get_func(xref.frm)
    if func:
        func_starts.add(func.start_ea)

result = json.dumps([hex(ea) for ea in sorted(func_starts)])
'''

    eval_result = await session.call_tool(
        name="py_eval",
        arguments={"code": py_code},
    )
    eval_data = parse_mcp_result(eval_result)
    if not isinstance(eval_data, dict):
        return set()

    result_str = eval_data.get("result", "")
    try:
        return {int(addr, 16) for addr in json.loads(result_str or "[]")}
    except Exception:
        return set()


async def _get_func_basic_info_via_mcp(session, func_va, image_base, debug=False):
    py_code = f'''
import idaapi, json

target_ea = {func_va}
func = idaapi.get_func(target_ea)
if func and func.start_ea == target_ea:
    result = json.dumps({{
        "func_va": hex(func.start_ea),
        "func_rva": hex(func.start_ea - {image_base}),
        "func_size": hex(func.end_ea - func.start_ea),
    }})
else:
    result = json.dumps(None)
'''

    eval_result = await session.call_tool(
        name="py_eval",
        arguments={"code": py_code},
    )
    eval_data = parse_mcp_result(eval_result)
    if not isinstance(eval_data, dict):
        return None

    result_str = eval_data.get("result", "")
    try:
        return json.loads(result_str or "null")
    except Exception:
        return None


async def preprocess_func_xrefs_via_mcp(
    session,
    func_name,
    xref_strings,
    xref_funcs,
    new_binary_dir,
    platform,
    image_base,
    debug=False,
):
    if not xref_strings and not xref_funcs:
        if debug:
            print(f"    Preprocess: no xrefs configured for {func_name}")
        return None

    candidate_sets = []

    for xref_string in xref_strings:
        func_starts = await _collect_xref_func_starts_for_string(
            session=session,
            xref_string=xref_string,
            debug=debug,
        )
        if not func_starts:
            if debug:
                print(
                    f'    Preprocess: no functions reference string "{xref_string}" for {func_name}'
                )
            return None
        candidate_sets.append(func_starts)

    for dep_func_name in xref_funcs:
        dep_yaml_path = os.path.join(
            new_binary_dir,
            f"{dep_func_name}.{platform}.yaml",
        )
        dep_yaml_data = _read_yaml_file(dep_yaml_path)
        if not isinstance(dep_yaml_data, dict):
            if debug:
                print(
                    "    Preprocess: failed to read dependency func YAML: "
                    f"{os.path.basename(dep_yaml_path)} for {func_name}"
                )
            return None

        dep_func_va_raw = dep_yaml_data.get("func_va")
        if dep_func_va_raw is None:
            if debug:
                print(
                    "    Preprocess: missing func_va in dependency YAML: "
                    f"{os.path.basename(dep_yaml_path)} for {func_name}"
                )
            return None

        try:
            dep_func_va = _parse_int_value(dep_func_va_raw)
        except Exception:
            if debug:
                print(
                    "    Preprocess: invalid func_va in dependency YAML: "
                    f"{os.path.basename(dep_yaml_path)} for {func_name}"
                )
            return None

        func_starts = await _collect_xref_func_starts_for_ea(
            session=session,
            target_ea=dep_func_va,
            debug=debug,
        )
        if not func_starts:
            if debug:
                print(
                    f"    Preprocess: no functions reference dependency {dep_func_name} for {func_name}"
                )
            return None
        candidate_sets.append(func_starts)

    common_funcs = set(candidate_sets[0])
    for candidate_set in candidate_sets[1:]:
        common_funcs &= candidate_set

    if len(common_funcs) != 1:
        if debug:
            print(
                f"    Preprocess: xref intersection yielded {len(common_funcs)} function(s) for {func_name} (need exactly 1)"
            )
        return None

    target_va = next(iter(common_funcs))
    sig_data = await preprocess_gen_func_sig_via_mcp(
        session=session,
        func_va=target_va,
        image_base=image_base,
        debug=debug,
    )
    if sig_data is None:
        basic_info = await _get_func_basic_info_via_mcp(
            session=session,
            func_va=target_va,
            image_base=image_base,
            debug=debug,
        )
        if basic_info is None:
            return None
        basic_info["func_name"] = func_name
        return basic_info

    sig_data["func_name"] = func_name
    return sig_data
```

- [ ] **Step 3: 用 monkeypatch 探针验证联合求交和当前版本 YAML 读取**

Run:

```bash
uv run python - <<'PY'
import asyncio
import tempfile
from pathlib import Path

import ida_analyze_util as util

assert hasattr(util, "preprocess_func_xrefs_via_mcp")

orig_read = util._read_yaml_file
orig_str = util._collect_xref_func_starts_for_string
orig_ea = util._collect_xref_func_starts_for_ea
orig_sig = util.preprocess_gen_func_sig_via_mcp

def fake_read_yaml(path):
    return orig_read(path)

async def fake_collect_string(session, xref_string, debug=False):
    assert xref_string == "needle"
    return {0x2000, 0x3000}

async def fake_collect_ea(session, target_ea, debug=False):
    assert target_ea == 0x5000
    return {0x2000}

async def fake_sig(session, func_va, image_base, debug=False):
    assert func_va == 0x2000
    assert image_base == 0x1000
    return {
        "func_va": "0x2000",
        "func_rva": "0x1000",
        "func_size": "0x40",
        "func_sig": "AA BB CC",
    }

try:
    util._read_yaml_file = fake_read_yaml
    util._collect_xref_func_starts_for_string = fake_collect_string
    util._collect_xref_func_starts_for_ea = fake_collect_ea
    util.preprocess_gen_func_sig_via_mcp = fake_sig

    with tempfile.TemporaryDirectory() as tmpdir:
        dep_yaml = Path(tmpdir) / "DepFunc.windows.yaml"
        dep_yaml.write_text("func_va: 0x5000\n", encoding="utf-8")

        result = asyncio.run(
            util.preprocess_func_xrefs_via_mcp(
                session=object(),
                func_name="Target",
                xref_strings=["needle"],
                xref_funcs=["DepFunc"],
                new_binary_dir=tmpdir,
                platform="windows",
                image_base=0x1000,
                debug=False,
            )
        )

    assert result == {
        "func_va": "0x2000",
        "func_rva": "0x1000",
        "func_size": "0x40",
        "func_sig": "AA BB CC",
        "func_name": "Target",
    }
    print("ok")
finally:
    util._read_yaml_file = orig_read
    util._collect_xref_func_starts_for_string = orig_str
    util._collect_xref_func_starts_for_ea = orig_ea
    util.preprocess_gen_func_sig_via_mcp = orig_sig
PY
```

Expected: PASS，输出 `ok`

- [ ] **Step 4: 提交本任务**

```bash
git add ida_analyze_util.py
git commit -m "refactor(preprocess): 引入统一 func_xrefs"
```

### Task 2: 将 `preprocess_common_skill()` 全量迁移到 `func_xrefs`

**Files:**
- Modify: `ida_analyze_util.py`
- Verify: 本任务 Step 1 / Step 3 的 `uv run python - <<'PY'` 与 `rg`

- [ ] **Step 1: 先写失败的流水线探针**

Run:

```bash
uv run python - <<'PY'
import asyncio
import tempfile
from pathlib import Path

import ida_analyze_util as util

class DummySession:
    pass

async def run_probe():
    with tempfile.TemporaryDirectory() as tmpdir:
        out_dir = Path(tmpdir) / "yaml"
        out_dir.mkdir()
        target_output = out_dir / "Target.windows.yaml"
        return await util.preprocess_common_skill(
            session=DummySession(),
            expected_outputs=[str(target_output)],
            old_yaml_map={},
            new_binary_dir=str(out_dir),
            platform="windows",
            image_base=0x1000,
            func_xrefs=[("Target", ["needle"], ["DepFunc"])],
            debug=False,
        )

asyncio.run(run_probe())
PY
```

Expected: FAIL，当前会因为 `preprocess_common_skill()` 还不接受 `func_xrefs` 或尚未接线统一 fallback 而报错。

- [ ] **Step 2: 修改 `preprocess_common_skill()` 的签名、文档、映射和回退调用**

在 `ida_analyze_util.py` 中把 `preprocess_common_skill()` 的相关代码整体改成下面这组结构：

```python
async def preprocess_common_skill(
    session,
    expected_outputs,
    old_yaml_map=None,
    new_binary_dir=None,
    platform="windows",
    image_base=0,
    func_names=None,
    gv_names=None,
    patch_names=None,
    struct_member_names=None,
    vtable_class_names=None,
    inherit_vfuncs=None,
    func_xrefs=None,
    func_vtable_relations=None,
    debug=False,
):
```

```python
    func_xrefs = func_xrefs or []

    func_xrefs_map = {}
    for spec in func_xrefs:
        if not isinstance(spec, (tuple, list)) or len(spec) != 3:
            if debug:
                print(f"    Preprocess: invalid func_xrefs spec: {spec}")
            return False

        func_name, xref_strings, xref_funcs = spec
        if func_name in func_xrefs_map:
            if debug:
                print(f"    Preprocess: duplicated func_xrefs target: {func_name}")
            return False

        xref_strings = list(xref_strings or [])
        xref_funcs = list(xref_funcs or [])
        if not xref_strings and not xref_funcs:
            if debug:
                print(f"    Preprocess: empty func_xrefs spec for {func_name}")
            return False

        func_xrefs_map[func_name] = {
            "xref_strings": xref_strings,
            "xref_funcs": xref_funcs,
        }
```

```python
    xref_only_names = [
        name for name in func_xrefs_map if name not in func_names
    ]
    all_func_names = list(func_names) + xref_only_names
```

```python
        if func_data is None and func_name in func_xrefs_map:
            xref_spec = func_xrefs_map[func_name]
            if debug:
                print(f"    Preprocess: trying func_xrefs fallback for {func_name}")
            func_data = await preprocess_func_xrefs_via_mcp(
                session=session,
                func_name=func_name,
                xref_strings=xref_spec["xref_strings"],
                xref_funcs=xref_spec["xref_funcs"],
                new_binary_dir=new_binary_dir,
                platform=platform,
                image_base=image_base,
                debug=debug,
            )
```

同时把 docstring 中所有 `func_xref_strings` 的说明、参数名和注释替换为 `func_xrefs`，并明确三元组契约和“只从当前版本 YAML 读取依赖函数地址”。

- [ ] **Step 3: 通过 fake helper 探针验证新参数接线与 YAML 写出**

Run:

```bash
uv run python - <<'PY'
import asyncio
import tempfile
from pathlib import Path

import ida_analyze_util as util

class DummySession:
    pass

orig_func_sig = util.preprocess_func_sig_via_mcp
orig_func_xrefs = util.preprocess_func_xrefs_via_mcp

async def fake_func_sig(**kwargs):
    return None

async def fake_func_xrefs(session, func_name, xref_strings, xref_funcs, new_binary_dir, platform, image_base, debug=False):
    assert func_name == "Target"
    assert xref_strings == ["needle"]
    assert xref_funcs == ["DepFunc"]
    assert Path(new_binary_dir).name == "yaml"
    assert platform == "windows"
    assert image_base == 0x1000
    return {
        "func_name": "Target",
        "func_va": "0x2000",
        "func_rva": "0x1000",
        "func_size": "0x40",
        "func_sig": "AA BB CC",
    }

try:
    util.preprocess_func_sig_via_mcp = fake_func_sig
    util.preprocess_func_xrefs_via_mcp = fake_func_xrefs

    with tempfile.TemporaryDirectory() as tmpdir:
        out_dir = Path(tmpdir) / "yaml"
        out_dir.mkdir()
        output_path = out_dir / "Target.windows.yaml"

        ok = asyncio.run(
            util.preprocess_common_skill(
                session=DummySession(),
                expected_outputs=[str(output_path)],
                old_yaml_map={},
                new_binary_dir=str(out_dir),
                platform="windows",
                image_base=0x1000,
                func_xrefs=[("Target", ["needle"], ["DepFunc"])],
                debug=False,
            )
        )

        assert ok is True
        text = output_path.read_text(encoding="utf-8")
        assert "func_name: Target" in text
        assert "func_sig: AA BB CC" in text

    print("ok")
finally:
    util.preprocess_func_sig_via_mcp = orig_func_sig
    util.preprocess_func_xrefs_via_mcp = orig_func_xrefs
PY
```

Run:

```bash
rg -n "func_xref_strings|preprocess_func_xref_strings_via_mcp" ida_analyze_util.py -S
```

Expected: 第一条命令 PASS 并输出 `ok`；第二条命令无输出。

- [ ] **Step 4: 提交本任务**

```bash
git add ida_analyze_util.py
git commit -m "refactor(preprocess): 切换 common skill 到 func_xrefs"
```

### Task 3: 批量迁移旧脚本并接入 `CNetworkGameClient_ProcessPacketEntities`

**Files:**
- Create: `ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntities.py`
- Modify: `config.yaml`
- Modify: `ida_preprocessor_scripts/find-CDemoRecorder_ParseMessage.py`
- Modify: `ida_preprocessor_scripts/find-CNetChan_ParseMessagesDemo.py`
- Modify: `ida_preprocessor_scripts/find-CNetChan_ProcessMessages.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntitiesInternal.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkGameClient_SendMove.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkGameClient_SendMovePacket.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_AllocateAndCopyConstructNetMessageAbstract.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_AssociateNetMessageGroupIdWithChannelCategory.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_AssociateNetMessageWithChannelCategoryAbstract.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_ComputeOrderForPriority.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkMessages_DeallocateNetMessageAbstract.py`
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
- Verify: 本任务 Step 1 / Step 5 的 `rg`、`test -f`、`py_compile`

- [ ] **Step 1: 先写失败的静态检查**

Run:

```bash
rg -n "FUNC_XREF_STRINGS|func_xref_strings=" ida_preprocessor_scripts -S
```

Run:

```bash
test -f ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntities.py
```

Run:

```bash
rg -n "find-CNetworkGameClient_ProcessPacketEntities|CNetworkGameClient_ProcessPacketEntities\\.{platform}" config.yaml -S
```

Expected: 第一条命令输出旧脚本匹配；第二条命令 FAIL；第三条命令无输出。

- [ ] **Step 2: 对全部旧脚本做机械迁移，把 `FUNC_XREF_STRINGS` 统一改为 `FUNC_XREFS`**

Run:

```bash
uv run python - <<'PY'
import re
from pathlib import Path

paths = [
    Path("ida_preprocessor_scripts/find-CDemoRecorder_ParseMessage.py"),
    Path("ida_preprocessor_scripts/find-CNetChan_ParseMessagesDemo.py"),
    Path("ida_preprocessor_scripts/find-CNetChan_ProcessMessages.py"),
    Path("ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntitiesInternal.py"),
    Path("ida_preprocessor_scripts/find-CNetworkGameClient_SendMove.py"),
    Path("ida_preprocessor_scripts/find-CNetworkGameClient_SendMovePacket.py"),
    Path("ida_preprocessor_scripts/find-CNetworkMessages_AllocateAndCopyConstructNetMessageAbstract.py"),
    Path("ida_preprocessor_scripts/find-CNetworkMessages_AssociateNetMessageGroupIdWithChannelCategory.py"),
    Path("ida_preprocessor_scripts/find-CNetworkMessages_AssociateNetMessageWithChannelCategoryAbstract.py"),
    Path("ida_preprocessor_scripts/find-CNetworkMessages_ComputeOrderForPriority.py"),
    Path("ida_preprocessor_scripts/find-CNetworkMessages_DeallocateNetMessageAbstract.py"),
    Path("ida_preprocessor_scripts/find-CNetworkMessages_FindOrCreateNetMessage.py"),
    Path("ida_preprocessor_scripts/find-CNetworkMessages_RegisterFieldChangeCallbackPriority.py"),
    Path("ida_preprocessor_scripts/find-CNetworkMessages_RegisterNetworkArrayFieldSerializer.py"),
    Path("ida_preprocessor_scripts/find-CNetworkMessages_RegisterNetworkCategory.py"),
    Path("ida_preprocessor_scripts/find-CNetworkMessages_RegisterNetworkFieldChangeCallbackInternal.py"),
    Path("ida_preprocessor_scripts/find-CNetworkMessages_RegisterNetworkFieldSerializer.py"),
    Path("ida_preprocessor_scripts/find-CNetworkMessages_SerializeInternal.py"),
    Path("ida_preprocessor_scripts/find-CNetworkMessages_SerializeMessageInternal.py"),
    Path("ida_preprocessor_scripts/find-CNetworkMessages_UnserializeFromStream.py"),
    Path("ida_preprocessor_scripts/find-CNetworkMessages_UnserializeMessageInternal.py"),
]

for path in paths:
    text = path.read_text(encoding="utf-8")
    assert "FUNC_XREF_STRINGS" in text, f"missing FUNC_XREF_STRINGS in {path}"

    text = text.replace("FUNC_XREF_STRINGS", "FUNC_XREFS")
    text = text.replace(
        "# (func_name, xref_strings_list)",
        "# (func_name, xref_strings_list, xref_funcs_list)",
    )
    text = text.replace(
        "func_xref_strings=FUNC_XREFS,",
        "func_xrefs=FUNC_XREFS,",
    )

    text, replaced = re.subn(
        r"(\n\s*\]\s*)\),",
        r"\1,\n    [] ),",
        text,
        count=1,
    )
    assert replaced == 1, f"failed to append empty xref_funcs_list in {path}"

    path.write_text(text, encoding="utf-8")
PY
```

如果某个脚本因为格式差异没有命中 `re.subn(..., count=1)`，不要继续盲改；直接手工把它改成：

```python
FUNC_XREFS = [
    # (func_name, xref_strings_list, xref_funcs_list)
    (
        "Target",
        [
            "literal 1",
            "literal 2",
        ],
        [],
    ),
]
```

并把调用点统一改成：

```python
        func_xrefs=FUNC_XREFS,
```

- [ ] **Step 3: 新建 `find-CNetworkGameClient_ProcessPacketEntities.py`**

Create `ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntities.py`:

```python
#!/usr/bin/env python3
"""Preprocess script for find-CNetworkGameClient_ProcessPacketEntities skill."""

from ida_analyze_util import preprocess_common_skill

TARGET_FUNCTION_NAMES = [
    "CNetworkGameClient_ProcessPacketEntities",
]

FUNC_XREFS = [
    # (func_name, xref_strings_list, xref_funcs_list)
    (
        "CNetworkGameClient_ProcessPacketEntities",
        [
            "CNetworkGameClientBase::OnReceivedUncompressedPacket(), received full update",
        ],
        [
            "CNetworkGameClient_ProcessPacketEntitiesInternal",
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
        debug=debug,
    )
```

- [ ] **Step 4: 更新 `config.yaml`，接入新技能并修正 symbol alias**

把 `networksystem` 模块中 `find-CNetworkGameClient_ProcessPacketEntitiesInternal` 后面补上一段：

```yaml
      - name: find-CNetworkGameClient_ProcessPacketEntitiesInternal
        expected_output:
          - CNetworkGameClient_ProcessPacketEntitiesInternal.{platform}.yaml

      - name: find-CNetworkGameClient_ProcessPacketEntities
        expected_output:
          - CNetworkGameClient_ProcessPacketEntities.{platform}.yaml
        expected_input:
          - CNetworkGameClient_ProcessPacketEntitiesInternal.{platform}.yaml
```

把 symbol 区域从：

```yaml
      - name: CNetworkGameClient_ProcessPacketEntitiesInternal
        category: func
        alias:
          - CNetworkGameClient::ProcessPacketEntitiesInternal
          - ProcessPacketEntities
```

改为：

```yaml
      - name: CNetworkGameClient_ProcessPacketEntitiesInternal
        category: func
        alias:
          - CNetworkGameClient::ProcessPacketEntitiesInternal

      - name: CNetworkGameClient_ProcessPacketEntities
        category: func
        alias:
          - CNetworkGameClient::ProcessPacketEntities
          - ProcessPacketEntities
```

- [ ] **Step 5: 运行迁移后的静态验证和语法检查**

Run:

```bash
rg -n "FUNC_XREF_STRINGS|func_xref_strings=" ida_preprocessor_scripts -S
```

Run:

```bash
rg -n "find-CNetworkGameClient_ProcessPacketEntities|CNetworkGameClient_ProcessPacketEntities\\.{platform}|CNetworkGameClient::ProcessPacketEntities" config.yaml ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntities.py -S
```

Run:

```bash
uv run python -m py_compile \
  ida_analyze_util.py \
  ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntities.py \
  ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntitiesInternal.py \
  ida_preprocessor_scripts/find-CNetworkGameClient_SendMove.py \
  ida_preprocessor_scripts/find-CNetworkGameClient_SendMovePacket.py \
  ida_preprocessor_scripts/find-CNetChan_ProcessMessages.py \
  ida_preprocessor_scripts/find-CNetChan_ParseMessagesDemo.py \
  ida_preprocessor_scripts/find-CDemoRecorder_ParseMessage.py \
  ida_preprocessor_scripts/find-CNetworkMessages_AllocateAndCopyConstructNetMessageAbstract.py \
  ida_preprocessor_scripts/find-CNetworkMessages_AssociateNetMessageGroupIdWithChannelCategory.py \
  ida_preprocessor_scripts/find-CNetworkMessages_AssociateNetMessageWithChannelCategoryAbstract.py \
  ida_preprocessor_scripts/find-CNetworkMessages_ComputeOrderForPriority.py \
  ida_preprocessor_scripts/find-CNetworkMessages_DeallocateNetMessageAbstract.py \
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

Expected: 第一条命令无输出；第二条命令能命中新脚本和配置；第三条命令 PASS 且无语法错误。

- [ ] **Step 6: 提交本任务**

```bash
git add config.yaml ida_preprocessor_scripts
git commit -m "fix(networksystem): 迁移 func_xrefs 并接入包实体函数"
```

### Task 4: 更新 Serena memory 并完成最终回归验证

**Files:**
- Memory: `preprocess_common_skill_func_xrefs`
- Verify: 本任务 Step 2 / Step 3 的 Serena memory 读写与内联 Python 探针

- [ ] **Step 1: 写入 Serena memory**

Use `mcp__serena__write_memory` with:

```text
memory_name: preprocess_common_skill_func_xrefs
content:
# preprocess_common_skill func_xrefs

## Summary
- `preprocess_common_skill` no longer accepts `func_xref_strings`
- Unified fallback parameter is `func_xrefs`

## Contract
- `func_xrefs` item format: `(func_name, xref_strings_list, xref_funcs_list)`
- `xref_strings_list` and `xref_funcs_list` cannot both be empty

## Dependency resolution
- `xref_funcs_list` resolves dependency addresses only from current-version YAML in `new_binary_dir`
- YAML path pattern: `{func_name}.{platform}.yaml`
- Required field: `func_va`
- Do not read `old_yaml_map`
- Do not trust existing IDA names for dependency resolution

## Operational notes
- Skill ordering in `config.yaml` must ensure dependency YAML exists before a `func_xrefs` script runs
- `func_xrefs` supports string-only, func-only, and mixed string+func intersection
- `CNetworkGameClient_ProcessPacketEntities` now depends on `CNetworkGameClient_ProcessPacketEntitiesInternal.{platform}.yaml`
```

Expected: memory write succeeds without truncation or validation errors.

- [ ] **Step 2: 读取 memory 并确认名称与内容可见**

Use `mcp__serena__list_memories` and `mcp__serena__read_memory`:

```text
list_memories()
read_memory("preprocess_common_skill_func_xrefs")
```

Expected: `list_memories` 中出现 `preprocess_common_skill_func_xrefs`，`read_memory` 能读回 Step 1 的关键条目。

- [ ] **Step 3: 运行最终探针，验证“依赖 YAML 缺失即失败”**

Run:

```bash
uv run python - <<'PY'
import asyncio
import tempfile

import ida_analyze_util as util

orig_str = util._collect_xref_func_starts_for_string
orig_ea = util._collect_xref_func_starts_for_ea
orig_sig = util.preprocess_gen_func_sig_via_mcp

async def fake_collect_string(session, xref_string, debug=False):
    return {0x2000}

async def fake_collect_ea(session, target_ea, debug=False):
    return {0x2000}

async def fake_sig(session, func_va, image_base, debug=False):
    return {
        "func_va": "0x2000",
        "func_rva": "0x1000",
        "func_size": "0x40",
        "func_sig": "AA BB CC",
    }

try:
    util._collect_xref_func_starts_for_string = fake_collect_string
    util._collect_xref_func_starts_for_ea = fake_collect_ea
    util.preprocess_gen_func_sig_via_mcp = fake_sig

    with tempfile.TemporaryDirectory() as tmpdir:
        result = asyncio.run(
            util.preprocess_func_xrefs_via_mcp(
                session=object(),
                func_name="Target",
                xref_strings=["needle"],
                xref_funcs=["MissingDep"],
                new_binary_dir=tmpdir,
                platform="windows",
                image_base=0x1000,
                debug=False,
            )
        )

    assert result is None
    print("ok")
finally:
    util._collect_xref_func_starts_for_string = orig_str
    util._collect_xref_func_starts_for_ea = orig_ea
    util.preprocess_gen_func_sig_via_mcp = orig_sig
PY
```

Run:

```bash
rg -n "func_xref_strings|FUNC_XREF_STRINGS|preprocess_func_xref_strings_via_mcp" ida_analyze_util.py ida_preprocessor_scripts config.yaml -S
```

Expected: 第一条命令 PASS 并输出 `ok`；第二条命令无输出。
