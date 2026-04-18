# Func Xrefs Exclude Funcs Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为 `preprocess_common_skill` / `preprocess_func_xrefs_via_mcp` 增加 `exclude_funcs_list` 过滤能力，强制全仓 `func_xrefs` 使用四元组格式，并接入 `CNetworkGameClient_RecordEntityBandwidth` 常规函数预处理。

**Architecture:** 先在 `ida_analyze_util.py` 中把 `func_xrefs` 契约从三元组升级为四元组，并在 `preprocess_func_xrefs_via_mcp()` 的“交集完成后、唯一性判断前”加入排除函数差集逻辑。随后将所有现有 `FUNC_XREFS*` 脚本统一迁移到四元组格式，新增 `find-CNetworkGameClient_RecordEntityBandwidth.py`，最后在 `config.yaml` 的 `engine` 模块中补齐 skill 与 symbol 声明，并用静态检查确保依赖链闭合。

**Tech Stack:** Python 3、PyYAML、IDA MCP `py_eval`、`rg`

---

## File Structure

- Modify: `ida_analyze_util.py`
  - 将 `preprocess_func_xrefs_via_mcp()` 的参数改为接收 `exclude_funcs`
  - 将 `preprocess_common_skill()` 的 `func_xrefs` 契约改为固定四元组
  - 在交集后增加 `excluded_func_addrs` 差集过滤
- Modify: `ida_preprocessor_scripts/find-CDemoRecorder_ParseMessage.py`
- Modify: `ida_preprocessor_scripts/find-CNetChan_ParseMessagesDemo.py`
- Modify: `ida_preprocessor_scripts/find-CNetChan_ProcessMessages.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntities.py`
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
- Modify: `ida_preprocessor_scripts/find-CNetworkServerService_Init.py`
  - 将所有 `FUNC_XREFS*` 条目从三元组补成四元组，第四项默认填 `[]`
- Create: `ida_preprocessor_scripts/find-CNetworkGameClient_RecordEntityBandwidth.py`
  - 使用 `exclude_funcs_list = ["CNetworkServerService_Init"]`
- Modify: `config.yaml`
  - 在 `engine` 模块中新增 `find-CNetworkGameClient_RecordEntityBandwidth`
  - 新增 `CNetworkGameClient_RecordEntityBandwidth` symbol

**前提假设：** `CNetworkServerService_Init.{platform}.yaml` 已经在 `engine` 模块的当前版本输出目录中生成；如果它实际落在其他模块目录，先停止执行本计划并重做设计，因为当前 `func_xrefs` 只会从同一个 `new_binary_dir` 读取依赖 YAML。

### Task 1: 升级 `ida_analyze_util.py` 的 `func_xrefs` 核心契约

**Files:**
- Modify: `ida_analyze_util.py`

- [ ] **Step 1: 记录当前三元组基线**

Run:

```bash
rg -n "len\\(spec\\) != 3|\\(func_name, xref_strings_list, xref_funcs_list\\)|preprocess_func_xrefs_via_mcp\\(" ida_analyze_util.py
```

Expected: 命中 `len(spec) != 3`、旧三元组 docstring，以及当前 `preprocess_func_xrefs_via_mcp()` 还没有 `exclude_funcs` 参数。

- [ ] **Step 2: 修改 `preprocess_func_xrefs_via_mcp()` 与 `preprocess_common_skill()`**

将 `ida_analyze_util.py` 中的关键代码改成下面的结构：

