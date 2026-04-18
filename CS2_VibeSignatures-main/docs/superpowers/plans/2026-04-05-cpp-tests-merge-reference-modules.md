# CPP Tests Merge Reference Modules Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为 `run_cpp_tests.py` / `cpp_tests_util.py` 增加默认启用的 `reference_modules` 合并对比能力，使多模块 YAML 先聚合再只执行一次 vtable compare，并保留显式回退到旧式逐模块 compare 的能力。

**Architecture:** 保留 `compare_compiler_vtable_with_yaml()` 作为统一 compare 入口，在 `cpp_tests_util.py` 中新增 merged reference 加载器，负责跨模块汇总 `vtable_size`、`vtable_numvfunc` 与 `functions_by_index`，同时记录来源与冲突。随后在 `run_cpp_tests.py` 中引入 `merge_reference_modules` 布尔语义，默认走 merged 模式，仅在显式关闭时保留当前逐模块 compare 行为。

**Tech Stack:** Python 3、PyYAML、`uv`、clang++、现有 `run_cpp_tests.py` CLI

---

## File Structure

- Modify: `cpp_tests_util.py`
  - 新增 merged reference 加载逻辑
  - 为 merged/single 两种模式统一整理 compare report 结构
  - 扩展文本 formatter，输出 merged modules / files / conflicts
- Modify: `run_cpp_tests.py`
  - 新增布尔解析辅助逻辑
  - 在 `run_one_test()` 中默认启用 merged compare
  - 仅在 `merge_reference_modules: false` 时保留逐模块 compare
- Modify: `config.yaml`（仅用于本地回退验证，不提交永久配置变更）
  - 用临时副本验证 `merge_reference_modules: false` 的旧行为

**验证说明：**

- 仓库当前没有一套独立维护的第一方 Python 单元测试目录，因此本计划不引入新的测试框架文件。
- 所有验证都使用现有工具完成：
  - `uv run python - <<'PY'` 内联验证 `cpp_tests_util.py` 的纯函数行为
  - `uv run run_cpp_tests.py -gamever 14141b -debug` 做现有 CLI 的定向 smoke 验证

### Task 1: 在 `cpp_tests_util.py` 中新增 merged reference 加载器

**Files:**
- Modify: `cpp_tests_util.py`

- [ ] **Step 1: 先写失败验证，确认 merged loader 目前不存在**

Run:

```bash
uv run python - <<'PY'
from cpp_tests_util import load_merged_reference_vtable_data
print(load_merged_reference_vtable_data)
PY
```

Expected: FAIL，报 `ImportError: cannot import name 'load_merged_reference_vtable_data'`。

- [ ] **Step 2: 实现 merged loader 与冲突记录辅助逻辑**

将 `cpp_tests_util.py` 中 `load_reference_vtable_data()` 上方补成下面的结构：

