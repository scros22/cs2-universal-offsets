#!/usr/bin/env python3
"""Utility helpers for C++ vtable test scripts."""

from __future__ import annotations

import re
from pathlib import Path
from typing import Any, Dict, List, Optional, Sequence

try:
    import yaml
except ImportError:
    yaml = None


VFTABLE_HEADER_RE = re.compile(
    r"^\s*(?:VFTable|VTable) indices for '([^']+)' \((\d+) entries\)\.\s*$"
)
VFTABLE_ENTRY_RE = re.compile(r"^\s*(\d+)\s+\|\s+(.+?)\s*$")


def map_target_triple_to_platform(target_triple: str) -> Optional[str]:
    """
    Map configured target triple to platform name used by YAML output files.

    Rules:
    - x86_64-pc-windows-msvc => windows
    - x86_64-pc-windows-gnu  => linux
    - x86_64-*-linux-gnu     => linux
    """
    if target_triple == "x86_64-pc-windows-msvc":
        return "windows"
    if target_triple == "x86_64-pc-windows-gnu":
        return "linux"
    if re.match(r"^x86_64-[^-]+-linux-gnu$", target_triple):
        return "linux"
    return None


def pointer_size_from_target_triple(target_triple: str) -> int:
    """Infer pointer size from the target triple."""
    if target_triple.startswith("x86_64-"):
        return 8
    return 8


def parse_vftable_layouts(compiler_output: str) -> Dict[str, Dict[str, Any]]:
    """
    Parse clang `-fdump-vtable-layouts` output.

    Returns:
        {
          "<ClassName>": {
            "declared_entries": int,
            "methods_by_index": {
              <idx>: {
                "signature": "<full signature line>",
                "member_name": "<member token if parsed>"
              }
            },
            "entry_count": int
          },
          ...
        }
    """
    parsed: Dict[str, Dict[str, Any]] = {}
    current_class: Optional[str] = None
    current_declared_entries = 0

    for raw_line in compiler_output.splitlines():
        header = VFTABLE_HEADER_RE.match(raw_line)
        if header:
            current_class = header.group(1)
            declared_entries = int(header.group(2))
            current_declared_entries = declared_entries
            parsed[current_class] = {
                "declared_entries": declared_entries,
                "methods_by_index": {},
                "entry_count": 0,
            }
            continue

        if current_class is None:
            continue

        entry = VFTABLE_ENTRY_RE.match(raw_line)
        if not entry:
            if (
                current_class is not None
                and parsed[current_class]["methods_by_index"]
                and not raw_line.strip()
            ):
                current_class = None
                current_declared_entries = 0
            continue

        index = int(entry.group(1))
        # Stop at section boundary to avoid swallowing "N | ..." lines
        # from other vtable-related blocks in clang output.
        if index >= current_declared_entries:
            current_class = None
            current_declared_entries = 0
            continue

        signature = entry.group(2).strip()
        member_name = _extract_member_name(signature, current_class)
        parsed[current_class]["methods_by_index"][index] = {
            "signature": signature,
            "member_name": member_name,
        }

        if len(parsed[current_class]["methods_by_index"]) >= current_declared_entries:
            current_class = None
            current_declared_entries = 0

    for class_name, section in parsed.items():
        section["entry_count"] = len(section["methods_by_index"])

    return parsed


def _extract_member_name(signature: str, class_name: str) -> str:
    marker = f"{class_name}::"
    pos = signature.find(marker)
    if pos < 0:
        return ""
    tail = signature[pos + len(marker) :]
    end = tail.find("(")
    if end < 0:
        end = len(tail)
    return tail[:end].strip()


def _parse_int_maybe(value: Any) -> Optional[int]:
    if value is None:
        return None
    if isinstance(value, int):
        return value
    if isinstance(value, str):
        text = value.strip()
        if not text:
            return None
        try:
            return int(text, 0)
        except ValueError:
            return None
    return None


def _normalize_reference_member_name(
    class_name: str,
    func_name: Optional[str],
    file_stem: str,
) -> str:
    candidate = (func_name or file_stem).strip()
    prefix = f"{class_name}_"
    if candidate.startswith(prefix):
        return candidate[len(prefix) :]
    return candidate


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
        item["sources"] = [dict(source) for source in sources]
    conflicts.append(item)