```python
async def preprocess_func_xrefs_via_mcp(
    session,
    func_name,
    xref_strings,
    xref_funcs,
    exclude_funcs,
    new_binary_dir,
    platform,
    image_base,
    debug=False,
):
    dep_func_names = xref_funcs or []
    excluded_func_names = exclude_funcs or []
    if dep_func_names or excluded_func_names:
        if not new_binary_dir:
            if debug:
                print(
                    f"    Preprocess: new_binary_dir is required for "
                    f"xref_funcs/exclude_funcs of {func_name}"
                )
            return None
        try:
            new_binary_dir = os.fspath(new_binary_dir)
        except Exception:
            if debug:
                print(
                    f"    Preprocess: invalid new_binary_dir for "
                    f"xref_funcs/exclude_funcs of {func_name}"
                )
            return None

    candidate_sets = []

    for xref_string in (xref_strings or []):
        addr_set = await _collect_xref_func_starts_for_string(
            session=session, xref_string=xref_string, debug=debug
        )
        if not addr_set:
            if debug:
                short = str(xref_string)[:80]
                print(f"    Preprocess: empty candidate set for string xref: {short}")
            return None
        candidate_sets.append(addr_set)

    for dep_func_name in dep_func_names:
        dep_yaml_path = os.path.join(
            new_binary_dir, f"{dep_func_name}.{platform}.yaml"
        )
        dep_data = _read_yaml_file(dep_yaml_path)
        if not isinstance(dep_data, dict):
            if debug:
                print(
                    f"    Preprocess: dependency YAML missing or invalid: "
                    f"{os.path.basename(dep_yaml_path)}"
                )
            return None

        try:
            dep_func_va = _parse_int_value(dep_data.get("func_va"))
        except Exception:
            if debug:
                print(
                    f"    Preprocess: invalid func_va in dependency YAML: "
                    f"{os.path.basename(dep_yaml_path)}"
                )
            return None

        addr_set = await _collect_xref_func_starts_for_ea(
            session=session, target_ea=dep_func_va, debug=debug
        )
        if not addr_set:
            if debug:
                print(
                    f"    Preprocess: empty candidate set for func xref: "
                    f"{dep_func_name}"
                )
            return None
        candidate_sets.append(addr_set)

    if not candidate_sets:
        if debug:
            print(f"    Preprocess: no xref candidates configured for {func_name}")
        return None

    excluded_func_addrs = set()
    for excluded_func_name in excluded_func_names:
        excluded_yaml_path = os.path.join(
            new_binary_dir, f"{excluded_func_name}.{platform}.yaml"
        )
        excluded_data = _read_yaml_file(excluded_yaml_path)
        if not isinstance(excluded_data, dict):
            if debug:
                print(
                    f"    Preprocess: exclude dependency YAML missing or invalid: "
                    f"{os.path.basename(excluded_yaml_path)}"
                )
            return None

        try:
            excluded_func_va = _parse_int_value(excluded_data.get("func_va"))
        except Exception:
            if debug:
                print(
                    f"    Preprocess: invalid func_va in exclude dependency YAML: "
                    f"{os.path.basename(excluded_yaml_path)}"
                )
            return None

        excluded_func_addrs.add(excluded_func_va)

    common_funcs = set(candidate_sets[0])
    for addr_set in candidate_sets[1:]:
        common_funcs &= addr_set

    common_funcs -= excluded_func_addrs

    if len(common_funcs) != 1:
        if debug:
            print(
                f"    Preprocess: xref intersection yielded {len(common_funcs)} "
                f"function(s) for {func_name} (need exactly 1)"
            )
        return None
```

并把 `preprocess_common_skill()` 中的 `func_xrefs` 解析改成：

```python
    - ``func_xrefs``: locate functions via unified xref fallback through
      ``preprocess_func_xrefs_via_mcp``. Each element is a tuple of
      ``(func_name, xref_strings_list, xref_funcs_list, exclude_funcs_list)``.
```