```python
def _append_reference_conflict(
    conflicts: List[Dict[str, Any]],
    *,
    conflict_type: str,
    message: str,
    index: Optional[int] = None,
    sources: Optional[List[Dict[str, Any]]] = None,
) -> None:
    item: Dict[str, Any] = {
        "type": conflict_type,
        "message": message,
    }
    if index is not None:
        item["index"] = index
    if sources:
        item["sources"] = sources
    conflicts.append(item)


def load_merged_reference_vtable_data(
    bindir: Path,
    gamever: str,
    class_name: str,
    platform: str,
    reference_modules: Sequence[str],
    alias_class_names: Sequence[str] = (),
) -> Optional[Dict[str, Any]]:
    if yaml is None:
        raise RuntimeError("PyYAML is required to read reference YAML files")

    class_names_to_try = [class_name] + [n for n in alias_class_names if n]
    merged: Dict[str, Any] = {
        "mode": "merged",
        "modules": [],
        "files": [],
        "vtable_size": None,
        "vtable_size_raw": None,
        "vtable_size_source": None,
        "vtable_numvfunc": None,
        "vtable_numvfunc_source": None,
        "functions_by_index": {},
        "conflicts": [],
    }
    alias_used: Optional[str] = None

    for module in reference_modules:
        module_dir = bindir / gamever / module
        if not module_dir.is_dir():
            continue

        module_hit = False
        for effective_class_name in class_names_to_try:
            pattern = f"{effective_class_name}_*.{platform}.yaml"
            files = sorted(module_dir.glob(pattern))
            if not files:
                continue

            if alias_used is None and effective_class_name != class_name:
                alias_used = effective_class_name

            for path in files:
                try:
                    with path.open("r", encoding="utf-8") as f:
                        payload = yaml.safe_load(f) or {}
                except Exception:
                    continue
                if not isinstance(payload, dict):
                    continue

                module_hit = True
                merged["files"].append(str(path))

                parsed_size = _parse_int_maybe(payload.get("vtable_size"))
                if parsed_size is not None:
                    size_source = {
                        "module": module,
                        "path": str(path),
                        "value": parsed_size,
                    }
                    current_size = merged.get("vtable_size")
                    if current_size is None:
                        merged["vtable_size"] = parsed_size
                        merged["vtable_size_raw"] = str(payload.get("vtable_size"))
                        merged["vtable_size_source"] = size_source
                    elif current_size != parsed_size:
                        previous_source = merged.get("vtable_size_source") or {
                            "module": "unknown",
                            "path": "unknown",
                            "value": current_size,
                        }
                        _append_reference_conflict(
                            merged["conflicts"],
                            conflict_type="reference_conflict_vtable_size",
                            message=(
                                f"Reference vtable_size conflict: "
                                f"{previous_source['module']}={current_size} vs "
                                f"{module}={parsed_size}."
                            ),
                            sources=[previous_source, size_source],
                        )

                parsed_numvfunc = _parse_int_maybe(payload.get("vtable_numvfunc"))
                if parsed_numvfunc is not None:
                    numvfunc_source = {
                        "module": module,
                        "path": str(path),
                        "value": parsed_numvfunc,
                    }
                    current_numvfunc = merged.get("vtable_numvfunc")
                    if current_numvfunc is None:
                        merged["vtable_numvfunc"] = parsed_numvfunc
                        merged["vtable_numvfunc_source"] = numvfunc_source
                    elif current_numvfunc != parsed_numvfunc:
                        previous_source = merged.get("vtable_numvfunc_source") or {
                            "module": "unknown",
                            "path": "unknown",
                            "value": current_numvfunc,
                        }
                        _append_reference_conflict(
                            merged["conflicts"],
                            conflict_type="reference_conflict_vtable_numvfunc",
                            message=(
                                f"Reference vtable_numvfunc conflict: "
                                f"{previous_source['module']}={current_numvfunc} vs "
                                f"{module}={parsed_numvfunc}."
                            ),
                            sources=[previous_source, numvfunc_source],
                        )

                parsed_index = _parse_int_maybe(payload.get("vfunc_index"))
                if parsed_index is None:
                    continue

                func_name = payload.get("func_name")
                member_name = _normalize_reference_member_name(
                    class_name=effective_class_name,
                    func_name=str(func_name) if func_name is not None else None,
                    file_stem=path.stem,
                )
                source = {
                    "module": module,
                    "path": str(path),
                    "func_name": (
                        str(func_name) if func_name is not None else path.stem
                    ),
                    "member_name": member_name,
                }

                current_entry = merged["functions_by_index"].get(parsed_index)
                if current_entry is None:
                    merged["functions_by_index"][parsed_index] = {
                        "func_name": source["func_name"],
                        "member_name": source["member_name"],
                        "path": source["path"],
                        "module": source["module"],
                        "sources": [source],
                    }
                    continue

                current_entry["sources"].append(source)
                current_member = current_entry.get("member_name", "")
                incoming_member = source.get("member_name", "")
                if current_member and incoming_member and current_member != incoming_member:
                    _append_reference_conflict(
                        merged["conflicts"],
                        conflict_type="reference_conflict_vfunc_name",
                        index=parsed_index,
                        message=(
                            f"Reference index {parsed_index} conflict: "
                            f"{current_entry['module']}={current_member} vs "
                            f"{module}={incoming_member}."
                        ),
                        sources=current_entry["sources"],
                    )
                elif not current_member and incoming_member:
                    current_entry["member_name"] = incoming_member
                    current_entry["func_name"] = source["func_name"]
                    current_entry["path"] = source["path"]
                    current_entry["module"] = source["module"]

        if module_hit:
            merged["modules"].append(module)

    if not merged["files"]:
        return None
    if alias_used:
        merged["alias_class_name"] = alias_used
    return merged
```