def load_merged_reference_vtable_data(
    bindir: Path,
    gamever: str,
    class_name: str,
    platform: str,
    reference_modules: Sequence[str],
    alias_class_names: Sequence[str] = (),
) -> Optional[Dict[str, Any]]:
    """Load and merge reference YAML info for a class from all modules."""
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
    alias_candidate: Optional[str] = None
    primary_class_hit = False

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

            for path in files:
                try:
                    with path.open("r", encoding="utf-8") as f:
                        payload = yaml.safe_load(f) or {}
                except Exception:
                    continue

                if not isinstance(payload, dict):
                    continue

                parsed_size = _parse_int_maybe(payload.get("vtable_size"))
                parsed_numvfunc = _parse_int_maybe(payload.get("vtable_numvfunc"))
                parsed_index = _parse_int_maybe(payload.get("vfunc_index"))
                has_reference_metadata = (
                    parsed_size is not None
                    or parsed_numvfunc is not None
                    or parsed_index is not None
                )
                if not has_reference_metadata:
                    continue

                module_hit = True
                merged["files"].append(str(path))
                if effective_class_name == class_name:
                    primary_class_hit = True
                elif alias_candidate is None:
                    alias_candidate = effective_class_name

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

                if parsed_index is None:
                    continue

                func_name = payload.get("func_name")
                source = {
                    "module": module,
                    "path": str(path),
                    "func_name": (
                        str(func_name) if func_name is not None else path.stem
                    ),
                    "member_name": _normalize_reference_member_name(
                        class_name=effective_class_name,
                        func_name=str(func_name) if func_name is not None else None,
                        file_stem=path.stem,
                    ),
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

    if not primary_class_hit and alias_candidate:
        merged["alias_class_name"] = alias_candidate
    return merged


def load_reference_vtable_data(
    bindir: Path,
    gamever: str,
    class_name: str,
    platform: str,
    reference_modules: Sequence[str],
    alias_class_names: Sequence[str] = (),
) -> Optional[Dict[str, Any]]:
    """
    Load reference YAML info for a class from modules in priority order.

    The first module that contains vtable/vfunc metadata is selected.
    When alias_class_names is provided, they are tried in order if the
    primary class_name yields no results within a given module.
    """
    if yaml is None:
        raise RuntimeError("PyYAML is required to read reference YAML files")

    class_names_to_try = [class_name] + [n for n in alias_class_names if n]

    for module in reference_modules:
        module_dir = bindir / gamever / module
        if not module_dir.is_dir():
            continue

        for effective_class_name in class_names_to_try:
            pattern = f"{effective_class_name}_*.{platform}.yaml"
            files = sorted(module_dir.glob(pattern))
            if not files:
                continue

            vtable_size: Optional[int] = None
            vtable_size_raw: Optional[str] = None
            vtable_numvfunc: Optional[int] = None
            reference_functions: Dict[int, Dict[str, str]] = {}
            matched_files: List[str] = []

            for path in files:
                try:
                    with path.open("r", encoding="utf-8") as f:
                        payload = yaml.safe_load(f) or {}
                except Exception:
                    continue

                if not isinstance(payload, dict):
                    continue

                matched_files.append(str(path))

                if "vtable_size" in payload:
                    parsed_size = _parse_int_maybe(payload.get("vtable_size"))
                    if parsed_size is not None:
                        vtable_size = parsed_size
                        vtable_size_raw = str(payload.get("vtable_size"))
                    parsed_numvfunc = _parse_int_maybe(payload.get("vtable_numvfunc"))
                    if parsed_numvfunc is not None:
                        vtable_numvfunc = parsed_numvfunc

                parsed_index = _parse_int_maybe(payload.get("vfunc_index"))
                if parsed_index is None:
                    continue

                func_name = payload.get("func_name")
                member_name = _normalize_reference_member_name(
                    class_name=effective_class_name,
                    func_name=str(func_name) if func_name is not None else None,
                    file_stem=path.stem,
                )
                reference_functions[parsed_index] = {
                    "func_name": str(func_name) if func_name is not None else path.stem,
                    "member_name": member_name,
                    "path": str(path),
                }

            if vtable_size is not None or reference_functions:
                result = {
                    "module": module,
                    "files": matched_files,
                    "vtable_size": vtable_size,
                    "vtable_size_raw": vtable_size_raw,
                    "vtable_numvfunc": vtable_numvfunc,
                    "functions_by_index": reference_functions,
                }
                if effective_class_name != class_name:
                    result["alias_class_name"] = effective_class_name
                return result

    return None


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
    """
    Compare compiler vtable layout dump against YAML references.

    Returns a structured report containing differences.
    """
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

    alias_used = reference.get("alias_class_name") if reference else None
    reference_mode = "merged" if merge_reference_modules else "single"
    reference_modules_merged = (
        list(reference.get("modules", []))
        if merge_reference_modules and reference
        else []
    )
    reference_files_merged = (
        list(reference.get("files", []))
        if merge_reference_modules and reference
        else []
    )
    reference_conflicts = (
        list(reference.get("conflicts", []))
        if merge_reference_modules and reference
        else []
    )

    report: Dict[str, Any] = {
        "class_name": class_name,
        "platform": platform,
        "requested_modules": list(reference_modules),
        "compiler_found": compiler_section is not None,
        "reference_found": reference is not None,
        "reference_module": reference.get("module") if reference else None,
        "reference_mode": reference_mode,
        "reference_modules_merged": reference_modules_merged,
        "reference_files_merged": reference_files_merged,
        "reference_conflicts": reference_conflicts,
        "differences": [],
        "notes": [],
    }

    if alias_used:
        report["alias_class_name"] = alias_used
        report["notes"].append(
            f"Reference YAML matched via alias symbol '{alias_used}' "
            f"(primary symbol '{class_name}' not found)."
        )

    if reference_conflicts:
        report["differences"].extend(reference_conflicts)

    compiler_missing = compiler_section is None
    if compiler_missing:
        report["notes"].append(
            f"No vtable section for class '{class_name}' found in compiler output."
        )
    else:
        compiler_entry_count = compiler_section["entry_count"]
        declared_entries = compiler_section["declared_entries"]
        methods_by_index = compiler_section["methods_by_index"]
        report["compiler_entry_count"] = compiler_entry_count
        report["compiler_declared_entries"] = declared_entries
        report["compiler_methods_by_index"] = methods_by_index

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
    report["reference_functions_by_index"] = reference_functions

    if compiler_missing:
        return report

    actual_size = compiler_entry_count * pointer_size
    report["compiler_vtable_size"] = actual_size

    if expected_size is not None and expected_size != actual_size:
        report["differences"].append(
            {
                "type": "vtable_size_mismatch",
                "message": (
                    f"vtable_size mismatch: YAML={hex(expected_size)} "
                    f"vs compiler={hex(actual_size)} (entry_count={compiler_entry_count}, "
                    f"ptr_size={pointer_size})."
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
            # "dtor" in YAML matches any destructor "~ClassName" from compiler
            if expected_member == "dtor" and actual_member.startswith("~"):
                continue
            report["differences"].append(
                {
                    "type": "vfunc_name_mismatch",
                    "message": (
                        f"Index {index} mismatch: YAML expects '{expected_member}' "
                        f"but compiler reports '{actual_member}'."
                    ),
                }
            )

    if not report["differences"]:
        report["notes"].append(
            "No differences detected for vtable_size/vtable_numvfunc/vfunc_index mapping."
        )

    return report


def format_vtable_compare_report(
    report: Dict[str, Any], *, include_differences: bool = True
) -> List[str]:
    """Format a comparison report into human-readable lines."""
    lines: List[str] = []
    lines.append(
        f"Class '{report['class_name']}' compare target platform: {report.get('platform', 'unknown')}"
    )

    compiler_found = report.get("compiler_found")
    if compiler_found:
        compiler_count = report.get("compiler_entry_count")
        compiler_declared = report.get("compiler_declared_entries")
        lines.append(
            f"Compiler vtable entries: parsed={compiler_count}, declared={compiler_declared}"
        )
    elif report.get("reference_mode") != "merged":
        lines.extend(report.get("notes", []))
        return lines

    if report.get("reference_mode") == "merged":
        lines.append("Reference mode: merged")
        merged_modules = report.get("reference_modules_merged", [])
        if merged_modules:
            lines.append(f"Reference modules: {', '.join(merged_modules)}")
        else:
            lines.append("Reference modules:")
        lines.append(
            f"Reference files merged: {len(report.get('reference_files_merged', []))}"
        )
        lines.append(
            f"Reference functions: {report.get('reference_functions_count', 0)}"
        )
        lines.append(
            "Reference conflicts found: "
            f"{len(report.get('reference_conflicts', []))}"
        )
    elif report.get("reference_found"):
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

    if include_differences:
        lines.extend(format_vtable_compare_differences(report))

    return lines


def format_vtable_compare_differences(report: Dict[str, Any]) -> List[str]:
    """Format the differences and notes portion of a comparison report."""
    lines: List[str] = []
    compiler_found = report.get("compiler_found")
    diffs = report.get("differences", [])
    if diffs:
        lines.append(f"Differences found: {len(diffs)}")
        for item in diffs:
            lines.append(f"- {item['message']}")
        if not compiler_found:
            for note in report.get("notes", []):
                lines.append(note)
    else:
        for note in report.get("notes", []):
            lines.append(note)
    return lines


def format_vtable_differences_for_agent(report: Dict[str, Any]) -> List[str]:
    """
    Format only the differences section in a console-like style for agent prompts.

    Example:
      Differences found: 2
      - Index 25 mismatch: ...
      - Index 27 mismatch: ...
    """
    diffs = report.get("differences", [])
    lines: List[str] = [f"Differences found: {len(diffs)}"]
    for item in diffs:
        lines.append(f"- {item['message']}")
    return lines


def format_compiler_vtable_entries(report: Dict[str, Any]) -> List[str]:
    """Format compiler vtable entries for debug output, one per line."""
    methods_by_index = report.get("compiler_methods_by_index", {})
    if not methods_by_index:
        return ["(no compiler vtable entries)"]
    lines: List[str] = []
    for index in sorted(methods_by_index.keys()):
        entry = methods_by_index[index]
        member_name = entry.get("member_name", "???")
        lines.append(f"[{index}] {member_name}")
    return lines


def format_reference_vtable_entries(report: Dict[str, Any]) -> List[str]:
    """Format YAML reference vtable entries for debug output, one per line."""
    functions_by_index = report.get("reference_functions_by_index", {})
    if not functions_by_index:
        return ["(no reference vtable entries)"]
    lines: List[str] = []
    for index in sorted(functions_by_index.keys()):
        entry = functions_by_index[index]
        member_name = entry.get("member_name", entry.get("func_name", "???"))
        lines.append(f"[{index}] {member_name}")
    return lines