```python
    func_xrefs_map = {}
    for spec in func_xrefs:
        if not isinstance(spec, (tuple, list)) or len(spec) != 4:
            if debug:
                print(f"    Preprocess: invalid func_xrefs spec: {spec}")
            return False

        func_name, xref_strings, xref_funcs, exclude_funcs = spec
        if not isinstance(func_name, str) or not func_name:
            if debug:
                print(f"    Preprocess: invalid func_xrefs target: {func_name}")
            return False

        if func_name in func_xrefs_map:
            if debug:
                print(f"    Preprocess: duplicated func_xrefs target: {func_name}")
            return False

        if not isinstance(xref_strings, (tuple, list)):
            if debug:
                print(
                    f"    Preprocess: invalid xref_strings type for "
                    f"{func_name}: {type(xref_strings).__name__}"
                )
            return False

        if not isinstance(xref_funcs, (tuple, list)):
            if debug:
                print(
                    f"    Preprocess: invalid xref_funcs type for "
                    f"{func_name}: {type(xref_funcs).__name__}"
                )
            return False

        if not isinstance(exclude_funcs, (tuple, list)):
            if debug:
                print(
                    f"    Preprocess: invalid exclude_funcs type for "
                    f"{func_name}: {type(exclude_funcs).__name__}"
                )
            return False

        xref_strings = list(xref_strings)
        xref_funcs = list(xref_funcs)
        exclude_funcs = list(exclude_funcs)

        if any(not isinstance(item, str) or not item for item in xref_strings):
            if debug:
                print(f"    Preprocess: invalid xref_strings values for {func_name}")
            return False

        if any(not isinstance(item, str) or not item for item in xref_funcs):
            if debug:
                print(f"    Preprocess: invalid xref_funcs values for {func_name}")
            return False

        if any(not isinstance(item, str) or not item for item in exclude_funcs):
            if debug:
                print(f"    Preprocess: invalid exclude_funcs values for {func_name}")
            return False

        if not xref_strings and not xref_funcs:
            if debug:
                print(f"    Preprocess: empty func_xrefs spec for {func_name}")
            return False

        func_xrefs_map[func_name] = {
            "xref_strings": xref_strings,
            "xref_funcs": xref_funcs,
            "exclude_funcs": exclude_funcs,
        }
```

并将调用点补齐：

```python
            func_data = await preprocess_func_xrefs_via_mcp(
                session=session,
                func_name=func_name,
                xref_strings=xref_spec["xref_strings"],
                xref_funcs=xref_spec["xref_funcs"],
                exclude_funcs=xref_spec["exclude_funcs"],
                new_binary_dir=new_binary_dir,
                platform=platform,
                image_base=image_base,
                debug=debug,
            )
```

- [ ] **Step 3: 做静态语法和契约检查**

Run:

```bash
python -m py_compile ida_analyze_util.py
rg -n "exclude_funcs|len\\(spec\\) != 4|xref_strings_list, xref_funcs_list, exclude_funcs_list" ida_analyze_util.py
```

Expected: `py_compile` 无输出且退出码为 0；`rg` 只命中新契约与 `exclude_funcs` 相关逻辑。

- [ ] **Step 4: 提交核心契约变更**

Run:

```bash
git add ida_analyze_util.py
git commit -m "feat: 增加func_xrefs排除函数能力"
```

Expected: 产生一条只包含核心逻辑变更的提交。

### Task 2: 把所有旧 `FUNC_XREFS*` 脚本迁移到四元组

**Files:**
- Modify: `ida_preprocessor_scripts/find-CDemoRecorder_ParseMessage.py`
- Modify: `ida_preprocessor_scripts/find-CNetChan_ParseMessagesDemo.py`
- Modify: `ida_preprocessor_scripts/find-CNetChan_ProcessMessages.py`
- Modify: `ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntities.py`
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
- Modify: `ida_preprocessor_scripts/find-CNetworkServerService_Init.py`

- [ ] **Step 1: 列出所有仍是三元组注释的脚本**

Run:

```bash
python - <<'PY'
from pathlib import Path

paths = sorted(Path("ida_preprocessor_scripts").glob("find-*.py"))
for path in paths:
    text = path.read_text(encoding="utf-8")
    if "# (func_name, xref_strings_list, xref_funcs_list)" in text:
        print(path.as_posix())
PY
```

Expected: 打印出本任务文件清单中的全部脚本。

- [ ] **Step 2: 将每个 `FUNC_XREFS*` 条目补成四元组**

把所有旧三元组：

```python
FUNC_XREFS = [
    # (func_name, xref_strings_list, xref_funcs_list)
    (
        "CNetworkGameClient_ProcessPacketEntitiesInternal",
        [
            "CL:  ProcessPacketEntities: frame window too big (>=%i)",
        ],
        [],
    ),
]
```

统一改成四元组：

```python
FUNC_XREFS = [
    # (func_name, xref_strings_list, xref_funcs_list, exclude_funcs_list)
    (
        "CNetworkGameClient_ProcessPacketEntitiesInternal",
        [
            "CL:  ProcessPacketEntities: frame window too big (>=%i)",
        ],
        [],
        [],
    ),
]
```