保留现有 `load_reference_vtable_data()`，不要删除旧逻辑；它仍是 `merge_reference_modules=False` 的回退实现。

- [ ] **Step 3: 运行补全合并验证，确认 alias + 多模块补全可用**

Run:

```bash
uv run python - <<'PY'
import tempfile
from pathlib import Path
import yaml

from cpp_tests_util import load_merged_reference_vtable_data

root = Path(tempfile.mkdtemp())
(root / "14141b" / "networksystem").mkdir(parents=True)
(root / "14141b" / "engine").mkdir(parents=True)

(root / "14141b" / "networksystem" / "CNetworkMessages_vtable.windows.yaml").write_text(
    yaml.safe_dump({"vtable_size": "0x130", "vtable_numvfunc": 38}),
    encoding="utf-8",
)
(root / "14141b" / "networksystem" / "CNetworkMessages_RegisterNetworkCategory.windows.yaml").write_text(
    yaml.safe_dump({"vfunc_index": 1, "func_name": "CNetworkMessages_RegisterNetworkCategory"}),
    encoding="utf-8",
)
(root / "14141b" / "engine" / "CNetworkMessages_Serialize.windows.yaml").write_text(
    yaml.safe_dump({"vfunc_index": 2, "func_name": "CNetworkMessages_Serialize"}),
    encoding="utf-8",
)

merged = load_merged_reference_vtable_data(
    bindir=root,
    gamever="14141b",
    class_name="INetworkMessages",
    platform="windows",
    reference_modules=["networksystem", "engine", "client"],
    alias_class_names=["CNetworkMessages"],
)

assert merged is not None
assert merged["mode"] == "merged"
assert merged["modules"] == ["networksystem", "engine"]
assert merged["alias_class_name"] == "CNetworkMessages"
assert merged["vtable_size"] == 0x130
assert merged["vtable_numvfunc"] == 38
assert merged["functions_by_index"][1]["member_name"] == "RegisterNetworkCategory"
assert merged["functions_by_index"][2]["member_name"] == "Serialize"
assert merged["conflicts"] == []
print("PASS: merged reference loader complements module data correctly")
PY
```

Expected: PASS，并输出 `PASS: merged reference loader complements module data correctly`。

- [ ] **Step 4: 提交当前 loader 改动**

Run:

```bash
git add cpp_tests_util.py
git commit -m "feat(cpp-tests): 增加合并式reference加载"
```

Expected: commit 成功，工作区只留下后续任务未完成的改动。

### Task 2: 扩展 compare/report 以支持 merged 模式与 reference 冲突

**Files:**
- Modify: `cpp_tests_util.py`

- [ ] **Step 1: 先写失败验证，确认 compare 入口还不支持 merged 参数**

Run:

```bash
uv run python - <<'PY'
from pathlib import Path
from cpp_tests_util import compare_compiler_vtable_with_yaml

compare_compiler_vtable_with_yaml(
    class_name="INetworkMessages",
    compiler_output="",
    bindir=Path("."),
    gamever="14141b",
    platform="windows",
    reference_modules=["networksystem"],
    pointer_size=8,
    alias_class_names=["CNetworkMessages"],
    merge_reference_modules=True,
)
PY
```

Expected: FAIL，报 `TypeError: compare_compiler_vtable_with_yaml() got an unexpected keyword argument 'merge_reference_modules'`。

- [ ] **Step 2: 修改 compare 入口与 formatter，统一 merged/single 报告结构**

将 `compare_compiler_vtable_with_yaml()` 与 `format_vtable_compare_report()` 改成下面的结构：

```python
def compare_compiler_vtable_with_yaml(
    *,
    class_name: str,
    compiler_output: str,
    bindir: Path,
    gamever: str,
    platform: str,
    reference_modules: Sequence[str],
    pointer_size: int,
    alias_class_names: Sequence[str] = (),
    merge_reference_modules: bool = True,
) -> Dict[str, Any]:
    parsed_layouts = parse_vftable_layouts(compiler_output)
    compiler_section = parsed_layouts.get(class_name)

    if merge_reference_modules:
        reference = load_merged_reference_vtable_data(
            bindir=bindir,
            gamever=gamever,
            class_name=class_name,
            platform=platform,
            reference_modules=reference_modules,
            alias_class_names=alias_class_names,
        )
    else:
        reference = load_reference_vtable_data(
            bindir=bindir,
            gamever=gamever,
            class_name=class_name,
            platform=platform,
            reference_modules=reference_modules,
            alias_class_names=alias_class_names,
        )

    reference_mode = "merged" if merge_reference_modules else "single"
    alias_used = reference.get("alias_class_name") if reference else None
    report: Dict[str, Any] = {
        "class_name": class_name,
        "platform": platform,
        "requested_modules": list(reference_modules),
        "reference_mode": reference_mode,
        "compiler_found": compiler_section is not None,
        "reference_found": reference is not None,
        "reference_module": reference.get("module") if reference else None,
        "reference_modules_merged": reference.get("modules", []) if reference else [],
        "reference_files_merged": reference.get("files", []) if reference else [],
        "reference_conflicts": reference.get("conflicts", []) if reference else [],
        "differences": [],
        "notes": [],
    }

    if alias_used:
        report["alias_class_name"] = alias_used
        report["notes"].append(
            f"Reference YAML matched via alias symbol '{alias_used}' "
            f"(primary symbol '{class_name}' not found)."
        )

    if compiler_section is None:
        report["notes"].append(
            f"No vtable section for class '{class_name}' found in compiler output."
        )
        return report

    compiler_entry_count = compiler_section["entry_count"]
    declared_entries = compiler_section["declared_entries"]
    methods_by_index = compiler_section["methods_by_index"]
    report["compiler_entry_count"] = compiler_entry_count
    report["compiler_declared_entries"] = declared_entries
    report["compiler_vtable_size"] = compiler_entry_count * pointer_size

    if declared_entries != compiler_entry_count:
        report["differences"].append(
            {
                "type": "compiler_declared_count_mismatch",
                "message": (
                    f"Compiler declares {declared_entries} vtable entries, "
                    f"but parsed {compiler_entry_count} entries."
                ),
            }
        )

    if reference is None:
        report["notes"].append(
            f"No matching reference YAML found for modules: {', '.join(reference_modules)}"
        )
        return report

    expected_size = reference.get("vtable_size")
    expected_numvfunc = reference.get("vtable_numvfunc")
    reference_functions = reference.get("functions_by_index", {})
    report["reference_vtable_size"] = expected_size
    report["reference_vtable_numvfunc"] = expected_numvfunc
    report["reference_functions_count"] = len(reference_functions)

    for conflict in report["reference_conflicts"]:
        report["differences"].append(
            {
                "type": conflict["type"],
                "message": conflict["message"],
            }
        )

    if expected_size is not None and expected_size != report["compiler_vtable_size"]:
        report["differences"].append(
            {
                "type": "vtable_size_mismatch",
                "message": (
                    f"vtable_size mismatch: YAML={hex(expected_size)} "
                    f"vs compiler={hex(report['compiler_vtable_size'])} "
                    f"(entry_count={compiler_entry_count}, ptr_size={pointer_size})."
                ),
            }
        )

    if expected_numvfunc is not None and expected_numvfunc != compiler_entry_count:
        report["differences"].append(
            {
                "type": "vtable_numvfunc_mismatch",
                "message": (
                    f"vtable_numvfunc mismatch: YAML={expected_numvfunc} "
                    f"vs compiler={compiler_entry_count}."
                ),
            }
        )

    for index in sorted(reference_functions.keys()):
        ref_item = reference_functions[index]
        compiled = methods_by_index.get(index)
        if compiled is None:
            report["differences"].append(
                {
                    "type": "vfunc_index_missing",
                    "message": (
                        f"Index {index} missing in compiler output "
                        f"(reference: {ref_item['func_name']}, file: {ref_item['path']})."
                    ),
                }
            )
            continue

        expected_member = ref_item.get("member_name", "")
        actual_member = compiled.get("member_name", "")
        if expected_member and actual_member and expected_member != actual_member:
            report["differences"].append(
                {
                    "type": "vfunc_name_mismatch",
                    "message": (
                        f"Index {index} mismatch: YAML expects '{expected_member}' "
                        f"but compiler reports '{actual_member}' "
                        f"(selected from {ref_item['module']}: {ref_item['path']})."
                    ),
                }
            )

    if not report["differences"]:
        report["notes"].append(
            "No differences detected for vtable_size/vtable_numvfunc/vfunc_index mapping."
        )

    return report
```