对有平台分支的文件（如 `ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntities.py`）也执行同样替换，让 `FUNC_XREFS_WINDOWS` 与 `FUNC_XREFS_LINUX` 都带上第四项空列表。

- [ ] **Step 3: 确认旧注释已清零**

Run:

```bash
rg -n "# \\(func_name, xref_strings_list, xref_funcs_list\\)$" ida_preprocessor_scripts
rg -n "# \\(func_name, xref_strings_list, xref_funcs_list, exclude_funcs_list\\)" ida_preprocessor_scripts
```

Expected: 第一条命令无输出；第二条命令命中本任务列出的所有脚本。

- [ ] **Step 4: 提交脚本迁移**

Run:

```bash
git add ida_preprocessor_scripts
git commit -m "refactor: 统一func_xrefs四元组格式"
```

Expected: 产生一条只包含脚本格式迁移的提交。

### Task 3: 新增 `find-CNetworkGameClient_RecordEntityBandwidth.py`

**Files:**
- Create: `ida_preprocessor_scripts/find-CNetworkGameClient_RecordEntityBandwidth.py`

- [ ] **Step 1: 确认目标脚本尚未存在**

Run:

```bash
test ! -f ida_preprocessor_scripts/find-CNetworkGameClient_RecordEntityBandwidth.py
```

Expected: 退出码为 0；如果退出码非 0，先打开现有文件确认是否需要改为“修改而不是创建”。

- [ ] **Step 2: 写入完整脚本**

Create `ida_preprocessor_scripts/find-CNetworkGameClient_RecordEntityBandwidth.py`:

```python
#!/usr/bin/env python3
"""Preprocess script for find-CNetworkGameClient_RecordEntityBandwidth skill."""

from ida_analyze_util import preprocess_common_skill


TARGET_FUNCTION_NAMES = [
    "CNetworkGameClient_RecordEntityBandwidth",
]

FUNC_XREFS = [
    # (func_name, xref_strings_list, xref_funcs_list, exclude_funcs_list)
    (
        "CNetworkGameClient_RecordEntityBandwidth",
        [
            "Local Player",
            "Other Players",
        ],
        [],
        [
            "CNetworkServerService_Init",
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

- [ ] **Step 3: 做定向语法与内容检查**

Run:

```bash
python -m py_compile ida_preprocessor_scripts/find-CNetworkGameClient_RecordEntityBandwidth.py
rg -n "CNetworkGameClient_RecordEntityBandwidth|CNetworkServerService_Init|exclude_funcs_list" ida_preprocessor_scripts/find-CNetworkGameClient_RecordEntityBandwidth.py
```

Expected: `py_compile` 无输出且退出码为 0；`rg` 命中新脚本中的目标函数名、排除函数名和四元组注释。

- [ ] **Step 4: 提交新增脚本**

Run:

```bash
git add ida_preprocessor_scripts/find-CNetworkGameClient_RecordEntityBandwidth.py
git commit -m "feat: 新增RecordEntityBandwidth预处理"
```

Expected: 产生一条只包含新脚本的提交。

### Task 4: 在 `config.yaml` 接入新 skill 与 symbol

**Files:**
- Modify: `config.yaml`

- [ ] **Step 1: 验证依赖函数已在同模块接入**

Run:

```bash
rg -n "find-CNetworkServerService_Init|CNetworkServerService_Init|find-CNetworkGameClient_ProcessPacketEntities|CNetworkGameClient_ProcessPacketEntities" config.yaml
```

Expected: `CNetworkServerService_Init` 的 skill/symbol 与 `CNetworkGameClient_ProcessPacketEntities*` 位于同一个 `engine` 模块块内；如果不是，停止执行并重审 `new_binary_dir` 假设。

- [ ] **Step 2: 在 `engine` 模块中插入 skill 与 symbol**

在 `config.yaml` 的 `engine` 模块技能区、将新 skill 放到 `find-CNetworkServerService_Init` 之后插入：

```yaml
      - name: find-CNetworkGameClient_RecordEntityBandwidth
        expected_output:
          - CNetworkGameClient_RecordEntityBandwidth.{platform}.yaml
        expected_input:
          - CNetworkServerService_Init.{platform}.yaml
```

在同一模块的 `symbols` 区、紧跟 `CNetworkGameClient_ProcessPacketEntities` 之后插入：

```yaml
      - name: CNetworkGameClient_RecordEntityBandwidth
        category: func
        alias:
          - CNetworkGameClient::RecordEntityBandwidth
          - RecordEntityBandwidth
```

- [ ] **Step 3: 用 YAML 解析验证配置闭合**

Run:

```bash
python - <<'PY'
from pathlib import Path
import yaml

config = yaml.safe_load(Path("config.yaml").read_text(encoding="utf-8"))
module = next(item for item in config["modules"] if item["name"] == "engine")

skill = next(
    item for item in module["skills"]
    if item["name"] == "find-CNetworkGameClient_RecordEntityBandwidth"
)
assert skill["expected_output"] == [
    "CNetworkGameClient_RecordEntityBandwidth.{platform}.yaml"
]
assert "CNetworkServerService_Init.{platform}.yaml" in skill.get("expected_input", [])

symbol = next(
    item for item in module["symbols"]
    if item["name"] == "CNetworkGameClient_RecordEntityBandwidth"
)
assert symbol["category"] == "func"
assert "CNetworkGameClient::RecordEntityBandwidth" in symbol.get("alias", [])
assert "RecordEntityBandwidth" in symbol.get("alias", [])
PY
```

Expected: 无输出且退出码为 0，说明 skill/symbol 与依赖项都已写入。

- [ ] **Step 4: 提交配置接入**

Run:

```bash
git add config.yaml
git commit -m "feat: 接入RecordEntityBandwidth配置"
```

Expected: 产生一条只包含 `config.yaml` 接入的提交。

### Task 5: 做最终静态回归验证

**Files:**
- Verify: `ida_analyze_util.py`
- Verify: `config.yaml`
- Verify: `ida_preprocessor_scripts/find-CNetworkGameClient_RecordEntityBandwidth.py`
- Verify: 本计划中所有迁移过的 `ida_preprocessor_scripts/*.py`

- [ ] **Step 1: 确认旧三元组契约已完全移除**

Run:

```bash
rg -n "len\\(spec\\) != 3|\\(func_name, xref_strings_list, xref_funcs_list\\)" ida_analyze_util.py
rg -n "# \\(func_name, xref_strings_list, xref_funcs_list\\)$" ida_preprocessor_scripts
```

Expected: 两条命令都无输出。

- [ ] **Step 2: 对所有修改过的 Python 文件执行语法检查**

Run:

```bash
python -m py_compile \
  ida_analyze_util.py \
  ida_preprocessor_scripts/find-CDemoRecorder_ParseMessage.py \
  ida_preprocessor_scripts/find-CNetChan_ParseMessagesDemo.py \
  ida_preprocessor_scripts/find-CNetChan_ProcessMessages.py \
  ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntities.py \
  ida_preprocessor_scripts/find-CNetworkGameClient_ProcessPacketEntitiesInternal.py \
  ida_preprocessor_scripts/find-CNetworkGameClient_RecordEntityBandwidth.py \
  ida_preprocessor_scripts/find-CNetworkGameClient_SendMove.py \
  ida_preprocessor_scripts/find-CNetworkGameClient_SendMovePacket.py \
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
  ida_preprocessor_scripts/find-CNetworkMessages_UnserializeMessageInternal.py \
  ida_preprocessor_scripts/find-CNetworkServerService_Init.py
```

Expected: 无输出且退出码为 0。

- [ ] **Step 3: 用定向检索确认新链路已打通**

Run:

```bash
rg -n "exclude_funcs|CNetworkGameClient_RecordEntityBandwidth|CNetworkServerService_Init" \
  ida_analyze_util.py \
  ida_preprocessor_scripts/find-CNetworkGameClient_RecordEntityBandwidth.py \
  config.yaml
```

Expected: 三个文件都命中相应关键字，证明“核心逻辑 + 新脚本 + 配置”已连通。

- [ ] **Step 4: 提交最终验证状态**

Run:

```bash
git add ida_analyze_util.py ida_preprocessor_scripts config.yaml
git commit -m "chore: 完成func_xrefs排除函数接入"
```

Expected: 产生最终收尾提交；如果前面已经按任务分提交，这里只在有额外修补时再提交，否则跳过。