```python
def format_vtable_compare_report(report: Dict[str, Any]) -> List[str]:
    lines: List[str] = []
    lines.append(
        f"Class '{report['class_name']}' compare target platform: {report.get('platform', 'unknown')}"
    )

    if not report.get("compiler_found"):
        lines.extend(report.get("notes", []))
        return lines

    lines.append(
        f"Compiler vtable entries: parsed={report.get('compiler_entry_count')}, "
        f"declared={report.get('compiler_declared_entries')}"
    )

    if report.get("reference_found"):
        if report.get("reference_mode") == "merged":
            merged_modules = report.get("reference_modules_merged", [])
            lines.append("Reference mode: merged")
            lines.append(
                f"Reference modules: {', '.join(merged_modules) if merged_modules else 'none'}"
            )
            lines.append(
                f"Reference files merged: {len(report.get('reference_files_merged', []))}"
            )
            lines.append(
                f"Reference functions: {report.get('reference_functions_count', 0)}"
            )
            lines.append(
                f"Reference conflicts found: {len(report.get('reference_conflicts', []))}"
            )
        else:
            lines.append(
                f"Reference module: {report.get('reference_module')}, "
                f"reference functions: {report.get('reference_functions_count', 0)}"
            )
    else:
        requested_modules = report.get("requested_modules", [])
        if requested_modules:
            lines.append(
                f"Reference module (requested): {', '.join(requested_modules)}; not found"
            )
        else:
            lines.append("Reference module: not found")

    diffs = report.get("differences", [])
    if diffs:
        lines.append(f"Differences found: {len(diffs)}")
        for item in diffs:
            lines.append(f"- {item['message']}")
    else:
        for note in report.get("notes", []):
            lines.append(note)

    return lines
```

保持 `format_vtable_differences_for_agent()` 继续只读取 `report["differences"]`，这样 merged 冲突会自动进入 `-fixheader` 提示文本。

- [ ] **Step 3: 运行 compare/report 验证，确认 merged 信息与冲突进入差异模型**

Run:

```bash
uv run python - <<'PY'
import tempfile
from pathlib import Path
import yaml

from cpp_tests_util import (
    compare_compiler_vtable_with_yaml,
    format_vtable_compare_report,
)

root = Path(tempfile.mkdtemp())
(root / "14141b" / "networksystem").mkdir(parents=True)
(root / "14141b" / "engine").mkdir(parents=True)
(root / "14141b" / "server").mkdir(parents=True)

(root / "14141b" / "networksystem" / "CNetworkMessages_vtable.windows.yaml").write_text(
    yaml.safe_dump({"vtable_size": "0x18", "vtable_numvfunc": 3}),
    encoding="utf-8",
)
(root / "14141b" / "networksystem" / "CNetworkMessages_RegisterNetworkCategory.windows.yaml").write_text(
    yaml.safe_dump({"vfunc_index": 1, "func_name": "CNetworkMessages_RegisterNetworkCategory"}),
    encoding="utf-8",
)
(root / "14141b" / "engine" / "CNetworkMessages_Serialize.windows.yaml").write_text(
    yaml.safe_dump({"vfunc_index": 2, "func_name": "CNetworkMessages_Serialize"}),
    encoding="utf-8",
)
(root / "14141b" / "server" / "CNetworkMessages_SerializeInternal.windows.yaml").write_text(
    yaml.safe_dump({"vfunc_index": 2, "func_name": "CNetworkMessages_SerializeInternal"}),
    encoding="utf-8",
)

compiler_output = \"\"\"\
VFTable indices for 'INetworkMessages' (3 entries).
  0 | INetworkMessages::~INetworkMessages()
  1 | INetworkMessages::RegisterNetworkCategory()
  2 | INetworkMessages::Serialize()
\"\"\"

report = compare_compiler_vtable_with_yaml(
    class_name="INetworkMessages",
    compiler_output=compiler_output,
    bindir=root,
    gamever="14141b",
    platform="windows",
    reference_modules=["networksystem", "engine", "server"],
    pointer_size=8,
    alias_class_names=["CNetworkMessages"],
    merge_reference_modules=True,
)
lines = format_vtable_compare_report(report)

assert report["reference_mode"] == "merged"
assert report["reference_modules_merged"] == ["networksystem", "engine", "server"]
assert any(item["type"] == "reference_conflict_vfunc_name" for item in report["differences"])
assert "Reference mode: merged" in lines
assert "Reference conflicts found: 1" in lines
assert any("Serialize vs SerializeInternal" in item["message"] for item in report["differences"])
print("PASS: merged compare report includes provenance and conflicts")
PY
```

Expected: PASS，并输出 `PASS: merged compare report includes provenance and conflicts`。

- [ ] **Step 4: 提交 compare/report 改动**

Run:

```bash
git add cpp_tests_util.py
git commit -m "feat(cpp-tests): 补齐合并模式对比报告"
```

Expected: commit 成功。

### Task 3: 在 `run_cpp_tests.py` 中默认启用 merged compare，并验证显式回退

**Files:**
- Modify: `run_cpp_tests.py`

- [ ] **Step 1: 记录当前入口基线，确认仍是逐模块循环 compare**

Run:

```bash
rg -n "_to_bool|merge_reference_modules|for module_name in reference_modules|compare_compiler_vtable_with_yaml\\(" run_cpp_tests.py
```

Expected:

- 没有 `_to_bool`
- 没有 `merge_reference_modules`
- 存在 `for module_name in reference_modules:` 循环

- [ ] **Step 2: 新增布尔解析并让 `run_one_test()` 默认合并 modules**

在 `_to_text()` 下方新增：

```python
def _to_bool(value: Any, default: bool = False) -> bool:
    if value is None:
        return default
    if isinstance(value, bool):
        return value
    if isinstance(value, str):
        text = value.strip().lower()
        if not text:
            return default
        if text in {"1", "true", "yes", "on"}:
            return True
        if text in {"0", "false", "no", "off"}:
            return False
    return bool(value)
```

然后把 `run_one_test()` 中的 compare 分支改成：

```python
        else:
            reference_modules = _to_list(test_item.get("reference_modules"))
            alias_symbols = _to_list(test_item.get("alias_symbols"))
            merge_reference_modules = _to_bool(
                test_item.get("merge_reference_modules"),
                default=True,
            )
            compare_reports = []

            if not reference_modules or merge_reference_modules:
                compare_reports.append(
                    compare_compiler_vtable_with_yaml(
                        class_name=symbol,
                        compiler_output=compile_output,
                        bindir=bindir,
                        gamever=args.gamever,
                        platform=platform,
                        reference_modules=reference_modules,
                        pointer_size=pointer_size_from_target_triple(target),
                        alias_class_names=alias_symbols,
                        merge_reference_modules=merge_reference_modules,
                    )
                )
            else:
                for module_name in reference_modules:
                    compare_reports.append(
                        compare_compiler_vtable_with_yaml(
                            class_name=symbol,
                            compiler_output=compile_output,
                            bindir=bindir,
                            gamever=args.gamever,
                            platform=platform,
                            reference_modules=[module_name],
                            pointer_size=pointer_size_from_target_triple(target),
                            alias_class_names=alias_symbols,
                            merge_reference_modules=False,
                        )
                    )
```

不要改 `compare_run_count` 与 `compare_diff_count` 的汇总逻辑；它们会自然跟随 `compare_reports` 的条数变化。

- [ ] **Step 3: 跑默认 merged smoke，确认 `INetworkMessages_MSVC` 只 compare 一次**

Run:

```bash
tmp_log="$(mktemp)"
uv run run_cpp_tests.py -gamever 14141b -debug >"$tmp_log"
uv run python - <<'PY' "$tmp_log"
from pathlib import Path
import sys

text = Path(sys.argv[1]).read_text(encoding="utf-8")
assert text.count("Class 'INetworkMessages' compare target platform: windows") == 1
assert "Reference mode: merged" in text
assert "Reference modules:" in text
assert "networksystem" in text
assert "engine" in text
assert "server" in text
print("PASS: default merged mode emits a single INetworkMessages compare report")
PY
```

Expected: PASS，并输出 `PASS: default merged mode emits a single INetworkMessages compare report`。

- [ ] **Step 4: 用临时 config 副本验证 `merge_reference_modules: false` 会恢复旧行为**

Run:

```bash
tmp_cfg="$(mktemp --suffix=.yaml)"
uv run python - <<'PY' "$tmp_cfg"
from pathlib import Path
import sys
import yaml

config = yaml.safe_load(Path("config.yaml").read_text(encoding="utf-8"))
for item in config.get("cpp_tests", []):
    if item.get("name") == "INetworkMessages_MSVC":
        item["merge_reference_modules"] = False

Path(sys.argv[1]).write_text(
    yaml.safe_dump(config, sort_keys=False, allow_unicode=True),
    encoding="utf-8",
)
PY

tmp_log="$(mktemp)"
uv run run_cpp_tests.py -configyaml "$tmp_cfg" -gamever 14141b -debug >"$tmp_log"
uv run python - <<'PY' "$tmp_log"
from pathlib import Path
import sys

text = Path(sys.argv[1]).read_text(encoding="utf-8")
assert text.count("Class 'INetworkMessages' compare target platform: windows") == 4
assert "Reference module: networksystem" in text
assert "Reference mode: merged" not in text
print("PASS: explicit opt-out restores per-module compare behavior")
PY
```

Expected: PASS，并输出 `PASS: explicit opt-out restores per-module compare behavior`。

- [ ] **Step 5: 提交入口与默认行为改动**

Run:

```bash
git add run_cpp_tests.py cpp_tests_util.py
git commit -m "feat(cpp-tests): 默认启用模块合并比对"
```

Expected: commit 成功，默认行为已切换，且显式回退仍可用。

## Self-Review Checklist

- [ ] 对照 `docs/superpowers/specs/2026-04-05-cpp-tests-merge-reference-modules-design.md`，确认以下要求都有任务覆盖：
  - 默认启用 merged 模式
  - 保留 `merge_reference_modules: false` 回退能力
  - 合并 `vtable_size` / `vtable_numvfunc` / `functions_by_index`
  - 冲突显式进入 `differences`
  - 报告保留来源模块与文件
  - `INetworkMessages_MSVC` 只 compare 一次
- [ ] 对计划文件执行占位词扫描，确认正文部分没有 `TBD`、`TODO`、`implement later`、`similar to` 之类的空洞描述：

```bash
uv run python - <<'PY'
from pathlib import Path

plan_path = Path("docs/superpowers/plans/2026-04-05-cpp-tests-merge-reference-modules.md")
body = plan_path.read_text(encoding="utf-8").split("## Self-Review Checklist")[0]
needles = ("TBD", "TODO", "implement later", "Similar to", "similar to")
hits = [needle for needle in needles if needle in body]
assert not hits, hits
print("PASS: no placeholders found in plan body")
PY
```

Expected: PASS，并输出 `PASS: no placeholders found in plan body`。

- [ ] 对照代码片段，确认名称前后一致：
  - `load_merged_reference_vtable_data`
  - `_append_reference_conflict`
  - `_to_bool`
  - `merge_reference_modules`
  - `reference_mode`
  - `reference_modules_merged`
  - `reference_files_merged`
  - `reference_conflicts`
