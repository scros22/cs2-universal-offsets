#!/usr/bin/env python3
"""Shared utility helpers for IDA analyze scripts."""

import json
import os
import re
import textwrap
from pathlib import Path

try:
    import yaml
except ImportError:
    yaml = None

try:
    from ida_llm_utils import (
        call_llm_text,
        create_openai_client,
        normalize_optional_temperature,
    )
except Exception:
    call_llm_text = None
    create_openai_client = None
    normalize_optional_temperature = None


# Combined vtable lookup + entry reading script for IDA py_eval.
# Merges logic from get-vtable-address/SKILL.md and write-vtable-as-yaml/SKILL.md.
# Uses CLASS_NAME_PLACEHOLDER for substitution (avoids brace-escaping issues).
# Returns JSON via the 'result' variable.
_VTABLE_PY_EVAL_TEMPLATE = r'''
import ida_bytes, ida_name, idaapi, idautils, ida_segment, json

class_name = CLASS_NAME_PLACEHOLDER
candidate_symbols = CANDIDATE_SYMBOLS_PLACEHOLDER
ptr_size = 8 if idaapi.inf_is_64bit() else 4

vtable_start = None
vtable_symbol = ""
is_linux = False

def _try_direct_symbol(symbol_name):
    global vtable_start, vtable_symbol, is_linux
    if not symbol_name:
        return False
    addr = ida_name.get_name_ea(idaapi.BADADDR, symbol_name)
    if addr == idaapi.BADADDR:
        return False
    if symbol_name.startswith("_ZTV"):
        vtable_start = addr + 0x10
        vtable_symbol = symbol_name + " + 0x10"
        is_linux = True
    else:
        vtable_start = addr
        vtable_symbol = symbol_name
        is_linux = False
    return True

for symbol_name in candidate_symbols:
    if _try_direct_symbol(symbol_name):
        break

# Direct symbol: Windows ??_7ClassName@@6B@
if vtable_start is None:
    win_name = "??_7" + class_name + "@@6B@"
    _try_direct_symbol(win_name)

# Direct symbol: Linux _ZTV<len>ClassName
if vtable_start is None:
    linux_name = "_ZTV" + str(len(class_name)) + class_name
    _try_direct_symbol(linux_name)

# RTTI fallback: Windows ??_R4ClassName@@6B@
if vtable_start is None:
    col_name = "??_R4" + class_name + "@@6B@"
    col_addr = ida_name.get_name_ea(idaapi.BADADDR, col_name)
    if col_addr != idaapi.BADADDR:
        is_linux = False
        rdata_seg = ida_segment.get_segm_by_name(".rdata")
        for ref in idautils.DataRefsTo(col_addr):
            if rdata_seg and not (rdata_seg.start_ea <= ref < rdata_seg.end_ea):
                continue
            vtable_start = ref + ptr_size
            sym = ida_name.get_name(vtable_start) or ("??_7" + class_name + "@@6B@")
            vtable_symbol = sym
            break

# RTTI fallback: Linux _ZTI<len>ClassName
if vtable_start is None:
    ti_name = "_ZTI" + str(len(class_name)) + class_name
    ti_addr = ida_name.get_name_ea(idaapi.BADADDR, ti_name)
    if ti_addr != idaapi.BADADDR:
        is_linux = True
        for ref in idautils.DataRefsTo(ti_addr):
            ott = ida_bytes.get_qword(ref - ptr_size) if ptr_size == 8 else ida_bytes.get_dword(ref - ptr_size)
            if ott == 0:
                vtable_start = ref + ptr_size
                ztv_addr = ref - ptr_size
                ztv_name = ida_name.get_name(ztv_addr) or ("_ZTV" + str(len(class_name)) + class_name)
                vtable_symbol = ztv_name + " + 0x10"
                break

if vtable_start is None:
    result = json.dumps(None)
else:
    vtable_seg = ida_segment.getseg(vtable_start)
    entries = {}
    count = 0
    for i in range(1000):
        ea = vtable_start + i * ptr_size
        if is_linux and i > 0:
            name = ida_name.get_name(ea)
            if name and (name.startswith("_ZTV") or name.startswith("_ZTI")):
                break
        ptr_value = ida_bytes.get_qword(ea) if ptr_size == 8 else ida_bytes.get_dword(ea)
        if ptr_value == 0:
            if is_linux:
                entries[count] = hex(ptr_value)
                count += 1
                continue
            else:
                break
        if ptr_value == 0xFFFFFFFFFFFFFFFF:
            break
        target_seg = ida_segment.getseg(ptr_value)
        if not target_seg:
            break
        # If an entry points back into the vtable's own segment (.rdata/.rodata),
        # it is metadata or unrelated data, not a virtual function.
        if vtable_seg and (vtable_seg.start_ea <= ptr_value < vtable_seg.end_ea):
            break
        if not (target_seg.perm & ida_segment.SEGPERM_EXEC):
            break
        func = idaapi.get_func(ptr_value)
        if func is not None:
            entries[count] = hex(ptr_value)
            count += 1
            continue
        flags = ida_bytes.get_full_flags(ptr_value)
        if ida_bytes.is_code(flags):
            entries[count] = hex(ptr_value)
            count += 1
            continue
        break

    size_in_bytes = count * ptr_size
    result = json.dumps({
        "vtable_class": class_name,
        "vtable_symbol": vtable_symbol,
        "vtable_va": hex(vtable_start),
        "vtable_size": hex(size_in_bytes),
        "vtable_numvfunc": count,
        "vtable_entries": entries
    })
'''


def parse_mcp_result(result):
    """Parse CallToolResult content to a Python object."""
    if result.content:
        text = result.content[0].text
        try:
            return json.loads(text)
        except (json.JSONDecodeError, TypeError):
            return text
    return None


def _normalize_mangled_class_names(mangled_class_names, debug=False):
    if mangled_class_names is None:
        return {}
    if not isinstance(mangled_class_names, dict):
        if debug:
            print(
                "    Preprocess: mangled_class_names must be a dict, got "
                f"{type(mangled_class_names).__name__}"
            )
        return None

    normalized = {}
    for class_name, aliases in mangled_class_names.items():
        if not isinstance(class_name, str) or not class_name:
            if debug:
                print(
                    "    Preprocess: invalid mangled_class_names key: "
                    f"{class_name!r}"
                )
            return None
        if not isinstance(aliases, (list, tuple)):
            if debug:
                print(
                    "    Preprocess: aliases for "
                    f"{class_name} must be a list/tuple"
                )
            return None

        normalized_aliases = []
        for alias in aliases:
            if not isinstance(alias, str) or not alias:
                if debug:
                    print(
                        "    Preprocess: invalid alias for "
                        f"{class_name}: {alias!r}"
                    )
                return None
            normalized_aliases.append(alias)

        normalized[class_name] = normalized_aliases

    return normalized


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
        for field_name in desired_fields:
            if not isinstance(field_name, str) or not field_name:
                if debug:
                    print(f"    Preprocess: invalid desired field list for {symbol_name}")
                return None

            if field_name == "vfunc_sig_max_match":
                if debug:
                    print(
                        f"    Preprocess: bare vfunc_sig_max_match field is "
                        f"not allowed for {symbol_name}"
                    )
                return None

            if field_name.startswith("vfunc_sig_max_match:"):
                if "vfunc_sig_max_match" in generation_options:
                    if debug:
                        print(
                            f"    Preprocess: duplicated vfunc_sig_max_match "
                            f"directive for {symbol_name}"
                        )
                    return None
                max_match_text = field_name.split(":", 1)[1]
                try:
                    max_match = int(max_match_text)
                except ValueError:
                    if debug:
                        print(
                            f"    Preprocess: invalid vfunc_sig_max_match "
                            f"value for {symbol_name}: {max_match_text}"
                        )
                    return None
                if max_match <= 0:
                    if debug:
                        print(
                            f"    Preprocess: invalid vfunc_sig_max_match "
                            f"value for {symbol_name}: {max_match_text}"
                        )
                    return None
                desired_output_fields.append("vfunc_sig_max_match")
                generation_options["vfunc_sig_max_match"] = max_match
                continue

            desired_output_fields.append(field_name)

        if "vfunc_sig_max_match" in generation_options and "vfunc_sig" not in desired_output_fields:
            if debug:
                print(
                    f"    Preprocess: vfunc_sig_max_match requires vfunc_sig "
                    f"for {symbol_name}"
                )
            return None
        normalized[symbol_name] = {
            "desired_output_fields": desired_output_fields,
            "generation_options": generation_options,
        }

    return normalized


def _build_target_kind_map(
    func_names,
    gv_names,
    patch_names,
    struct_member_names,
    vtable_class_names,
    inherit_vfuncs,
    func_xrefs_map,
    debug=False,
):
    target_kind_map = {}

    def _register(symbol_name, target_kind):
        existing_kind = target_kind_map.get(symbol_name)
        if existing_kind is not None and existing_kind != target_kind:
            if debug:
                print(
                    f"    Preprocess: symbol kind conflict for {symbol_name}: "
                    f"{existing_kind} vs {target_kind}"
                )
            return False
        target_kind_map[symbol_name] = target_kind
        return True

    for func_name in list(func_names) + list(func_xrefs_map):
        if not _register(func_name, "func"):
            return None
    for inherit_spec in inherit_vfuncs:
        if not _register(inherit_spec[0], "func"):
            return None
    for gv_name in gv_names:
        if not _register(gv_name, "gv"):
            return None
    for patch_name in patch_names:
        if not _register(patch_name, "patch"):
            return None
    for struct_member_name in struct_member_names:
        if not _register(struct_member_name, "struct_member"):
            return None
    for class_name in vtable_class_names:
        if not _register(class_name, "vtable"):
            return None

    return target_kind_map


def _get_mangled_class_aliases(mangled_class_names, class_name):
    aliases = (mangled_class_names or {}).get(class_name, [])
    if not aliases:
        return None
    return list(aliases)


def build_remote_text_export_py_eval(
    *,
    output_path,
    producer_code,
    content_var="payload_text",
    format_name="text",
):
    """Build a py_eval script that writes large text to disk and returns a small ack."""
    output_path_str = os.fspath(output_path)
    if not os.path.isabs(output_path_str):
        raise ValueError(f"output_path must be absolute, got {output_path_str!r}")
    if not str(producer_code).strip():
        raise ValueError("producer_code cannot be empty")
    if not str(content_var).strip():
        raise ValueError("content_var cannot be empty")

    producer_block = textwrap.indent(str(producer_code).rstrip(), "    ")
    return (
        "import json, os, traceback\n"
        f"output_path = {output_path_str!r}\n"
        f"format_name = {str(format_name)!r}\n"
        "tmp_path = output_path + '.tmp'\n"
        "def _truncate_text(value, limit=800):\n"
        "    text = '' if value is None else str(value)\n"
        "    return text if len(text) <= limit else text[:limit] + ' [truncated]'\n"
        "try:\n"
        "    if not os.path.isabs(output_path):\n"
        "        raise ValueError(f'output_path must be absolute: {output_path}')\n"
        f"{producer_block}\n"
        f"    payload_text = str({content_var})\n"
        "    parent_dir = os.path.dirname(output_path)\n"
        "    if parent_dir:\n"
        "        os.makedirs(parent_dir, exist_ok=True)\n"
        "    with open(tmp_path, 'w', encoding='utf-8') as handle:\n"
        "        handle.write(payload_text)\n"
        "    os.replace(tmp_path, output_path)\n"
        "    result = json.dumps({\n"
        "        'ok': True,\n"
        "        'output_path': output_path,\n"
        "        'bytes_written': len(payload_text.encode('utf-8')),\n"
        "        'format': format_name,\n"
        "    })\n"
        "except Exception as exc:\n"
        "    try:\n"
        "        if os.path.exists(tmp_path):\n"
        "            os.unlink(tmp_path)\n"
        "    except Exception:\n"
        "        pass\n"
        "    result = json.dumps({\n"
        "        'ok': False,\n"
        "        'output_path': output_path,\n"
        "        'error': _truncate_text(exc),\n"
        "        'traceback': _truncate_text(traceback.format_exc()),\n"
        "    })\n"
    )


def _build_vtable_py_eval(class_name, symbol_aliases=None):
    """Build the vtable py_eval script for the given class name."""
    return (
        _VTABLE_PY_EVAL_TEMPLATE
        .replace("CLASS_NAME_PLACEHOLDER", json.dumps(class_name))
        .replace(
            "CANDIDATE_SYMBOLS_PLACEHOLDER",
            json.dumps(list(symbol_aliases or [])),
        )
    )


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
GV_YAML_ORDER = [
    "gv_name",
    "gv_va",
    "gv_rva",
    "gv_sig",
    "gv_sig_va",
    "gv_inst_offset",
    "gv_inst_length",
    "gv_inst_disp",
]
VTABLE_YAML_ORDER = [
    "vtable_class",
    "vtable_symbol",
    "vtable_va",
    "vtable_rva",
    "vtable_size",
    "vtable_numvfunc",
    "vtable_entries",
]
PATCH_YAML_ORDER = ["patch_name", "patch_sig", "patch_bytes"]
STRUCT_MEMBER_YAML_ORDER = [
    "struct_name",
    "member_name",
    "offset",
    "size",
    "offset_sig",
    "offset_sig_disp",
]
TARGET_KIND_TO_FIELD_ORDER = {
    "func": FUNC_YAML_ORDER,
    "gv": GV_YAML_ORDER,
    "vtable": VTABLE_YAML_ORDER,
    "patch": PATCH_YAML_ORDER,
    "struct_member": STRUCT_MEMBER_YAML_ORDER,
}
TARGET_KIND_TO_FIELD_SET = {
    kind: set(field_order)
    for kind, field_order in TARGET_KIND_TO_FIELD_ORDER.items()
}


def _build_ordered_yaml_payload(data, ordered_keys):
    payload = {}
    for key in ordered_keys:
        if key not in data:
            continue
        value = data[key]
        if key == "vtable_entries":
            normalized_entries = {
                int(entry_index): str(entry_value)
                for entry_index, entry_value in value.items()
            }
            payload[key] = dict(sorted(normalized_entries.items()))
            continue
        if key.endswith("_va") or key.endswith("_rva") or key.endswith("_size"):
            payload[key] = str(value)
            continue
        payload[key] = value
    return payload


def _assemble_symbol_payload(symbol_name, target_kind, candidate_data, desired_fields_map, debug=False):
    desired_field_spec = desired_fields_map.get(symbol_name)
    if desired_field_spec is None:
        if debug:
            print(f"    Preprocess: missing desired-fields entry for {symbol_name}")
        return None
    desired_fields = desired_field_spec["desired_output_fields"]

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


def write_vtable_yaml(path, data):
    """Write vtable YAML matching the format produced by write-vtable-as-yaml skill."""
    if yaml is None:
        raise RuntimeError("PyYAML is required to write vtable YAML")

    payload = _build_ordered_yaml_payload(data, VTABLE_YAML_ORDER)

    with open(path, "w", encoding="utf-8") as f:
        yaml.safe_dump(
            payload,
            f,
            sort_keys=False,
            default_flow_style=False,
            allow_unicode=False,
        )

def write_func_yaml(path, data):
    """Write function/vfunc YAML with the same key set and key order as write-func-as-yaml; scalar quoting/styling is handled by PyYAML."""
    if yaml is None:
        raise RuntimeError("PyYAML is required to write function YAML")

    payload = _build_ordered_yaml_payload(data, FUNC_YAML_ORDER)

    with open(path, "w", encoding="utf-8") as f:
        yaml.safe_dump(
            payload,
            f,
            sort_keys=False,
            default_flow_style=False,
            allow_unicode=False,
        )

def write_gv_yaml(path, data):
    """Write global-variable YAML with the same key set and key order as write-globalvar-as-yaml; scalar quoting/styling is handled by PyYAML."""
    if yaml is None:
        raise RuntimeError("PyYAML is required to write global-variable YAML")

    payload = _build_ordered_yaml_payload(data, GV_YAML_ORDER)

    with open(path, "w", encoding="utf-8") as f:
        yaml.safe_dump(
            payload,
            f,
            sort_keys=False,
            default_flow_style=False,
            allow_unicode=False,
        )




def write_patch_yaml(path, data):
    """Write patch YAML with stable key order."""
    if yaml is None:
        raise RuntimeError("PyYAML is required to write patch YAML")

    payload = _build_ordered_yaml_payload(data, PATCH_YAML_ORDER)

    with open(path, "w", encoding="utf-8") as f:
        yaml.safe_dump(
            payload,
            f,
            sort_keys=False,
            default_flow_style=False,
            allow_unicode=False,
        )

def write_struct_offset_yaml(path, data):
    """Write struct-member offset YAML matching write-structoffset-as-yaml key order."""
    if yaml is None:
        raise RuntimeError("PyYAML is required to write struct offset YAML")

    payload = _build_ordered_yaml_payload(data, STRUCT_MEMBER_YAML_ORDER)

    with open(path, "w", encoding="utf-8") as f:
        yaml.safe_dump(
            payload,
            f,
            sort_keys=False,
            default_flow_style=False,
            allow_unicode=False,
        )


def _empty_llm_decompile_result():
    return {
        "found_vcall": [],
        "found_call": [],
        "found_funcptr": [],
        "found_gv": [],
        "found_struct_offset": [],
    }


def _get_preprocessor_scripts_dir():
    return Path(__file__).resolve().parent / "ida_preprocessor_scripts"


def _resolve_llm_decompile_template_value(value, platform):
    resolved = str(value or "")
    platform_text = str(platform or "").strip()
    if platform_text:
        resolved = resolved.replace("{platform}", platform_text)
    return resolved


def _debug_print_multiline(label, text, debug=False):
    if not debug:
        return
    print(f"    Preprocess: {label} BEGIN")
    print(str(text or "<empty>"))
    print(f"    Preprocess: {label} END")


def _debug_print_json(label, value, debug=False):
    if not debug:
        return
    try:
        rendered = json.dumps(value, indent=2, ensure_ascii=False, sort_keys=False)
    except Exception:
        rendered = repr(value)
    _debug_print_multiline(label, rendered, debug=True)


def _build_struct_member_symbol_name(struct_name, member_name):
    struct_name_text = str(struct_name or "").strip()
    member_name_text = str(member_name or "").strip()
    if not struct_name_text or not member_name_text:
        return None
    return f"{struct_name_text}_{member_name_text}"


def _load_struct_member_metadata_from_yaml(old_path):
    if yaml is None or not old_path or not os.path.exists(old_path):
        return {}

    try:
        with open(old_path, "r", encoding="utf-8") as f:
            old_data = yaml.safe_load(f)
    except Exception:
        return {}

    if not isinstance(old_data, dict):
        return {}

    metadata = {}
    struct_name = str(old_data.get("struct_name", "") or "").strip()
    member_name = str(old_data.get("member_name", "") or "").strip()
    if struct_name and member_name:
        metadata["struct_name"] = struct_name
        metadata["member_name"] = member_name

    raw_size = old_data.get("size")
    if raw_size is not None:
        try:
            size_value = _parse_int_value(raw_size)
        except Exception:
            size_value = None
        if isinstance(size_value, int) and size_value > 0:
            metadata["size"] = size_value

    return metadata


def _build_llm_decompile_specs_map(llm_decompile_specs, debug=False):
    specs_map = {}
    for spec in llm_decompile_specs or []:
        if not isinstance(spec, (tuple, list)) or len(spec) != 3:
            if debug:
                print(f"    Preprocess: invalid llm_decompile spec: {spec}")
            return None

        func_name, prompt_path, reference_yaml_path = spec
        if not isinstance(func_name, str) or not func_name:
            if debug:
                print(f"    Preprocess: invalid llm_decompile target: {func_name}")
            return None
        if not isinstance(prompt_path, str) or not prompt_path:
            if debug:
                print(
                    "    Preprocess: invalid llm_decompile prompt path for "
                    f"{func_name}: {prompt_path!r}"
                )
            return None
        if not isinstance(reference_yaml_path, str) or not reference_yaml_path:
            if debug:
                print(
                    "    Preprocess: invalid llm_decompile reference path for "
                    f"{func_name}: {reference_yaml_path!r}"
                )
            return None
        if func_name in specs_map:
            if debug:
                print(f"    Preprocess: duplicated llm_decompile target: {func_name}")
            return None

        specs_map[func_name] = {
            "prompt_path": prompt_path,
            "reference_yaml_path": reference_yaml_path,
        }

    return specs_map


def _parse_yaml_mapping(text):
    if yaml is None:
        return None
    try:
        parsed = yaml.load(text, Loader=yaml.BaseLoader)
    except yaml.YAMLError:
        return None
    if parsed is None:
        return {}
    if isinstance(parsed, dict):
        return parsed
    return None


def _normalize_llm_entries(entries, required_keys):
    if isinstance(entries, (str, bytes, bytearray)) or not isinstance(entries, (list, tuple)):
        return []

    normalized = []
    for entry in entries:
        if not isinstance(entry, dict):
            continue
        item = {}
        valid = True
        for key in required_keys:
            value = str(entry.get(key, "")).strip()
            if not value:
                valid = False
                break
            item[key] = value
        if valid:
            normalized.append(item)
    return normalized


def _normalize_llm_struct_offset_entries(entries):
    if isinstance(entries, (str, bytes, bytearray)) or not isinstance(entries, (list, tuple)):
        return []

    normalized = []
    for entry in entries:
        if not isinstance(entry, dict):
            continue
        item = {}
        valid = True
        for key in ("insn_va", "insn_disasm", "offset", "struct_name", "member_name"):
            value = str(entry.get(key, "")).strip()
            if not value:
                valid = False
                break
            item[key] = value
        if not valid:
            continue

        size_value = entry.get("size")
        if size_value is not None:
            size_text = str(size_value).strip()
            if size_text:
                item["size"] = size_text

        normalized.append(item)
    return normalized


def _extract_slot_only_vfunc_candidates_from_llm_result(
    func_name,
    llm_result,
    debug=False,
):
    normalized_func_name = str(func_name or "").strip()
    if not normalized_func_name:
        return None

    normalized_offsets = []
    seen_offsets = set()
    candidate_inst_vas = []
    seen_inst_vas = set()
    for entry in (llm_result or {}).get("found_vcall", []):
        if not isinstance(entry, dict):
            continue
        if str(entry.get("func_name", "")).strip() != normalized_func_name:
            continue

        raw_offset = entry.get("vfunc_offset")
        try:
            offset_value = _parse_int_value(raw_offset)
        except Exception:
            continue

        if offset_value < 0 or offset_value % 8 != 0:
            if debug:
                print(
                    f"    Preprocess: invalid slot-only vfunc offset for "
                    f"{normalized_func_name}: {raw_offset}"
                )
            return None

        if offset_value in seen_offsets:
            pass
        else:
            seen_offsets.add(offset_value)
            normalized_offsets.append(offset_value)

        raw_insn_va = entry.get("insn_va")
        try:
            insn_va_value = _parse_int_value(raw_insn_va)
        except Exception:
            continue
        if insn_va_value < 0 or insn_va_value in seen_inst_vas:
            continue
        seen_inst_vas.add(insn_va_value)
        candidate_inst_vas.append(hex(insn_va_value))

    if not normalized_offsets:
        return None

    if len(normalized_offsets) != 1:
        if debug:
            rendered_offsets = ", ".join(hex(value) for value in normalized_offsets)
            print(
                f"    Preprocess: ambiguous slot-only vfunc offsets for "
                f"{normalized_func_name}: {rendered_offsets}"
            )
        return None

    resolved_offset = normalized_offsets[0]
    return {
        "func_name": normalized_func_name,
        "vfunc_offset": hex(resolved_offset),
        "vfunc_index": resolved_offset // 8,
        "candidate_inst_vas": candidate_inst_vas,
    }


def _build_slot_only_vfunc_payload_from_llm_result(
    func_name,
    llm_result,
    *,
    vtable_name=None,
    debug=False,
):
    slot_only_info = _extract_slot_only_vfunc_candidates_from_llm_result(
        func_name,
        llm_result,
        debug=debug,
    )
    if slot_only_info is None:
        return None

    payload = {
        "func_name": slot_only_info["func_name"],
        "vfunc_offset": slot_only_info["vfunc_offset"],
        "vfunc_index": slot_only_info["vfunc_index"],
    }
    if vtable_name:
        payload["vtable_name"] = str(vtable_name).strip()

    if debug:
        print(
            f"    Preprocess: using slot-only fallback for "
            f"{slot_only_info['func_name']} at offset {slot_only_info['vfunc_offset']}"
        )

    return payload


async def _build_enriched_slot_only_vfunc_payload_via_mcp(
    session,
    func_name,
    llm_result,
    *,
    vtable_name=None,
    vfunc_sig_max_match=1,
    require_vfunc_sig=False,
    require_vtable_name=False,
    debug=False,
):
    slot_only_info = _extract_slot_only_vfunc_candidates_from_llm_result(
        func_name,
        llm_result,
        debug=debug,
    )
    if slot_only_info is None:
        return None

    payload = {
        "func_name": slot_only_info["func_name"],
        "vfunc_offset": slot_only_info["vfunc_offset"],
        "vfunc_index": slot_only_info["vfunc_index"],
    }

    normalized_vtable_name = str(vtable_name or "").strip()
    if normalized_vtable_name:
        payload["vtable_name"] = normalized_vtable_name
    elif require_vtable_name or require_vfunc_sig:
        if debug:
            print(
                f"    Preprocess: slot-only fallback missing vtable_name for "
                f"{slot_only_info['func_name']}"
            )
        return None

    if debug:
        print(
            f"    Preprocess: using slot-only fallback for "
            f"{slot_only_info['func_name']} at offset {slot_only_info['vfunc_offset']}"
        )

    if not require_vfunc_sig:
        return payload

    candidate_inst_vas = slot_only_info.get("candidate_inst_vas", [])
    if not candidate_inst_vas:
        if debug:
            print(
                f"    Preprocess: no slot-only instruction candidates for "
                f"{slot_only_info['func_name']}"
            )
        return None

    for inst_va in candidate_inst_vas:
        sig_data = await preprocess_gen_vfunc_sig_via_mcp(
            session=session,
            inst_va=inst_va,
            vfunc_offset=slot_only_info["vfunc_offset"],
            max_match_count=vfunc_sig_max_match,
            debug=debug,
        )
        if not isinstance(sig_data, dict):
            continue
        vfunc_sig = sig_data.get("vfunc_sig")
        if not vfunc_sig:
            continue
        payload["vfunc_sig"] = str(vfunc_sig)
        payload["vfunc_sig_max_match"] = int(
            sig_data.get("vfunc_sig_max_match", vfunc_sig_max_match)
        )
        if sig_data.get("vfunc_sig_disp") not in (None, 0, "0", "0x0"):
            payload["vfunc_sig_disp"] = sig_data["vfunc_sig_disp"]
        return payload

    if debug:
        print(
            f"    Preprocess: failed to generate slot-only vfunc_sig for "
            f"{slot_only_info['func_name']}"
        )
    return None


def _load_symbol_lookup_candidates(symbol_name, debug=False):
    normalized_name = str(symbol_name or "").strip()
    if not normalized_name:
        return []

    candidates = []
    seen = set()

    def _append_candidate(raw_value):
        text = str(raw_value or "").strip()
        if not text or text in seen:
            return
        seen.add(text)
        candidates.append(text)

    _append_candidate(normalized_name)

    config_path = Path(__file__).resolve().parent / "config.yaml"
    if yaml is None or not config_path.is_file():
        return candidates

    try:
        config_data = yaml.safe_load(config_path.read_text(encoding="utf-8")) or {}
    except Exception as exc:
        if debug:
            print(
                f"    Preprocess: failed to read config aliases for "
                f"{normalized_name}: {exc}"
            )
        return candidates

    modules = config_data.get("modules")
    if not isinstance(modules, list):
        return candidates

    for module_entry in modules:
        if not isinstance(module_entry, dict):
            continue
        symbols = module_entry.get("symbols")
        if not isinstance(symbols, list):
            continue
        for symbol_entry in symbols:
            if not isinstance(symbol_entry, dict):
                continue
            if str(symbol_entry.get("name", "")).strip() != normalized_name:
                continue
            _append_candidate(symbol_entry.get("name"))
            raw_aliases = symbol_entry.get("alias")
            if isinstance(raw_aliases, (list, tuple)):
                for alias in raw_aliases:
                    _append_candidate(alias)
            elif raw_aliases is not None:
                _append_candidate(raw_aliases)

    if debug:
        print(
            f"    Preprocess: symbol lookup candidates for {normalized_name}: "
            f"{', '.join(candidates) if candidates else '<none>'}"
        )

    return candidates


def _parse_py_eval_json_result(eval_result, debug=False, context="py_eval"):
    try:
        parsed = parse_mcp_result(eval_result)
    except Exception as exc:
        if debug:
            print(f"    Preprocess: failed to parse {context} payload: {exc}")
        return None

    if not isinstance(parsed, dict):
        return None

    stderr_text = str(parsed.get("stderr", "")).strip()
    if stderr_text and debug:
        print(f"    Preprocess: {context} stderr:")
        print(stderr_text)

    result_text = parsed.get("result", "")
    if not result_text:
        return None

    try:
        return json.loads(result_text)
    except (json.JSONDecodeError, TypeError) as exc:
        if debug:
            print(f"    Preprocess: invalid {context} JSON payload: {exc}")
        return None


async def _find_function_addr_by_names_via_mcp(session, candidate_names, debug=False):
    ordered_candidates = []
    seen_candidates = set()
    for raw_name in candidate_names or []:
        text = str(raw_name or "").strip()
        if not text or text in seen_candidates:
            continue
        seen_candidates.add(text)
        ordered_candidates.append(text)

    if not ordered_candidates:
        return None

    py_code = (
        "import ida_funcs, ida_name, idaapi, json\n"
        f"candidate_names = {json.dumps(ordered_candidates)}\n"
        "matches = []\n"
        "seen_addrs = set()\n"
        "for candidate_name in candidate_names:\n"
        "    ea = ida_name.get_name_ea(idaapi.BADADDR, candidate_name)\n"
        "    if ea == idaapi.BADADDR:\n"
        "        continue\n"
        "    func = ida_funcs.get_func(ea)\n"
        "    if func is None:\n"
        "        continue\n"
        "    func_start = int(func.start_ea)\n"
        "    func_va = hex(func_start)\n"
        "    if func_va in seen_addrs:\n"
        "        continue\n"
        "    seen_addrs.add(func_va)\n"
        "    matches.append({'name': candidate_name, 'func_va': func_va})\n"
        "result = json.dumps(matches)\n"
    )

    try:
        eval_result = await session.call_tool(
            name="py_eval",
            arguments={"code": py_code},
        )
    except Exception as exc:
        if debug:
            print(f"    Preprocess: py_eval error while resolving function addr: {exc}")
        return None

    match_payload = _parse_py_eval_json_result(
        eval_result,
        debug=debug,
        context="llm_decompile function lookup",
    )
    if not isinstance(match_payload, list):
        return None

    resolved_matches = []
    seen_func_vas = set()
    for item in match_payload:
        if not isinstance(item, dict):
            continue
        func_va = str(item.get("func_va", "")).strip()
        if not func_va:
            continue
        try:
            int(func_va, 0)
        except (TypeError, ValueError):
            continue
        if func_va in seen_func_vas:
            continue
        seen_func_vas.add(func_va)
        resolved_matches.append(func_va)

    if len(resolved_matches) != 1:
        if debug:
            print(
                "    Preprocess: llm_decompile function lookup returned "
                f"{len(resolved_matches)} matches: {resolved_matches}"
            )
        return None

    return resolved_matches[0]


async def _export_function_detail_via_mcp(session, func_name, func_va, debug=False):
    try:
        func_va_int = _parse_int_value(func_va)
    except Exception:
        return None

    py_code = (
        "import ida_funcs, ida_lines, ida_segment, idautils, idc, json\n"
        "try:\n"
        "    import ida_hexrays\n"
        "except Exception:\n"
        "    ida_hexrays = None\n"
        f"func_ea = {func_va_int}\n"
        "def get_disasm(start_ea):\n"
        "    func = ida_funcs.get_func(start_ea)\n"
        "    if func is None:\n"
        "        return ''\n"
        "    lines = []\n"
        "    for ea in idautils.FuncItems(func.start_ea):\n"
        "        if ea < func.start_ea or ea >= func.end_ea:\n"
        "            continue\n"
        "        seg = ida_segment.getseg(ea)\n"
        "        seg_name = ida_segment.get_segm_name(seg) if seg else ''\n"
        "        address_text = f'{seg_name}:{ea:016X}' if seg_name else f'{ea:016X}'\n"
        "        disasm_line = idc.generate_disasm_line(ea, 0) or ''\n"
        "        lines.append(f\"{address_text}                 {ida_lines.tag_remove(disasm_line)}\")\n"
        "    return '\\n'.join(lines)\n"
        "def get_pseudocode(start_ea):\n"
        "    if ida_hexrays is None:\n"
        "        return ''\n"
        "    try:\n"
        "        if not ida_hexrays.init_hexrays_plugin():\n"
        "            return ''\n"
        "        cfunc = ida_hexrays.decompile(start_ea)\n"
        "    except Exception:\n"
        "        return ''\n"
        "    if not cfunc:\n"
        "        return ''\n"
        "    return '\\n'.join(ida_lines.tag_remove(line.line) for line in cfunc.get_pseudocode())\n"
        "func = ida_funcs.get_func(func_ea)\n"
        "if func is None:\n"
        "    raise ValueError(f'Function not found: {hex(func_ea)}')\n"
        "func_start = int(func.start_ea)\n"
        "result = json.dumps({\n"
        "    'func_name': ida_funcs.get_func_name(func_start) or f'sub_{func_start:X}',\n"
        "    'func_va': hex(func_start),\n"
        "    'disasm_code': get_disasm(func_start),\n"
        "    'procedure': get_pseudocode(func_start),\n"
        "})\n"
    )

    try:
        eval_result = await session.call_tool(
            name="py_eval",
            arguments={"code": py_code},
        )
    except Exception as exc:
        if debug:
            print(
                f"    Preprocess: py_eval error while exporting function detail "
                f"for {func_name}: {exc}"
            )
        return None

    detail_payload = _parse_py_eval_json_result(
        eval_result,
        debug=debug,
        context=f"llm_decompile function export for {func_name}",
    )
    if not isinstance(detail_payload, dict):
        return None

    resolved_func_va = str(detail_payload.get("func_va", "")).strip()
    disasm_code = str(detail_payload.get("disasm_code", "") or "").strip()
    procedure = detail_payload.get("procedure", "")
    if not resolved_func_va or not disasm_code:
        return None
    try:
        int(resolved_func_va, 0)
    except (TypeError, ValueError):
        return None

    if procedure is None:
        procedure = ""
    elif not isinstance(procedure, str):
        return None

    return {
        "func_name": str(func_name or "").strip() or str(detail_payload.get("func_name", "")).strip(),
        "func_va": resolved_func_va,
        "disasm_code": disasm_code,
        "procedure": procedure,
    }


async def _resolve_direct_call_target_via_mcp(session, insn_va, debug=False):
    try:
        insn_va_int = _parse_int_value(insn_va)
    except Exception:
        return None

    py_code = (
        "import ida_funcs, idautils, json\n"
        f"insn_ea = {insn_va_int}\n"
        "matches = []\n"
        "seen_addrs = set()\n"
        "for target_ea in idautils.CodeRefsFrom(insn_ea, False):\n"
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
                "    Preprocess: py_eval error while resolving direct call target: "
                f"{exc}"
            )
        return None

    match_payload = _parse_py_eval_json_result(
        eval_result,
        debug=debug,
        context="llm_decompile direct call target lookup",
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
                "    Preprocess: llm_decompile direct call target lookup returned "
                f"{len(resolved_matches)} matches: {resolved_matches}"
            )
        return None

    return resolved_matches[0]


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


async def _resolve_direct_gv_target_via_mcp(session, insn_va, debug=False):
    try:
        insn_va_int = _parse_int_value(insn_va)
    except Exception:
        return None

    py_code = (
        "import idautils, json\n"
        f"insn_ea = {insn_va_int}\n"
        "matches = []\n"
        "seen_addrs = set()\n"
        "for target_ea in idautils.DataRefsFrom(insn_ea):\n"
        "    gv_va = hex(int(target_ea))\n"
        "    if gv_va in seen_addrs:\n"
        "        continue\n"
        "    seen_addrs.add(gv_va)\n"
        "    matches.append({'gv_va': gv_va})\n"
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
                "    Preprocess: py_eval error while resolving direct gv target: "
                f"{exc}"
            )
        return None

    match_payload = _parse_py_eval_json_result(
        eval_result,
        debug=debug,
        context="llm_decompile direct gv target lookup",
    )
    if not isinstance(match_payload, list):
        return None

    resolved_matches = []
    seen_gv_vas = set()
    for item in match_payload:
        if not isinstance(item, dict):
            continue
        gv_va = str(item.get("gv_va", "")).strip()
        if not gv_va or gv_va in seen_gv_vas:
            continue
        try:
            int(gv_va, 0)
        except (TypeError, ValueError):
            continue
        seen_gv_vas.add(gv_va)
        resolved_matches.append(gv_va)

    if len(resolved_matches) != 1:
        if debug:
            print(
                "    Preprocess: llm_decompile direct gv target lookup returned "
                f"{len(resolved_matches)} matches: {resolved_matches}"
            )
        return None

    return resolved_matches[0]


async def _load_llm_decompile_target_detail_via_mcp(session, target_func_name, debug=False):
    normalized_target_name = str(target_func_name or "").strip()
    if not normalized_target_name:
        if debug:
            print("    Preprocess: llm_decompile target func_name missing in reference YAML")
        return None

    candidate_names = _load_symbol_lookup_candidates(
        normalized_target_name,
        debug=debug,
    )
    func_va = await _find_function_addr_by_names_via_mcp(
        session,
        candidate_names,
        debug=debug,
    )
    if func_va is None:
        if debug:
            print(
                f"    Preprocess: failed to resolve llm_decompile target "
                f"function address for {normalized_target_name}"
            )
        return None

    detail_payload = await _export_function_detail_via_mcp(
        session,
        normalized_target_name,
        func_va,
        debug=debug,
    )
    if detail_payload is None and debug:
        print(
            f"    Preprocess: failed to export llm_decompile target detail "
            f"for {normalized_target_name}"
        )
    return detail_payload


def parse_llm_decompile_response(response_text):
    response_text = str(response_text or "").strip()
    if not response_text:
        return _empty_llm_decompile_result()

    candidates = []
    for match in re.finditer(
        r"```(?:yaml|yml)[ \t]*\n?(.*?)```",
        response_text,
        re.IGNORECASE | re.DOTALL,
    ):
        candidates.append(match.group(1).strip())
    if not candidates:
        for match in re.finditer(r"```[ \t]*\n(.*?)```", response_text, re.DOTALL):
            candidates.append(match.group(1).strip())
    if not candidates:
        candidates.append(response_text)

    parsed = None
    for candidate in candidates:
        if not candidate:
            continue
        parsed = _parse_yaml_mapping(candidate)
        if parsed is not None:
            break

    if not isinstance(parsed, dict):
        return _empty_llm_decompile_result()

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


def _prepare_llm_decompile_request(
    func_name,
    llm_decompile_specs_map,
    llm_config,
    platform=None,
    debug=False,
):
    llm_spec = (llm_decompile_specs_map or {}).get(func_name)
    if llm_spec is None:
        return None

    if yaml is None:
        if debug:
            print("    Preprocess: PyYAML is required for llm_decompile fallback")
        return None

    if not isinstance(llm_config, dict):
        if debug:
            print(f"    Preprocess: llm_config missing or invalid for {func_name}")
        return None

    model = str(llm_config.get("model", "")).strip()
    if not model:
        if debug:
            print(f"    Preprocess: llm_config.model missing for {func_name}")
        return None

    temperature = llm_config.get("temperature")
    if temperature is not None:
        if callable(normalize_optional_temperature):
            try:
                temperature = normalize_optional_temperature(
                    temperature,
                    "llm_config.temperature",
                )
            except ValueError as exc:
                if debug:
                    print(
                        f"    Preprocess: invalid llm_decompile temperature for "
                        f"{func_name}: {exc}"
                    )
                return None
        else:
            try:
                temperature = float(temperature)
            except (TypeError, ValueError):
                if debug:
                    print(
                        f"    Preprocess: invalid llm_decompile temperature for "
                        f"{func_name}: {temperature!r}"
                    )
                return None

    if not callable(create_openai_client):
        if debug:
            print(f"    Preprocess: create_openai_client unavailable for {func_name}")
        return None

    scripts_dir = _get_preprocessor_scripts_dir()
    prompt_path = Path(
        _resolve_llm_decompile_template_value(
            llm_spec["prompt_path"],
            platform,
        )
    )
    reference_yaml_path = Path(
        _resolve_llm_decompile_template_value(
            llm_spec["reference_yaml_path"],
            platform,
        )
    )
    if not prompt_path.is_absolute():
        prompt_path = scripts_dir / prompt_path
    if not reference_yaml_path.is_absolute():
        reference_yaml_path = scripts_dir / reference_yaml_path
    prompt_path = prompt_path.resolve()
    reference_yaml_path = reference_yaml_path.resolve()

    if not prompt_path.is_file():
        if debug:
            print(
                f"    Preprocess: llm_decompile prompt missing for {func_name}: "
                f"{prompt_path}"
            )
        return None

    if not reference_yaml_path.is_file():
        if debug:
            print(
                f"    Preprocess: llm_decompile reference missing for {func_name}: "
                f"{reference_yaml_path}"
            )
        return None

    try:
        prompt_template = prompt_path.read_text(encoding="utf-8")
    except OSError as exc:
        if debug:
            print(
                f"    Preprocess: failed to read llm_decompile prompt for "
                f"{func_name}: {exc}"
            )
        return None

    try:
        with open(reference_yaml_path, "r", encoding="utf-8") as handle:
            reference_data = yaml.safe_load(handle) or {}
    except Exception as exc:
        if debug:
            print(
                f"    Preprocess: failed to read llm_decompile reference for "
                f"{func_name}: {exc}"
            )
        return None

    if not isinstance(reference_data, dict):
        if debug:
            print(
                f"    Preprocess: invalid llm_decompile reference payload for "
                f"{func_name}"
            )
        return None

    target_func_name = str(reference_data.get("func_name", "") or "").strip()
    if not target_func_name:
        if debug:
            print(
                f"    Preprocess: llm_decompile reference func_name missing for "
                f"{func_name}"
            )
        return None

    try:
        client = create_openai_client(
            llm_config.get("api_key"),
            llm_config.get("base_url"),
            api_key_required_message=(
                "llm_config.api_key is required for llm_decompile fallback"
            ),
        )
    except Exception as exc:
        if debug:
            print(
                f"    Preprocess: llm_decompile client unavailable for "
                f"{func_name}: {exc}"
            )
        return None

    if debug:
        print(
            f"    Preprocess: llm_decompile request ready for {func_name}: "
            f"platform={str(platform or '').strip() or '<empty>'}, "
            f"model={model}, prompt_path={prompt_path}, "
            f"reference_yaml_path={reference_yaml_path}"
        )
        _debug_print_json(
            f"llm_decompile reference payload for {func_name}",
            reference_data,
            debug=True,
        )

    return {
        "client": client,
        "model": model,
        "prompt_path": os.fspath(prompt_path),
        "reference_yaml_path": os.fspath(reference_yaml_path),
        "prompt_template": prompt_template,
        "target_func_name": target_func_name,
        "disasm_for_reference": str(reference_data.get("disasm_code", "") or ""),
        "procedure_for_reference": str(reference_data.get("procedure", "") or ""),
        "temperature": temperature,
    }


def _build_llm_decompile_request_cache_key(llm_request):
    if not isinstance(llm_request, dict):
        return None
    model = str(llm_request.get("model", "")).strip()
    prompt_path = str(llm_request.get("prompt_path", "")).strip()
    reference_yaml_path = str(llm_request.get("reference_yaml_path", "")).strip()
    if not model or not prompt_path or not reference_yaml_path:
        return None
    temperature = llm_request.get("temperature")
    return model, prompt_path, reference_yaml_path, temperature


async def call_llm_decompile(
    client,
    model,
    symbol_name_list,
    disasm_code,
    procedure,
    disasm_for_reference="",
    procedure_for_reference="",
    prompt_template=None,
    platform=None,
    temperature=None,
    debug=False,
):
    if not callable(call_llm_text):
        if debug:
            print("    Preprocess: call_llm_text unavailable for llm_decompile")
        return _empty_llm_decompile_result()

    if isinstance(symbol_name_list, (list, tuple, set)):
        symbol_name_text = ", ".join(
            str(item).strip() for item in symbol_name_list if str(item).strip()
        )
    else:
        symbol_name_text = str(symbol_name_list or "").strip()

    if prompt_template is not None:
        try:
            prompt = _resolve_llm_decompile_template_value(
                prompt_template,
                platform,
            ).format(
                symbol_name_list=symbol_name_text,
                disasm_for_reference=str(disasm_for_reference or ""),
                procedure_for_reference=str(procedure_for_reference or ""),
                disasm_code=str(disasm_code or ""),
                procedure=str(procedure or ""),
                platform=str(platform or "").strip(),
            )
        except Exception as exc:
            if debug:
                print(
                    f"    Preprocess: failed to format llm_decompile prompt for "
                    f"{symbol_name_text}: {exc}"
                )
            return _empty_llm_decompile_result()
    else:
        prompt = (
            "You are a reverse engineering expert.\n\n"
            f"Reference disassembly:\n{disasm_for_reference}\n\n"
            f"Reference procedure:\n{procedure_for_reference}\n\n"
            f"Target disassembly:\n{disasm_code}\n\n"
            f"Target procedure:\n{procedure}\n\n"
            f"Please collect all references to \"{symbol_name_text}\" and output YAML."
        )
    system_prompt = "You are a reverse engineering expert."
    if debug:
        print(
            f"    Preprocess: calling llm_decompile for {symbol_name_text} "
            f"with model={str(model).strip()} platform={str(platform or '').strip() or '<empty>'}"
        )
        _debug_print_multiline(
            f"llm_decompile system prompt for {symbol_name_text}",
            system_prompt,
            debug=True,
        )
        _debug_print_multiline(
            f"llm_decompile user prompt for {symbol_name_text}",
            prompt,
            debug=True,
        )
    try:
        request_kwargs = {
            "client": client,
            "model": str(model).strip(),
            "messages": [
                {"role": "system", "content": system_prompt},
                {"role": "user", "content": prompt},
            ],
        }
        normalized_temperature = temperature
        if normalized_temperature is not None and callable(normalize_optional_temperature):
            normalized_temperature = normalize_optional_temperature(
                normalized_temperature,
                "temperature",
            )
        if normalized_temperature is not None:
            request_kwargs["temperature"] = normalized_temperature
        content = call_llm_text(**request_kwargs)
    except Exception as exc:
        if debug:
            print(
                f"    Preprocess: llm_decompile call failed for "
                f"{symbol_name_text}: {exc}"
            )
        return _empty_llm_decompile_result()
    if debug:
        _debug_print_multiline(
            f"llm_decompile raw response for {symbol_name_text}",
            content,
            debug=True,
        )
    parsed_result = parse_llm_decompile_response(content)
    if debug:
        _debug_print_json(
            f"llm_decompile parsed response for {symbol_name_text}",
            parsed_result,
            debug=True,
        )
    return parsed_result

async def preprocess_vtable_via_mcp(
    session,
    class_name,
    image_base,
    platform,
    debug=False,
    symbol_aliases=None,
):
    """
    Preprocess a vtable output by looking up the class vtable via py_eval.

    No old YAML is needed - vtable lookup is purely class-name-based.

    Args:
        session: Active MCP ClientSession
        class_name: Class name (e.g., "CSource2Server")
        image_base: Binary image base address (int)
        platform: "windows" or "linux"
        debug: Enable debug output

    Returns:
        Dict with vtable YAML data, or None on failure
    """
    _ = platform  # Reserved for future platform-specific behavior.
    py_code = _build_vtable_py_eval(class_name, symbol_aliases=symbol_aliases)

    try:
        result = await session.call_tool(
            name="py_eval",
            arguments={"code": py_code}
        )
        result_data = parse_mcp_result(result)
    except Exception as e:
        if debug:
            print(f"    Preprocess vtable: py_eval error for {class_name}: {e}")
        return None

    # Parse py_eval result
    vtable_info = None
    if isinstance(result_data, dict):
        result_str = result_data.get("result", "")
        if result_str:
            try:
                vtable_info = json.loads(result_str)
            except (json.JSONDecodeError, TypeError):
                pass

    if not vtable_info or not isinstance(vtable_info, dict):
        if debug:
            print(f"    Preprocess vtable: no result for {class_name}")
        return None

    # Compute vtable_rva (py_eval doesn't know image_base)
    vtable_va_int = int(vtable_info["vtable_va"], 16)
    vtable_rva = hex(vtable_va_int - image_base)

    # Convert vtable_entries keys from string to int (JSON serialization side-effect)
    raw_entries = vtable_info.get("vtable_entries", {})
    entries = {int(k): v for k, v in raw_entries.items()}

    return {
        "vtable_class": vtable_info["vtable_class"],
        "vtable_symbol": vtable_info["vtable_symbol"],
        "vtable_va": vtable_info["vtable_va"],
        "vtable_rva": vtable_rva,
        "vtable_size": vtable_info["vtable_size"],
        "vtable_numvfunc": vtable_info["vtable_numvfunc"],
        "vtable_entries": entries,
    }


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
    """
    Preprocess a function output by reusing old-version signature metadata.

    Primary path:
    - Reuse old `func_sig`, locate unique match in the new binary, and resolve
      function metadata from the matched function head.

    Fallback path (for old YAML without `func_sig`):
    - Reuse old `vfunc_sig` (must uniquely match in the new binary), then reuse
      old vfunc index/offset and resolve function metadata from the
      corresponding entry in the new vtable YAML.
    - After resolving function VA/size from vtable, try to generate a new
      function-head `func_sig` automatically.

    Args:
        session: Active MCP ClientSession
        new_path: Full path to expected output YAML
        old_path: Full path to old version YAML (may be None)
        image_base: Binary image base address (int)
        new_binary_dir: Directory for new version outputs
        platform: "windows" or "linux"
        debug: Enable debug output

    Returns:
        Dict with function YAML data, or None on failure
    """
    if yaml is None:
        if debug:
            print("    Preprocess: PyYAML is required for func_sig preprocessing")
        return None

    normalized_mangled_class_names = _normalize_mangled_class_names(
        mangled_class_names,
        debug=debug,
    )
    if normalized_mangled_class_names is None:
        return None

    if (
        direct_func_va is not None
        or direct_vtable_class is not None
        or direct_vfunc_offset is not None
    ):
        return await _preprocess_direct_func_sig_via_mcp(
            session=session,
            new_path=new_path,
            image_base=image_base,
            platform=platform,
            func_name=func_name,
            direct_func_va=direct_func_va,
            direct_vtable_class=direct_vtable_class,
            direct_vfunc_offset=direct_vfunc_offset,
            require_func_sig=True,
            normalized_mangled_class_names=normalized_mangled_class_names,
            debug=debug,
        )

    # Check if old YAML exists
    if not old_path or not os.path.exists(old_path):
        if debug:
            print(f"    Preprocess: no old YAML for {os.path.basename(new_path)}")
        return None

    # Read old YAML
    try:
        with open(old_path, "r", encoding="utf-8") as f:
            old_data = yaml.safe_load(f)
    except Exception:
        return None

    if not old_data or not isinstance(old_data, dict):
        return None

    def _parse_int_field(value, field_name):
        if isinstance(value, int):
            return value
        if isinstance(value, str):
            raw = value.strip()
            if not raw:
                raise ValueError(f"empty {field_name}")
            return int(raw, 0)
        return int(value)

    def _parse_strict_int_field(value, field_name):
        if isinstance(value, bool):
            raise ValueError(f"invalid {field_name}")
        if isinstance(value, int):
            return value
        if isinstance(value, str):
            raw = value.strip()
            if not raw:
                raise ValueError(f"empty {field_name}")
            return int(raw, 0)
        raise ValueError(f"invalid {field_name}")

    async def _find_unique_match(signature, label):
        try:
            fb_result = await session.call_tool(
                name="find_bytes",
                arguments={"patterns": [signature], "limit": 2}
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
        if match_count != 1:
            if debug:
                print(f"    Preprocess: {label} matched {match_count} (need 1)")
            return None

        return matches[0]

    async def _find_match_with_limit(signature, label, max_match_count):
        try:
            max_match_count = _parse_strict_int_field(
                max_match_count,
                "max_match_count",
            )
        except Exception:
            return None

        if max_match_count <= 0:
            return None

        try:
            fb_result = await session.call_tool(
                name="find_bytes",
                arguments={
                    "patterns": [signature],
                    "limit": max_match_count + 1,
                }
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
                    f"(need 1..{max_match_count})"
                )
            return None

        return matches[0]

    async def _get_func_info(addr_expr):
        py_code = (
            f"import idaapi, json\n"
            f"addr = {addr_expr}\n"
            f"f = idaapi.get_func(addr)\n"
            f"if f and f.start_ea == addr:\n"
            f"    result = json.dumps({{'func_va': hex(f.start_ea), 'func_size': hex(f.end_ea - f.start_ea)}})\n"
            f"else:\n"
            f"    result = json.dumps(None)\n"
        )
        try:
            fi_result = await session.call_tool(
                name="py_eval",
                arguments={"code": py_code}
            )
            fi_data = parse_mcp_result(fi_result)
        except Exception as e:
            if debug:
                print(f"    Preprocess: py_eval error: {e}")
            return None

        func_info = None
        if isinstance(fi_data, dict):
            stderr_text = fi_data.get("stderr", "")
            if stderr_text and debug:
                print("    Preprocess: py_eval stderr:")
                print(stderr_text.strip())
            result_str = fi_data.get("result", "")
            if result_str:
                try:
                    func_info = json.loads(result_str)
                except (json.JSONDecodeError, TypeError):
                    pass

        if not isinstance(func_info, dict):
            return None
        if "func_va" not in func_info or "func_size" not in func_info:
            return None
        return func_info

    async def _load_vtable_data(vtable_name):
        vtable_yaml_path = os.path.join(
            new_binary_dir,
            f"{vtable_name}_vtable.{platform}.yaml"
        )

        if not os.path.exists(vtable_yaml_path):
            # Generate vtable YAML on-the-fly via py_eval
            vtable_gen_data = await preprocess_vtable_via_mcp(
                session,
                vtable_name,
                image_base,
                platform,
                debug,
                _get_mangled_class_aliases(
                    normalized_mangled_class_names,
                    vtable_name,
                ),
            )
            if vtable_gen_data is None:
                if debug:
                    print(
                        "    Preprocess: vtable YAML not found and generation failed: "
                        f"{os.path.basename(vtable_yaml_path)}"
                    )
                return None
            write_vtable_yaml(vtable_yaml_path, vtable_gen_data)
            if debug:
                print(f"    Preprocess: generated vtable YAML: {os.path.basename(vtable_yaml_path)}")

        try:
            with open(vtable_yaml_path, "r", encoding="utf-8") as vf:
                vtable_data = yaml.safe_load(vf)
        except Exception:
            return None

        if not isinstance(vtable_data, dict):
            return None
        return vtable_data

    func_sig = old_data.get("func_sig")
    vfunc_sig = old_data.get("vfunc_sig")
    vtable_name = old_data.get("vtable_name")

    used_vfunc_fallback = False
    vfunc_index = None
    vfunc_offset = None
    vfunc_match_addr = None
    vfunc_sig_max_match = 1

    if func_sig:
        match_addr = await _find_unique_match(
            func_sig, f"{os.path.basename(old_path)} func_sig"
        )
        if match_addr is None:
            return None

        func_info = await _get_func_info(match_addr)
        if not func_info:
            if debug:
                print(f"    Preprocess: could not get func info at {match_addr}")
            return None
    else:
        used_vfunc_fallback = True
        if not vfunc_sig:
            if debug:
                print(f"    Preprocess: no func_sig/vfunc_sig in {os.path.basename(old_path)}")
            return None
        if not vtable_name:
            if debug:
                print(
                    "    Preprocess: no vtable_name for vfunc fallback in "
                    f"{os.path.basename(old_path)}"
                )
            return None

        try:
            vfunc_sig_max_match = _parse_strict_int_field(
                old_data.get("vfunc_sig_max_match", 1),
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
                    "    Preprocess: invalid vfunc_sig_max_match in "
                    f"{os.path.basename(old_path)}"
                )
            return None

        has_index = old_data.get("vfunc_index") is not None
        has_offset = old_data.get("vfunc_offset") is not None
        if not has_index and not has_offset:
            if debug:
                print(
                    "    Preprocess: missing vfunc_index/vfunc_offset in "
                    f"{os.path.basename(old_path)}"
                )
            return None

        try:
            if has_index:
                vfunc_index = _parse_int_field(old_data.get("vfunc_index"), "vfunc_index")
            if has_offset:
                vfunc_offset = _parse_int_field(old_data.get("vfunc_offset"), "vfunc_offset")
        except Exception:
            if debug:
                print(f"    Preprocess: invalid vfunc metadata in {os.path.basename(old_path)}")
            return None

        if vfunc_index is None:
            if vfunc_offset % 8 != 0:
                if debug:
                    print(
                        "    Preprocess: vfunc_offset is not 8-byte aligned in "
                        f"{os.path.basename(old_path)}"
                    )
                return None
            vfunc_index = vfunc_offset // 8
        if vfunc_offset is None:
            vfunc_offset = vfunc_index * 8

        if vfunc_index < 0 or vfunc_offset < 0 or vfunc_offset != vfunc_index * 8:
            if debug:
                print(
                    "    Preprocess: inconsistent vfunc_index/vfunc_offset in "
                    f"{os.path.basename(old_path)}"
                )
            return None

        vfunc_match_addr = await _find_match_with_limit(
            vfunc_sig,
            f"{os.path.basename(old_path)} vfunc_sig",
            vfunc_sig_max_match,
        )
        if vfunc_match_addr is None:
            return None

        # If the old YAML never had func_va, the vtable is only used as
        # metadata (e.g. pure-interface classes like IGameTypes whose vtable
        # symbol cannot be resolved).  Carry forward vfunc metadata as-is.
        old_has_func_va = old_data.get("func_va") is not None
        if not old_has_func_va:
            if func_name is None:
                func_name = old_data.get("func_name")
            if func_name is None:
                func_name = os.path.basename(new_path).rsplit(".", 2)[0]

            new_data = {
                "func_name": func_name,
                "vfunc_sig": vfunc_sig,
                "vfunc_sig_max_match": vfunc_sig_max_match,
                "vtable_name": vtable_name,
                "vfunc_offset": hex(vfunc_offset),
                "vfunc_index": vfunc_index,
            }
            if debug:
                print(
                    "    Preprocess: reused vfunc_sig metadata (no vtable resolution) at "
                    f"{vfunc_match_addr} for {os.path.basename(new_path)}"
                )
            return new_data

        vtable_data = await _load_vtable_data(vtable_name)
        if not isinstance(vtable_data, dict):
            return None

        vtable_entries = vtable_data.get("vtable_entries", {})
        func_va_from_vtable = None
        for idx, entry_addr in vtable_entries.items():
            try:
                idx_int = int(idx)
            except Exception:
                continue
            if idx_int != vfunc_index:
                continue
            try:
                func_va_from_vtable = int(str(entry_addr), 16)
            except Exception:
                func_va_from_vtable = None
            break

        if func_va_from_vtable is None:
            if debug:
                print(
                    "    Preprocess: vfunc_index not found in vtable entries: "
                    f"{vtable_name}[{vfunc_index}]"
                )
            return None

        func_info = await _get_func_info(hex(func_va_from_vtable))
        if not func_info:
            if debug:
                print(
                    "    Preprocess: could not get func info from vtable entry: "
                    f"{vtable_name}[{vfunc_index}] -> {hex(func_va_from_vtable)}"
                )
            return None

    func_va_hex = func_info["func_va"]
    func_va_int = int(func_va_hex, 16)
    func_size_hex = func_info["func_size"]

    # Resolve func_name: explicit parameter > old YAML > derive from filename
    if func_name is None:
        func_name = old_data.get("func_name")
    if func_name is None:
        func_name = os.path.basename(new_path).rsplit(".", 2)[0]

    # Build new YAML data
    new_data = {
        "func_name": func_name,
        "func_va": func_va_hex,
        "func_rva": hex(func_va_int - image_base),
        "func_size": func_size_hex,
    }
    if func_sig:
        new_data["func_sig"] = func_sig

    # vfunc fallback path: reuse old index/offset and regenerate func_sig from vtable-resolved function.
    if used_vfunc_fallback:
        new_data["vfunc_sig"] = vfunc_sig
        new_data["vfunc_sig_max_match"] = vfunc_sig_max_match
        new_data["vtable_name"] = vtable_name
        new_data["vfunc_offset"] = hex(vfunc_offset)
        new_data["vfunc_index"] = vfunc_index

        if debug:
            print(
                "    Preprocess: reused vfunc_sig + vtable metadata at "
                f"{vfunc_match_addr} for {os.path.basename(new_path)}"
            )
        return new_data

    # For vfunc with func_sig input: cross-reference with new vtable YAML for vfunc_offset/index.
    if vtable_name:
        vtable_data = await _load_vtable_data(vtable_name)
        if not isinstance(vtable_data, dict):
            return None

        vtable_entries = vtable_data.get("vtable_entries", {})
        found_index = None
        for idx, entry_addr in vtable_entries.items():
            try:
                idx_int = int(idx)
                entry_int = int(str(entry_addr), 16)
            except Exception:
                continue
            if entry_int == func_va_int:
                found_index = idx_int
                break

        if found_index is None:
            if debug:
                print(f"    Preprocess: {func_va_hex} not in {vtable_name} vtable entries")
            return None

        new_data["vtable_name"] = vtable_name
        new_data["vfunc_offset"] = hex(found_index * 8)
        new_data["vfunc_index"] = found_index

    return new_data


async def preprocess_gen_func_sig_via_mcp(
    session,
    func_va,
    image_base,
    min_sig_bytes=6,
    max_sig_bytes=240,
    max_instructions=100,
    extra_wildcard_offsets=None,
    debug=False,
):
    """
    Generate a shortest unique function-head signature for a known function address.

    The generated signature always starts at the function entry (func start address),
    never from the middle of a function. The routine progressively searches for the
    shortest prefix that still uniquely resolves to the target function.

    Wildcards:
    - Auto wildcard: volatile operand bytes (imm/near/far/mem/displ) and branch/call
      relative offsets are wildcarded programmatically.
    - Extra wildcard: caller may provide additional byte offsets (relative to func head)
      via extra_wildcard_offsets.

    Args:
        session: Active MCP ClientSession.
        func_va: Function virtual address (int or hex string) and must be function head.
        image_base: Binary image base address (int).
        min_sig_bytes: Minimum signature prefix length to try.
        max_sig_bytes: Maximum bytes collected from function head.
        max_instructions: Max instructions collected from function head.
        extra_wildcard_offsets: Optional iterable of extra wildcard offsets.
        debug: Enable debug output.

    Returns:
        Dict with function YAML data (func_va, func_rva, func_size, func_sig),
        or None on failure.
    """

    def _parse_int(value):
        if isinstance(value, int):
            return value
        if isinstance(value, str):
            raw = value.strip()
            if not raw:
                raise ValueError("empty integer string")
            return int(raw, 0)
        return int(value)

    def _parse_addr(value):
        if isinstance(value, int):
            return value
        if isinstance(value, str):
            return int(value.strip(), 0)
        return int(value)

    try:
        func_va_int = _parse_int(func_va)
    except Exception:
        if debug:
            print(f"    Preprocess: invalid func_va: {func_va}")
        return None

    try:
        min_sig_bytes = max(1, int(min_sig_bytes))
        max_sig_bytes = max(1, int(max_sig_bytes))
        max_instructions = max(1, int(max_instructions))
    except Exception:
        if debug:
            print("    Preprocess: invalid signature generation limits")
        return None

    extra_wildcard_set = set()
    if extra_wildcard_offsets:
        try:
            for offset in extra_wildcard_offsets:
                parsed = _parse_int(offset)
                if parsed >= 0:
                    extra_wildcard_set.add(parsed)
        except Exception:
            if debug:
                print("    Preprocess: invalid extra_wildcard_offsets")
            return None

    py_code = (
        "import idaapi, ida_bytes, idautils, ida_ua, json\n"
        f"target_ea = {func_va_int}\n"
        f"max_sig_bytes = {max_sig_bytes}\n"
        f"max_instructions = {max_instructions}\n"
        "f = idaapi.get_func(target_ea)\n"
        "if not f or f.start_ea != target_ea:\n"
        "    result = json.dumps(None)\n"
        "else:\n"
        "    limit_end = min(f.end_ea, target_ea + max_sig_bytes)\n"
        "    insts = []\n"
        "    cursor = target_ea\n"
        "    total = 0\n"
        "    while cursor < f.end_ea and cursor < limit_end and len(insts) < max_instructions:\n"
        "        insn = idautils.DecodeInstruction(cursor)\n"
        "        if not insn or insn.size <= 0:\n"
        "            break\n"
        "        raw = ida_bytes.get_bytes(cursor, insn.size)\n"
        "        if not raw:\n"
        "            break\n"
        "        wild = set()\n"
        "        for op in insn.ops:\n"
        "            op_type = int(op.type)\n"
        "            if op_type == int(idaapi.o_void):\n"
        "                continue\n"
        "            if op_type in (int(idaapi.o_imm), int(idaapi.o_near), int(idaapi.o_far), int(idaapi.o_mem), int(idaapi.o_displ)):\n"
        "                offb = int(op.offb)\n"
        "                if offb > 0 and offb < insn.size:\n"
        "                    dsz = ida_ua.get_dtype_size(getattr(op, 'dtype', getattr(op, 'dtyp', 0)))\n"
        "                    if dsz <= 0:\n"
        "                        dsz = insn.size - offb\n"
        "                    end = min(insn.size, offb + dsz)\n"
        "                    for i in range(offb, end):\n"
        "                        wild.add(i)\n"
        "                offo = int(op.offo)\n"
        "                if offo > 0 and offo < insn.size:\n"
        "                    dsz2 = ida_ua.get_dtype_size(getattr(op, 'dtype', getattr(op, 'dtyp', 0)))\n"
        "                    if dsz2 <= 0:\n"
        "                        dsz2 = insn.size - offo\n"
        "                    end2 = min(insn.size, offo + dsz2)\n"
        "                    for i in range(offo, end2):\n"
        "                        wild.add(i)\n"
        "        b0 = raw[0]\n"
        "        if b0 in (0xE8, 0xE9, 0xEB):\n"
        "            for i in range(1, insn.size):\n"
        "                wild.add(i)\n"
        "        elif b0 == 0x0F and insn.size >= 2 and (raw[1] & 0xF0) == 0x80:\n"
        "            for i in range(2, insn.size):\n"
        "                wild.add(i)\n"
        "        elif 0x70 <= b0 <= 0x7F:\n"
        "            for i in range(1, insn.size):\n"
        "                wild.add(i)\n"
        "        insts.append({'ea': hex(cursor), 'size': insn.size, 'bytes': raw.hex(), 'wild': sorted(wild)})\n"
        "        cursor += insn.size\n"
        "        total += insn.size\n"
        "        if total >= max_sig_bytes:\n"
        "            break\n"
        "    result = json.dumps({'func_va': hex(f.start_ea), 'func_size': hex(f.end_ea - f.start_ea), 'insts': insts})\n"
    )

    try:
        fi_result = await session.call_tool(
            name="py_eval",
            arguments={"code": py_code},
        )
        fi_data = parse_mcp_result(fi_result)
    except Exception as e:
        if debug:
            print(f"    Preprocess: py_eval error while generating func_sig: {e}")
        return None

    func_info = None
    if isinstance(fi_data, dict):
        stderr_text = fi_data.get("stderr", "")
        if stderr_text and debug:
            print("    Preprocess: py_eval stderr:")
            print(stderr_text.strip())
        result_str = fi_data.get("result", "")
        if result_str:
            try:
                func_info = json.loads(result_str)
            except (json.JSONDecodeError, TypeError):
                pass

    if not isinstance(func_info, dict):
        if debug:
            print(f"    Preprocess: could not resolve function head at {hex(func_va_int)}")
        return None

    insts = func_info.get("insts", [])
    if not isinstance(insts, list) or len(insts) == 0:
        if debug:
            print(f"    Preprocess: no instruction bytes available at {hex(func_va_int)}")
        return None

    sig_tokens = []
    raw_tokens = []  # Parallel to sig_tokens; always stores actual hex values (never wildcarded)
    inst_boundaries = []
    for inst in insts:
        try:
            inst_size = int(inst.get("size", 0))
            inst_hex = str(inst.get("bytes", ""))
            if inst_size <= 0 or len(inst_hex) != inst_size * 2:
                if debug:
                    print("    Preprocess: malformed instruction bytes from py_eval")
                return None

            inst_bytes = [int(inst_hex[i:i + 2], 16) for i in range(0, len(inst_hex), 2)]
            inst_wild = set()
            for item in inst.get("wild", []):
                pos = int(item)
                if 0 <= pos < inst_size:
                    inst_wild.add(pos)
        except Exception:
            if debug:
                print("    Preprocess: failed to decode instruction bytes for func_sig")
            return None

        base_offset = len(sig_tokens)
        for rel_idx, value in enumerate(inst_bytes):
            abs_off = base_offset + rel_idx
            use_wild = (rel_idx in inst_wild) or (abs_off in extra_wildcard_set)
            sig_tokens.append("??" if use_wild else f"{value:02X}")
            raw_tokens.append(f"{value:02X}")

        # Growth step must align to the next full instruction boundary.
        inst_boundaries.append(len(sig_tokens))

    if len(sig_tokens) == 0 or len(inst_boundaries) == 0:
        if debug:
            print(f"    Preprocess: empty signature token stream at {hex(func_va_int)}")
        return None

    search_start = min_sig_bytes

    best_sig = None
    for prefix_len in inst_boundaries:
        if prefix_len < search_start:
            continue
        prefix_tokens = sig_tokens[:prefix_len]

        # Skip signatures that are all wildcards.
        if all(token == "??" for token in prefix_tokens):
            continue

        candidate_sig = " ".join(prefix_tokens)
        try:
            fb_result = await session.call_tool(
                name="find_bytes",
                arguments={"patterns": [candidate_sig], "limit": 2},
            )
            fb_data = parse_mcp_result(fb_result)
        except Exception as e:
            if debug:
                print(f"    Preprocess: find_bytes error while testing generated sig: {e}")
            return None

        if not isinstance(fb_data, list) or len(fb_data) == 0:
            continue

        entry = fb_data[0]
        matches = entry.get("matches", [])
        match_count = entry.get("n", len(matches))

        if match_count != 1 or not matches:
            continue

        try:
            match_addr = _parse_addr(matches[0])
        except Exception:
            continue

        # Signature must resolve to the target function head, not middle/function body.
        if match_addr != func_va_int:
            continue

        best_sig = candidate_sig
        break

    # --- Fallback: selectively un-wildcard to differentiate from competitors ---
    if not best_sig and len(sig_tokens) > 0:
        full_len = inst_boundaries[-1] if inst_boundaries else len(sig_tokens)
        full_candidate = " ".join(sig_tokens[:full_len])

        # Find competing matches using the full-length wildcarded pattern.
        competitor_addrs = []
        try:
            fb_result = await session.call_tool(
                name="find_bytes",
                arguments={"patterns": [full_candidate], "limit": 16},
            )
            fb_data = parse_mcp_result(fb_result)
            if isinstance(fb_data, list) and len(fb_data) > 0:
                for m in fb_data[0].get("matches", []):
                    try:
                        addr = _parse_addr(m)
                        if addr != func_va_int:
                            competitor_addrs.append(addr)
                    except Exception:
                        pass
        except Exception:
            competitor_addrs = []

        if competitor_addrs:
            # Read raw bytes from each competitor at the same offset range.
            addrs_list = [hex(a) for a in competitor_addrs]
            py_read_code = (
                "import ida_bytes, json\n"
                f"addrs = {json.dumps(addrs_list)}\n"
                f"size = {full_len}\n"
                "out = []\n"
                "for a in addrs:\n"
                "    ea = int(a, 16)\n"
                "    raw = ida_bytes.get_bytes(ea, size)\n"
                "    out.append(raw.hex().upper() if raw and len(raw) == size else '')\n"
                "result = json.dumps(out)\n"
            )
            competitor_hex_list = []
            try:
                gb_result = await session.call_tool(
                    name="py_eval",
                    arguments={"code": py_read_code},
                )
                gb_data = parse_mcp_result(gb_result)
                if isinstance(gb_data, dict):
                    result_str = gb_data.get("result", "")
                    if result_str:
                        parsed = json.loads(result_str)
                        if isinstance(parsed, list):
                            competitor_hex_list = [h for h in parsed if h]
            except Exception:
                pass

            if competitor_hex_list:
                # Identify wildcarded positions where our byte differs from
                # at least one competitor — un-wildcarding these helps distinguish.
                wc_positions = [
                    i for i, t in enumerate(sig_tokens[:full_len]) if t == "??"
                ]
                differing_wc = []
                for pos in wc_positions:
                    our_hex = raw_tokens[pos]
                    for comp_hex in competitor_hex_list:
                        if (
                            len(comp_hex) >= (pos + 1) * 2
                            and comp_hex[pos * 2 : pos * 2 + 2] != our_hex
                        ):
                            differing_wc.append(pos)
                            break

                if differing_wc:
                    if debug:
                        print(
                            f"    Preprocess: fallback un-wildcarding "
                            f"{len(differing_wc)} byte(s) to differentiate "
                            f"from {len(competitor_addrs)} competitor(s)"
                        )
                    refined_tokens = list(sig_tokens[:full_len])
                    for pos in differing_wc:
                        refined_tokens[pos] = raw_tokens[pos]

                    # Re-test at each instruction boundary with refined tokens.
                    for prefix_len in inst_boundaries:
                        if prefix_len < search_start:
                            continue
                        if prefix_len > full_len:
                            break
                        prefix = refined_tokens[:prefix_len]
                        if all(t == "??" for t in prefix):
                            continue
                        candidate_sig = " ".join(prefix)
                        try:
                            fb_result = await session.call_tool(
                                name="find_bytes",
                                arguments={
                                    "patterns": [candidate_sig],
                                    "limit": 2,
                                },
                            )
                            fb_data = parse_mcp_result(fb_result)
                        except Exception:
                            break
                        if not isinstance(fb_data, list) or len(fb_data) == 0:
                            continue
                        entry = fb_data[0]
                        matches = entry.get("matches", [])
                        if entry.get("n", len(matches)) != 1 or not matches:
                            continue
                        try:
                            match_addr = _parse_addr(matches[0])
                        except Exception:
                            continue
                        if match_addr != func_va_int:
                            continue
                        best_sig = candidate_sig
                        if debug:
                            print(
                                "    Preprocess: fallback generated unique "
                                f"func_sig ({len(best_sig.split())} bytes) "
                                f"for {hex(func_va_int)}"
                            )
                        break

    if not best_sig:
        if debug:
            print(
                "    Preprocess: failed to generate a unique function-head signature "
                f"for {hex(func_va_int)}"
            )
        return None

    try:
        resolved_func_va = str(func_info["func_va"])
        resolved_func_va_int = int(resolved_func_va, 16)
        resolved_func_size = str(func_info["func_size"])
    except Exception:
        if debug:
            print("    Preprocess: invalid func info returned from py_eval")
        return None

    if resolved_func_va_int != func_va_int:
        if debug:
            print(
                "    Preprocess: function head mismatch while generating func_sig "
                f"({hex(resolved_func_va_int)} != {hex(func_va_int)})"
            )
        return None

    if debug:
        print(
            "    Preprocess: generated shortest unique func_sig "
            f"({len(best_sig.split())} bytes) for {hex(func_va_int)}"
        )

    return {
        "func_va": resolved_func_va,
        "func_rva": hex(resolved_func_va_int - image_base),
        "func_size": resolved_func_size,
        "func_sig": best_sig,
    }


async def preprocess_gen_vfunc_sig_via_mcp(
    session,
    inst_va,
    vfunc_offset,
    max_match_count=1,
    min_sig_bytes=6,
    max_sig_bytes=96,
    max_instructions=64,
    extra_wildcard_offsets=None,
    debug=False,
):
    """
    Generate a shortest unique signature for a known virtual-call instruction.

    The generated signature starts at the virtual-call instruction itself
    (`vfunc_sig_disp = 0`). The first instruction is fully fixed, including the
    displacement bytes that encode `vfunc_offset`; subsequent instructions may
    wildcard volatile operands and branch displacements.

    Args:
        session: Active MCP ClientSession.
        inst_va: Virtual-call instruction address (int or hex string).
        vfunc_offset: Expected vtable slot displacement encoded by the target
            instruction (int or hex string).
        max_match_count: Maximum acceptable number of signature matches while
            still accepting the signature, as long as it contains `inst_va`.
        min_sig_bytes: Minimum signature prefix length to try.
        max_sig_bytes: Maximum bytes collected from signature start.
        max_instructions: Max instructions collected from signature start.
        extra_wildcard_offsets: Optional iterable of extra wildcard offsets
            relative to signature start.
        debug: Enable debug output.

    Returns:
        Dict with `vfunc_sig` metadata, or None on failure.
    """

    def _parse_addr(value):
        if isinstance(value, int):
            return value
        if isinstance(value, str):
            raw = value.strip()
            if not raw:
                raise ValueError("empty address string")
            return int(raw, 0)
        return int(value)

    try:
        inst_va_int = _parse_addr(inst_va)
        vfunc_offset_int = _parse_addr(vfunc_offset)
    except Exception:
        if debug:
            print(
                "    Preprocess: invalid vfunc signature generation inputs: "
                f"inst_va={inst_va!r}, vfunc_offset={vfunc_offset!r}"
            )
        return None

    if inst_va_int < 0 or vfunc_offset_int < 0:
        if debug:
            print(
                "    Preprocess: vfunc signature generation inputs must be >= 0: "
                f"inst_va={hex(inst_va_int)}, vfunc_offset={hex(vfunc_offset_int)}"
            )
        return None

    try:
        min_sig_bytes = max(1, int(min_sig_bytes))
        max_sig_bytes = max(1, int(max_sig_bytes))
        max_instructions = max(1, int(max_instructions))
        max_match_count = int(max_match_count)
    except Exception:
        if debug:
            print("    Preprocess: invalid vfunc signature generation limits")
        return None

    if max_match_count <= 0:
        if debug:
            print("    Preprocess: invalid max_match_count for vfunc_sig")
        return None

    extra_wildcard_set = set()
    if extra_wildcard_offsets:
        try:
            for offset in extra_wildcard_offsets:
                parsed = _parse_addr(offset)
                if parsed >= 0:
                    extra_wildcard_set.add(parsed)
        except Exception:
            if debug:
                print("    Preprocess: invalid extra_wildcard_offsets for vfunc_sig")
            return None

    py_code = (
        "import idaapi, ida_bytes, idautils, ida_ua, json\n"
        f"target_inst = {inst_va_int}\n"
        f"target_vfunc_offset = {vfunc_offset_int}\n"
        f"max_sig_bytes = {max_sig_bytes}\n"
        f"max_instructions = {max_instructions}\n"
        "\n"
        "def _find_vfunc_disp(insn, raw, expected):\n"
        "    hits = []\n"
        "    for op in insn.ops:\n"
        "        ot = int(op.type)\n"
        "        if ot == int(idaapi.o_void):\n"
        "            continue\n"
        "        if ot not in (int(idaapi.o_displ), int(idaapi.o_mem), int(idaapi.o_imm)):\n"
        "            continue\n"
        "        for attr in ('offb', 'offo'):\n"
        "            off = int(getattr(op, attr, 0))\n"
        "            if off <= 0 or off >= insn.size:\n"
        "                continue\n"
        "            sizes = []\n"
        "            dsz = ida_ua.get_dtype_size(getattr(op, 'dtype', getattr(op, 'dtyp', 0)))\n"
        "            if dsz > 0:\n"
        "                sizes.append(dsz)\n"
        "            for s in (1, 2, 4, 8):\n"
        "                if s not in sizes:\n"
        "                    sizes.append(s)\n"
        "            for sz in sizes:\n"
        "                if off + sz > insn.size:\n"
        "                    continue\n"
        "                chunk = raw[off:off + sz]\n"
        "                unsigned_val = int.from_bytes(chunk, 'little', signed=False)\n"
        "                signed_val = int.from_bytes(chunk, 'little', signed=True)\n"
        "                expected_mod = expected & ((1 << (8 * sz)) - 1)\n"
        "                if unsigned_val == expected_mod or signed_val == expected:\n"
        "                    hits.append((off, sz))\n"
        "    uniq = []\n"
        "    seen = set()\n"
        "    for hit in hits:\n"
        "        if hit in seen:\n"
        "            continue\n"
        "        seen.add(hit)\n"
        "        uniq.append(hit)\n"
        "    uniq.sort(key=lambda item: (item[1], -item[0]), reverse=True)\n"
        "    return uniq\n"
        "\n"
        "def _wildcard_instruction(insn, raw):\n"
        "    wild = set()\n"
        "    for op in insn.ops:\n"
        "        ot = int(op.type)\n"
        "        if ot == int(idaapi.o_void):\n"
        "            continue\n"
        "        if ot in (int(idaapi.o_imm), int(idaapi.o_near), int(idaapi.o_far), int(idaapi.o_mem), int(idaapi.o_displ)):\n"
        "            offb = int(getattr(op, 'offb', 0))\n"
        "            if offb > 0 and offb < insn.size:\n"
        "                dsz = ida_ua.get_dtype_size(getattr(op, 'dtype', getattr(op, 'dtyp', 0)))\n"
        "                if dsz <= 0:\n"
        "                    dsz = insn.size - offb\n"
        "                for i in range(offb, min(insn.size, offb + dsz)):\n"
        "                    wild.add(i)\n"
        "            offo = int(getattr(op, 'offo', 0))\n"
        "            if offo > 0 and offo < insn.size:\n"
        "                dsz2 = ida_ua.get_dtype_size(getattr(op, 'dtype', getattr(op, 'dtyp', 0)))\n"
        "                if dsz2 <= 0:\n"
        "                    dsz2 = insn.size - offo\n"
        "                for i in range(offo, min(insn.size, offo + dsz2)):\n"
        "                    wild.add(i)\n"
        "    b0 = raw[0]\n"
        "    if b0 in (0xE8, 0xE9, 0xEB):\n"
        "        for i in range(1, insn.size):\n"
        "            wild.add(i)\n"
        "    elif b0 == 0x0F and insn.size >= 2 and (raw[1] & 0xF0) == 0x80:\n"
        "        for i in range(2, insn.size):\n"
        "            wild.add(i)\n"
        "    elif 0x70 <= b0 <= 0x7F:\n"
        "        for i in range(1, insn.size):\n"
        "            wild.add(i)\n"
        "    return sorted(wild)\n"
        "\n"
        "globals().update(locals())\n"
        "\n"
        "insn0 = idautils.DecodeInstruction(target_inst)\n"
        "raw0 = ida_bytes.get_bytes(target_inst, insn0.size) if insn0 and insn0.size > 0 else None\n"
        "if not insn0 or insn0.size <= 0 or not raw0:\n"
        "    result = json.dumps(None)\n"
        "else:\n"
        "    disp_hits = _find_vfunc_disp(insn0, raw0, target_vfunc_offset)\n"
        "    if not disp_hits:\n"
        "        result = json.dumps(None)\n"
        "    else:\n"
        "        disp_off, disp_size = disp_hits[0]\n"
        "        insts = []\n"
        "        cursor = target_inst\n"
        "        total = 0\n"
        "        while cursor < target_inst + max_sig_bytes and len(insts) < max_instructions:\n"
        "            insn = idautils.DecodeInstruction(cursor)\n"
        "            if not insn or insn.size <= 0:\n"
        "                break\n"
        "            raw = ida_bytes.get_bytes(cursor, insn.size)\n"
        "            if not raw:\n"
        "                break\n"
        "            wild = [] if cursor == target_inst else _wildcard_instruction(insn, raw)\n"
        "            insts.append({\n"
        "                'ea': hex(cursor),\n"
        "                'size': insn.size,\n"
        "                'bytes': raw.hex(),\n"
        "                'wild': wild,\n"
        "            })\n"
        "            cursor += insn.size\n"
        "            total += insn.size\n"
        "            if total >= max_sig_bytes:\n"
        "                break\n"
        "        result = json.dumps({\n"
        "            'vfunc_sig_va': hex(target_inst),\n"
        "            'vfunc_inst_length': insn0.size,\n"
        "            'vfunc_disp_offset': disp_off,\n"
        "            'vfunc_disp_size': disp_size,\n"
        "            'insts': insts,\n"
        "        })\n"
    )

    try:
        sig_result = await session.call_tool(
            name="py_eval",
            arguments={"code": py_code},
        )
        sig_data = parse_mcp_result(sig_result)
    except Exception as e:
        if debug:
            print(f"    Preprocess: py_eval error while generating vfunc_sig: {e}")
        return None

    candidate_info = None
    if isinstance(sig_data, dict):
        stderr_text = sig_data.get("stderr", "")
        if stderr_text and debug:
            print("    Preprocess: py_eval stderr:")
            print(stderr_text.strip())
        result_str = sig_data.get("result", "")
        if result_str:
            try:
                candidate_info = json.loads(result_str)
            except (json.JSONDecodeError, TypeError):
                pass

    if not isinstance(candidate_info, dict):
        if debug:
            print(
                "    Preprocess: target instruction does not expose the expected "
                f"vfunc offset at {hex(inst_va_int)}"
            )
        return None

    try:
        first_len = int(candidate_info["vfunc_inst_length"])
        disp_off = int(candidate_info["vfunc_disp_offset"])
        disp_size = int(candidate_info["vfunc_disp_size"])
        insts = candidate_info.get("insts", [])
    except Exception:
        if debug:
            print("    Preprocess: malformed vfunc signature candidate metadata")
        return None

    if first_len <= 0 or disp_off < 0 or disp_size <= 0:
        if debug:
            print("    Preprocess: invalid vfunc signature candidate lengths")
        return None

    if not isinstance(insts, list) or len(insts) == 0:
        if debug:
            print(f"    Preprocess: no instruction bytes available at {hex(inst_va_int)}")
        return None

    sig_tokens = []
    inst_boundaries = []
    for inst in insts:
        try:
            inst_size = int(inst.get("size", 0))
            inst_hex = str(inst.get("bytes", ""))
            if inst_size <= 0 or len(inst_hex) != inst_size * 2:
                if debug:
                    print("    Preprocess: malformed instruction bytes from py_eval")
                return None

            inst_bytes = [
                int(inst_hex[i:i + 2], 16)
                for i in range(0, len(inst_hex), 2)
            ]
            inst_wild = set()
            for item in inst.get("wild", []):
                pos = int(item)
                if 0 <= pos < inst_size:
                    inst_wild.add(pos)
        except Exception:
            if debug:
                print("    Preprocess: failed to decode instruction bytes for vfunc_sig")
            return None

        base_offset = len(sig_tokens)
        for rel_idx, value in enumerate(inst_bytes):
            abs_off = base_offset + rel_idx
            use_wild = (rel_idx in inst_wild) or (abs_off in extra_wildcard_set)
            sig_tokens.append("??" if use_wild else f"{value:02X}")

        inst_boundaries.append(len(sig_tokens))

    if len(sig_tokens) == 0 or len(inst_boundaries) == 0:
        if debug:
            print(f"    Preprocess: empty vfunc signature token stream at {hex(inst_va_int)}")
        return None

    search_start = max(min_sig_bytes, first_len)

    best_sig = None
    best_sig_len = None
    for prefix_len in inst_boundaries:
        if prefix_len < search_start:
            continue
        prefix_tokens = sig_tokens[:prefix_len]
        if all(token == "??" for token in prefix_tokens):
            continue

        candidate_sig = " ".join(prefix_tokens)
        try:
            fb_result = await session.call_tool(
                name="find_bytes",
                arguments={
                    "patterns": [candidate_sig],
                    "limit": max_match_count + 1,
                },
            )
            fb_data = parse_mcp_result(fb_result)
        except Exception as e:
            if debug:
                print(
                    f"    Preprocess: find_bytes error while testing generated "
                    f"vfunc_sig: {e}"
                )
            return None

        if not isinstance(fb_data, list) or len(fb_data) == 0:
            continue

        entry = fb_data[0]
        matches = entry.get("matches", [])
        match_count = entry.get("n", len(matches))

        if match_count < 1 or match_count > max_match_count or not matches:
            continue

        match_addrs = set()
        try:
            for match in matches:
                match_addrs.add(_parse_addr(match))
        except Exception:
            continue

        if inst_va_int not in match_addrs:
            continue

        best_sig = candidate_sig
        best_sig_len = prefix_len
        break

    if not best_sig:
        if debug:
            print(
                "    Preprocess: failed to generate a unique vfunc signature "
                f"for {hex(inst_va_int)}"
            )
        return None

    if debug:
        print(
            "    Preprocess: generated shortest unique vfunc_sig "
            f"({best_sig_len} bytes) for {hex(inst_va_int)}"
        )

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


async def preprocess_gen_gv_sig_via_mcp(
    session,
    gv_va,
    image_base,
    gv_access_inst_va=None,
    gv_access_func_va=None,
    min_sig_bytes=8,
    max_sig_bytes=96,
    max_instructions=64,
    max_candidates=32,
    extra_wildcard_offsets=None,
    debug=False,
):
    """
    Generate a shortest unique signature for a known global variable address.

    The generated signature MUST resolve to an instruction that accesses the global
    variable (GV-accessing instruction). The signature start address equals that
    instruction address (gv_inst_offset = 0).

    Args:
        session: Active MCP ClientSession.
        gv_va: Global variable virtual address (int or hex string).
        image_base: Binary image base address (int).
        gv_access_inst_va: Optional instruction address known to access gv_va.
        gv_access_func_va: Optional function address to constrain candidate search.
        min_sig_bytes: Minimum signature prefix length to try.
        max_sig_bytes: Maximum bytes collected from signature start.
        max_instructions: Max instructions collected from signature start.
        max_candidates: Maximum GV-access instruction candidates to evaluate.
        extra_wildcard_offsets: Optional iterable of extra wildcard offsets relative
            to signature start.
        debug: Enable debug output.

    Returns:
        Dict with global-variable YAML data, or None on failure.
    """

    def _parse_int(value):
        if isinstance(value, int):
            return value
        if isinstance(value, str):
            raw = value.strip()
            if not raw:
                raise ValueError("empty integer string")
            return int(raw, 0)
        return int(value)

    def _parse_addr(value):
        if isinstance(value, int):
            return value
        if isinstance(value, str):
            return int(value.strip(), 0)
        return int(value)

    try:
        gv_va_int = _parse_int(gv_va)
    except Exception:
        if debug:
            print(f"    Preprocess: invalid gv_va: {gv_va}")
        return None

    try:
        min_sig_bytes = max(1, int(min_sig_bytes))
        max_sig_bytes = max(1, int(max_sig_bytes))
        max_instructions = max(1, int(max_instructions))
        max_candidates = max(1, int(max_candidates))
    except Exception:
        if debug:
            print("    Preprocess: invalid gv signature generation limits")
        return None

    inst_va_int = None
    if gv_access_inst_va is not None:
        try:
            inst_va_int = _parse_int(gv_access_inst_va)
        except Exception:
            if debug:
                print(f"    Preprocess: invalid gv_access_inst_va: {gv_access_inst_va}")
            return None

    func_va_int = None
    if gv_access_func_va is not None:
        try:
            func_va_int = _parse_int(gv_access_func_va)
        except Exception:
            if debug:
                print(f"    Preprocess: invalid gv_access_func_va: {gv_access_func_va}")
            return None

    extra_wildcard_set = set()
    if extra_wildcard_offsets:
        try:
            for offset in extra_wildcard_offsets:
                parsed = _parse_int(offset)
                if parsed >= 0:
                    extra_wildcard_set.add(parsed)
        except Exception:
            if debug:
                print("    Preprocess: invalid extra_wildcard_offsets")
            return None

    py_code = (
        "import idaapi, ida_bytes, idautils, ida_ua, json\n"
        f"target_gv = {gv_va_int}\n"
        f"target_inst = {inst_va_int if inst_va_int is not None else 'None'}\n"
        f"target_func = {func_va_int if func_va_int is not None else 'None'}\n"
        f"max_sig_bytes = {max_sig_bytes}\n"
        f"max_instructions = {max_instructions}\n"
        f"max_candidates = {max_candidates}\n"
        "\n"
        "def _resolve_disp_off(insn_ea, insn, raw):\n"
        "    cand_offsets = set()\n"
        "    for op in insn.ops:\n"
        "        op_type = int(op.type)\n"
        "        if op_type == int(idaapi.o_void):\n"
        "            continue\n"
        "        offb = int(getattr(op, 'offb', 0))\n"
        "        offo = int(getattr(op, 'offo', 0))\n"
        "        if offb > 0 and offb + 4 <= insn.size:\n"
        "            cand_offsets.add(offb)\n"
        "        if offo > 0 and offo + 4 <= insn.size:\n"
        "            cand_offsets.add(offo)\n"
        "\n"
        "    for off in sorted(cand_offsets):\n"
        "        disp_i32 = int.from_bytes(raw[off:off + 4], 'little', signed=True)\n"
        "        resolved = (insn_ea + insn.size + disp_i32) & 0xFFFFFFFFFFFFFFFF\n"
        "        if resolved == target_gv:\n"
        "            return off\n"
        "\n"
        "    return None\n"
        "\n"
        "def _collect_sig_stream(inst_ea, disp_off):\n"
        "    f = idaapi.get_func(inst_ea)\n"
        "    if not f:\n"
        "        return None\n"
        "\n"
        "    limit_end = min(f.end_ea, inst_ea + max_sig_bytes)\n"
        "    cursor = inst_ea\n"
        "    total = 0\n"
        "    insts = []\n"
        "    first_len = None\n"
        "\n"
        "    while cursor < f.end_ea and cursor < limit_end and len(insts) < max_instructions:\n"
        "        insn = idautils.DecodeInstruction(cursor)\n"
        "        if not insn or insn.size <= 0:\n"
        "            break\n"
        "\n"
        "        raw = ida_bytes.get_bytes(cursor, insn.size)\n"
        "        if not raw:\n"
        "            break\n"
        "\n"
        "        wild = set()\n"
        "        for op in insn.ops:\n"
        "            op_type = int(op.type)\n"
        "            if op_type == int(idaapi.o_void):\n"
        "                continue\n"
        "\n"
        "            if op_type in (int(idaapi.o_imm), int(idaapi.o_near), int(idaapi.o_far), int(idaapi.o_mem), int(idaapi.o_displ)):\n"
        "                offb = int(getattr(op, 'offb', 0))\n"
        "                if offb > 0 and offb < insn.size:\n"
        "                    dsz = ida_ua.get_dtype_size(getattr(op, 'dtype', getattr(op, 'dtyp', 0)))\n"
        "                    if dsz <= 0:\n"
        "                        dsz = insn.size - offb\n"
        "                    end = min(insn.size, offb + dsz)\n"
        "                    for i in range(offb, end):\n"
        "                        wild.add(i)\n"
        "\n"
        "                offo = int(getattr(op, 'offo', 0))\n"
        "                if offo > 0 and offo < insn.size:\n"
        "                    dsz2 = ida_ua.get_dtype_size(getattr(op, 'dtype', getattr(op, 'dtyp', 0)))\n"
        "                    if dsz2 <= 0:\n"
        "                        dsz2 = insn.size - offo\n"
        "                    end2 = min(insn.size, offo + dsz2)\n"
        "                    for i in range(offo, end2):\n"
        "                        wild.add(i)\n"
        "\n"
        "        b0 = raw[0]\n"
        "        if b0 in (0xE8, 0xE9, 0xEB):\n"
        "            for i in range(1, insn.size):\n"
        "                wild.add(i)\n"
        "        elif b0 == 0x0F and insn.size >= 2 and (raw[1] & 0xF0) == 0x80:\n"
        "            for i in range(2, insn.size):\n"
        "                wild.add(i)\n"
        "        elif 0x70 <= b0 <= 0x7F:\n"
        "            for i in range(1, insn.size):\n"
        "                wild.add(i)\n"
        "\n"
        "        if cursor == inst_ea:\n"
        "            first_len = insn.size\n"
        "            for i in range(disp_off, min(insn.size, disp_off + 4)):\n"
        "                wild.add(i)\n"
        "\n"
        "        insts.append({'ea': hex(cursor), 'size': insn.size, 'bytes': raw.hex(), 'wild': sorted(wild)})\n"
        "\n"
        "        cursor += insn.size\n"
        "        total += insn.size\n"
        "        if total >= max_sig_bytes:\n"
        "            break\n"
        "\n"
        "    if not insts or first_len is None:\n"
        "        return None\n"
        "\n"
        "    return {\n"
        "        'gv_inst_va': hex(inst_ea),\n"
        "        'gv_inst_length': first_len,\n"
        "        'gv_inst_disp': disp_off,\n"
        "        'insts': insts,\n"
        "    }\n"
        "\n"
        "candidates = []\n"
        "seen = set()\n"
        "\n"
        "def _try_add(inst_ea):\n"
        "    if inst_ea in seen:\n"
        "        return\n"
        "    seen.add(inst_ea)\n"
        "\n"
        "    insn = idautils.DecodeInstruction(inst_ea)\n"
        "    if not insn or insn.size <= 0:\n"
        "        return\n"
        "\n"
        "    raw = ida_bytes.get_bytes(inst_ea, insn.size)\n"
        "    if not raw:\n"
        "        return\n"
        "\n"
        "    disp_off = _resolve_disp_off(inst_ea, insn, raw)\n"
        "    if disp_off is None:\n"
        "        return\n"
        "\n"
        "    packed = _collect_sig_stream(inst_ea, disp_off)\n"
        "    if packed is None:\n"
        "        return\n"
        "\n"
        "    candidates.append(packed)\n"
        "\n"
        "globals().update(locals())\n"
        "\n"
        "if target_inst is not None:\n"
        "    _try_add(target_inst)\n"
        "elif target_func is not None:\n"
        "    f = idaapi.get_func(target_func)\n"
        "    if f:\n"
        "        ea = f.start_ea\n"
        "        while ea < f.end_ea and len(candidates) < max_candidates:\n"
        "            flags = ida_bytes.get_full_flags(ea)\n"
        "            if ida_bytes.is_code(flags):\n"
        "                _try_add(ea)\n"
        "\n"
        "            next_ea = ida_bytes.next_head(ea, f.end_ea)\n"
        "            if next_ea == idaapi.BADADDR or next_ea <= ea:\n"
        "                break\n"
        "            ea = next_ea\n"
        "else:\n"
        "    for ref in idautils.DataRefsTo(target_gv):\n"
        "        if len(candidates) >= max_candidates:\n"
        "            break\n"
        "\n"
        "        flags = ida_bytes.get_full_flags(ref)\n"
        "        if not ida_bytes.is_code(flags):\n"
        "            continue\n"
        "\n"
        "        _try_add(ref)\n"
        "\n"
        "result = json.dumps(candidates)\n"
    )

    try:
        gv_result = await session.call_tool(
            name="py_eval",
            arguments={"code": py_code},
        )
        gv_data = parse_mcp_result(gv_result)
    except Exception as e:
        if debug:
            print(f"    Preprocess: py_eval error while generating gv_sig: {e}")
        return None

    candidate_infos = None
    if isinstance(gv_data, dict):
        stderr_text = gv_data.get("stderr", "")
        if stderr_text and debug:
            print("    Preprocess: py_eval stderr:")
            print(stderr_text.strip())

        result_str = gv_data.get("result", "")
        if result_str:
            try:
                candidate_infos = json.loads(result_str)
            except (json.JSONDecodeError, TypeError):
                pass

    if not isinstance(candidate_infos, list) or len(candidate_infos) == 0:
        if debug:
            print(f"    Preprocess: no gv-access instruction candidates for {hex(gv_va_int)}")
        return None

    best = None

    for cand in candidate_infos:
        try:
            gv_inst_va = _parse_addr(cand.get("gv_inst_va"))
            gv_inst_length = int(cand.get("gv_inst_length"))
            gv_inst_disp = int(cand.get("gv_inst_disp"))
            insts = cand.get("insts", [])
        except Exception:
            continue

        if not isinstance(insts, list) or len(insts) == 0 or gv_inst_length <= 0 or gv_inst_disp < 0:
            continue

        sig_tokens = []
        inst_boundaries = []
        malformed = False
        for inst in insts:
            try:
                inst_size = int(inst.get("size", 0))
                inst_hex = str(inst.get("bytes", ""))
                if inst_size <= 0 or len(inst_hex) != inst_size * 2:
                    malformed = True
                    break

                inst_bytes = [int(inst_hex[i:i + 2], 16) for i in range(0, len(inst_hex), 2)]
                inst_wild = set()
                for item in inst.get("wild", []):
                    pos = int(item)
                    if 0 <= pos < inst_size:
                        inst_wild.add(pos)
            except Exception:
                malformed = True
                break

            base_offset = len(sig_tokens)
            for rel_idx, value in enumerate(inst_bytes):
                abs_off = base_offset + rel_idx
                use_wild = (rel_idx in inst_wild) or (abs_off in extra_wildcard_set)
                sig_tokens.append("??" if use_wild else f"{value:02X}")

            # Growth step must align to the next full instruction boundary.
            inst_boundaries.append(len(sig_tokens))

        if malformed or len(sig_tokens) == 0 or len(inst_boundaries) == 0:
            continue

        search_start = min_sig_bytes

        for prefix_len in inst_boundaries:
            if prefix_len < search_start:
                continue
            prefix_tokens = sig_tokens[:prefix_len]

            if all(token == "??" for token in prefix_tokens):
                continue

            candidate_sig = " ".join(prefix_tokens)
            try:
                fb_result = await session.call_tool(
                    name="find_bytes",
                    arguments={"patterns": [candidate_sig], "limit": 2},
                )
                fb_data = parse_mcp_result(fb_result)
            except Exception as e:
                if debug:
                    print(f"    Preprocess: find_bytes error while testing generated gv_sig: {e}")
                return None

            if not isinstance(fb_data, list) or len(fb_data) == 0:
                continue

            entry = fb_data[0]
            matches = entry.get("matches", [])
            match_count = entry.get("n", len(matches))

            if match_count != 1 or not matches:
                continue

            try:
                match_addr = _parse_addr(matches[0])
            except Exception:
                continue

            # Signature must resolve to this GV-accessing instruction address.
            if match_addr != gv_inst_va:
                continue

            if best is None or prefix_len < best["sig_len"]:
                best = {
                    "sig": candidate_sig,
                    "sig_len": prefix_len,
                    "gv_sig_va": gv_inst_va,
                    "gv_inst_length": gv_inst_length,
                    "gv_inst_disp": gv_inst_disp,
                }
            break

    if best is None:
        if debug:
            print(
                "    Preprocess: failed to generate a unique gv-access signature "
                f"for {hex(gv_va_int)}"
            )
        return None

    if debug:
        print(
            "    Preprocess: generated shortest unique gv_sig "
            f"({best['sig_len']} bytes) for {hex(gv_va_int)} at {hex(best['gv_sig_va'])}"
        )

    return {
        "gv_va": hex(gv_va_int),
        "gv_rva": hex(gv_va_int - image_base),
        "gv_sig": best["sig"],
        "gv_sig_va": hex(best["gv_sig_va"]),
        "gv_inst_offset": 0,
        "gv_inst_length": best["gv_inst_length"],
        "gv_inst_disp": best["gv_inst_disp"],
    }

async def preprocess_gv_sig_via_mcp(
    session, new_path, old_path, image_base, new_binary_dir, platform, debug=False
):
    """
    Preprocess a global-variable output by reusing old-version gv_sig signature.

    Searches the old signature in the new binary via find_bytes.
    For unique matches, resolves gv_va from RIP-relative instruction metadata
    (gv_inst_offset, gv_inst_length, gv_inst_disp) and builds new YAML data.

    Args:
        session: Active MCP ClientSession
        new_path: Full path to expected output YAML
        old_path: Full path to old version YAML (may be None)
        image_base: Binary image base address (int)
        new_binary_dir: Directory for new version outputs (reserved)
        platform: "windows" or "linux" (reserved)
        debug: Enable debug output

    Returns:
        Dict with global-variable YAML data, or None on failure
    """
    _ = new_binary_dir, platform  # Reserved for future platform-specific behavior.

    if yaml is None:
        if debug:
            print("    Preprocess: PyYAML is required for gv_sig preprocessing")
        return None

    # Check if old YAML exists
    if not old_path or not os.path.exists(old_path):
        if debug:
            print(f"    Preprocess: no old YAML for {os.path.basename(new_path)}")
        return None

    # Read old YAML
    try:
        with open(old_path, "r", encoding="utf-8") as f:
            old_data = yaml.safe_load(f)
    except Exception:
        return None

    if not old_data or not isinstance(old_data, dict):
        return None

    gv_sig = old_data.get("gv_sig")
    if not gv_sig:
        if debug:
            print(f"    Preprocess: no gv_sig in {os.path.basename(old_path)}")
        return None

    def _parse_int_field(value, field_name):
        if isinstance(value, int):
            return value
        if isinstance(value, str):
            raw = value.strip()
            if not raw:
                raise ValueError(f"empty {field_name}")
            return int(raw, 0)
        return int(value)

    try:
        gv_inst_offset = _parse_int_field(old_data.get("gv_inst_offset"), "gv_inst_offset")
        gv_inst_length = _parse_int_field(old_data.get("gv_inst_length"), "gv_inst_length")
        gv_inst_disp = _parse_int_field(old_data.get("gv_inst_disp"), "gv_inst_disp")
    except Exception:
        if debug:
            print(f"    Preprocess: invalid gv instruction metadata in {os.path.basename(old_path)}")
        return None

    if gv_inst_offset < 0 or gv_inst_length <= 0 or gv_inst_disp < 0:
        if debug:
            print(f"    Preprocess: invalid gv instruction values in {os.path.basename(old_path)}")
        return None

    # Search signature in new binary via MCP find_bytes
    try:
        fb_result = await session.call_tool(
            name="find_bytes",
            arguments={"patterns": [gv_sig], "limit": 2}
        )
        fb_data = parse_mcp_result(fb_result)
    except Exception as e:
        if debug:
            print(f"    Preprocess: find_bytes error: {e}")
        return None

    # Parse find_bytes result: list of {pattern, matches, n, ...}
    if not isinstance(fb_data, list) or len(fb_data) == 0:
        return None

    entry = fb_data[0]
    matches = entry.get("matches", [])
    match_count = entry.get("n", len(matches))

    if match_count != 1:
        if debug:
            print(f"    Preprocess: {os.path.basename(old_path)} gv sig matched {match_count} (need 1)")
        return None

    match_addr = matches[0]  # hex string like "0x1804f3df3"

    # Resolve gv_va from instruction metadata via py_eval
    py_code = (
        f"import ida_bytes, json\n"
        f"sig_addr = {match_addr}\n"
        f"inst_addr = sig_addr + {gv_inst_offset}\n"
        f"inst_length = {gv_inst_length}\n"
        f"inst_disp = {gv_inst_disp}\n"
        f"inst_bytes = ida_bytes.get_bytes(inst_addr, inst_length)\n"
        f"if not inst_bytes or len(inst_bytes) < inst_disp + 4:\n"
        f"    result = json.dumps(None)\n"
        f"else:\n"
        f"    disp_bytes = inst_bytes[inst_disp:inst_disp + 4]\n"
        f"    disp_i32 = int.from_bytes(disp_bytes, 'little', signed=True)\n"
        f"    gv_addr = (inst_addr + inst_length + disp_i32) & 0xFFFFFFFFFFFFFFFF\n"
        f"    result = json.dumps({{'gv_va': hex(gv_addr), 'gv_sig_va': hex(sig_addr)}})\n"
    )

    try:
        gv_result = await session.call_tool(
            name="py_eval",
            arguments={"code": py_code}
        )
        gv_data = parse_mcp_result(gv_result)
    except Exception as e:
        if debug:
            print(f"    Preprocess: py_eval error: {e}")
        return None

    # Parse py_eval result
    gv_info = None
    if isinstance(gv_data, dict):
        stderr_text = gv_data.get("stderr", "")
        if stderr_text and debug:
            print("    Preprocess: py_eval stderr:")
            print(stderr_text.strip())
        result_str = gv_data.get("result", "")
        if result_str:
            try:
                gv_info = json.loads(result_str)
            except (json.JSONDecodeError, TypeError):
                pass

    if not gv_info:
        if debug:
            print(f"    Preprocess: could not resolve global variable at {match_addr}")
        return None

    try:
        gv_va_hex = str(gv_info["gv_va"])
        gv_va_int = int(gv_va_hex, 16)
    except Exception:
        if debug:
            print(f"    Preprocess: invalid gv_va parsed from {match_addr}")
        return None

    gv_sig_va_hex = str(gv_info.get("gv_sig_va", match_addr))

    return {
        "gv_name": old_data.get("gv_name") or os.path.basename(new_path).split(".")[0],
        "gv_va": gv_va_hex,
        "gv_rva": hex(gv_va_int - image_base),
        "gv_sig": gv_sig,
        "gv_sig_va": gv_sig_va_hex,
        "gv_inst_offset": gv_inst_offset,
        "gv_inst_length": gv_inst_length,
        "gv_inst_disp": gv_inst_disp,
    }




async def preprocess_patch_via_mcp(
    session, new_path, old_path, image_base, new_binary_dir, platform, debug=False
):
    """
    Preprocess a patch output by reusing old-version patch metadata.

    Verifies that old ``patch_sig`` can be uniquely found in the new binary via
    ``find_bytes``. On success, reuses ``patch_name``, ``patch_sig``, and
    ``patch_bytes`` from old YAML.

    Args:
        session: Active MCP ClientSession
        new_path: Full path to expected output YAML
        old_path: Full path to old version YAML (may be None)
        image_base: Binary image base address (reserved)
        new_binary_dir: Directory for new version outputs (reserved)
        platform: "windows" or "linux" (reserved)
        debug: Enable debug output

    Returns:
        Dict with patch YAML data, or None on failure
    """
    _ = image_base, new_binary_dir, platform  # Reserved for future behavior.

    if yaml is None:
        if debug:
            print("    Preprocess: PyYAML is required for patch preprocessing")
        return None

    # Check if old YAML exists
    if not old_path or not os.path.exists(old_path):
        if debug:
            print(f"    Preprocess: no old YAML for {os.path.basename(new_path)}")
        return None

    # Read old YAML
    try:
        with open(old_path, "r", encoding="utf-8") as f:
            old_data = yaml.safe_load(f)
    except Exception:
        return None

    if not old_data or not isinstance(old_data, dict):
        return None

    patch_sig = old_data.get("patch_sig")
    patch_bytes = old_data.get("patch_bytes")

    if not patch_sig:
        if debug:
            print(f"    Preprocess: no patch_sig in {os.path.basename(old_path)}")
        return None

    if not patch_bytes:
        if debug:
            print(f"    Preprocess: no patch_bytes in {os.path.basename(old_path)}")
        return None

    # Verify patch_sig uniquely matches in new binary via MCP find_bytes.
    try:
        fb_result = await session.call_tool(
            name="find_bytes",
            arguments={"patterns": [patch_sig], "limit": 2}
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
    try:
        match_count_int = int(match_count)
    except Exception:
        match_count_int = len(matches)

    if match_count_int != 1:
        if debug:
            print(
                "    Preprocess: patch_sig matched "
                f"{match_count_int} (need 1) in {os.path.basename(old_path)}"
            )
        return None

    return {
        "patch_name": old_data.get("patch_name") or os.path.basename(new_path).rsplit(".", 2)[0],
        "patch_sig": patch_sig,
        "patch_bytes": patch_bytes,
    }

async def preprocess_struct_offset_sig_via_mcp(
    session, new_path, old_path, image_base, new_binary_dir, platform, debug=False
):
    """
    Preprocess a struct-member offset output by reusing old-version offset_sig signature.

    Searches the old signature in the new binary via find_bytes.
    For unique matches, decodes the target instruction (match + offset_sig_disp)
    and extracts the struct offset displacement/immediate.

    Args:
        session: Active MCP ClientSession
        new_path: Full path to expected output YAML
        old_path: Full path to old version YAML (may be None)
        image_base: Binary image base address (reserved)
        new_binary_dir: Directory for new version outputs (reserved)
        platform: "windows" or "linux" (reserved)
        debug: Enable debug output

    Returns:
        Dict with struct-member YAML data, or None on failure
    """
    _ = image_base, new_binary_dir, platform  # Reserved for future platform-specific behavior.

    if yaml is None:
        if debug:
            print("    Preprocess: PyYAML is required for struct offset preprocessing")
        return None

    if not old_path or not os.path.exists(old_path):
        if debug:
            print(f"    Preprocess: no old YAML for {os.path.basename(new_path)}")
        return None

    try:
        with open(old_path, "r", encoding="utf-8") as f:
            old_data = yaml.safe_load(f)
    except Exception:
        return None

    if not old_data or not isinstance(old_data, dict):
        return None

    struct_name = old_data.get("struct_name")
    member_name = old_data.get("member_name")
    offset_sig = old_data.get("offset_sig")
    if not struct_name or not member_name:
        if debug:
            print(f"    Preprocess: missing struct_name/member_name in {os.path.basename(old_path)}")
        return None
    if not offset_sig:
        if debug:
            print(f"    Preprocess: no offset_sig in {os.path.basename(old_path)}")
        return None

    def _parse_int_field(value, field_name):
        if isinstance(value, int):
            return value
        if isinstance(value, str):
            raw = value.strip()
            if not raw:
                raise ValueError(f"empty {field_name}")
            return int(raw, 0)
        return int(value)

    offset_sig_disp = 0
    try:
        raw_disp = old_data.get("offset_sig_disp")
        if raw_disp is not None:
            offset_sig_disp = _parse_int_field(raw_disp, "offset_sig_disp")
    except Exception:
        if debug:
            print(f"    Preprocess: invalid offset_sig_disp in {os.path.basename(old_path)}")
        return None

    if offset_sig_disp < 0:
        if debug:
            print(f"    Preprocess: offset_sig_disp must be >= 0 in {os.path.basename(old_path)}")
        return None

    old_offset = None
    try:
        if old_data.get("offset") is not None:
            old_offset = _parse_int_field(old_data.get("offset"), "offset")
    except Exception:
        if debug:
            print(f"    Preprocess: invalid offset in {os.path.basename(old_path)}")

    # Search signature in new binary via MCP find_bytes
    try:
        fb_result = await session.call_tool(
            name="find_bytes",
            arguments={"patterns": [offset_sig], "limit": 2}
        )
        fb_data = parse_mcp_result(fb_result)
    except Exception as e:
        if debug:
            print(f"    Preprocess: find_bytes error: {e}")
        return None

    if not isinstance(fb_data, list) or len(fb_data) == 0:
        return None

    entry = fb_data[0]
    matches = entry.get("matches", [])
    match_count = entry.get("n", len(matches))

    if match_count != 1:
        if debug:
            print(f"    Preprocess: {os.path.basename(old_path)} offset sig matched {match_count} (need 1)")
        return None

    match_addr = matches[0]
    expected_offset_expr = "None" if old_offset is None else str(old_offset)

    py_code = (
        "import idaapi, ida_bytes, idautils, ida_ua, json\n"
        f"sig_addr = {match_addr}\n"
        f"inst_addr = sig_addr + {offset_sig_disp}\n"
        f"expected_offset = {expected_offset_expr}\n"
        "insn = idautils.DecodeInstruction(inst_addr)\n"
        "raw = ida_bytes.get_bytes(inst_addr, insn.size) if insn and insn.size > 0 else None\n"
        "if not insn or insn.size <= 0 or not raw:\n"
        "    result = json.dumps(None)\n"
        "else:\n"
        "    candidates = []\n"
        "    for op in insn.ops:\n"
        "        ot = int(op.type)\n"
        "        if ot == int(idaapi.o_void):\n"
        "            continue\n"
        "        if ot not in (int(idaapi.o_displ), int(idaapi.o_mem), int(idaapi.o_imm)):\n"
        "            continue\n"
        "        for attr in ('offb', 'offo'):\n"
        "            off = int(getattr(op, attr, 0))\n"
        "            if off <= 0 or off >= insn.size:\n"
        "                continue\n"
        "            sizes = []\n"
        "            dsz = ida_ua.get_dtype_size(getattr(op, 'dtype', getattr(op, 'dtyp', 0)))\n"
        "            if dsz > 0:\n"
        "                sizes.append(dsz)\n"
        "            for s in (1, 2, 4, 8):\n"
        "                if s not in sizes:\n"
        "                    sizes.append(s)\n"
        "            for sz in sizes:\n"
        "                if off + sz > insn.size:\n"
        "                    continue\n"
        "                chunk = raw[off:off + sz]\n"
        "                unsigned_val = int.from_bytes(chunk, 'little', signed=False)\n"
        "                signed_val = int.from_bytes(chunk, 'little', signed=True)\n"
        "                expected_match = False\n"
        "                if expected_offset is not None:\n"
        "                    expected_mod = expected_offset & ((1 << (8 * sz)) - 1)\n"
        "                    expected_match = unsigned_val == expected_mod or signed_val == expected_offset\n"
        "                candidates.append({\n"
        "                    'off': off,\n"
        "                    'size': sz,\n"
        "                    'unsigned': unsigned_val,\n"
        "                    'signed': signed_val,\n"
        "                    'expected': expected_match,\n"
        "                })\n"
        "    uniq = []\n"
        "    seen = set()\n"
        "    for c in candidates:\n"
        "        key = (c['off'], c['size'])\n"
        "        if key in seen:\n"
        "            continue\n"
        "        seen.add(key)\n"
        "        uniq.append(c)\n"
        "    if not uniq:\n"
        "        result = json.dumps(None)\n"
        "    else:\n"
        "        preferred = [c for c in uniq if c['expected']]\n"
        "        pool = preferred if preferred else uniq\n"
        "        pool.sort(key=lambda c: (c['size'], -c['off']), reverse=True)\n"
        "        best = pool[0]\n"
        "        final_offset = best['signed'] if best['signed'] < 0 else best['unsigned']\n"
        "        result = json.dumps({\n"
        "            'offset': final_offset,\n"
        "            'sig_va': hex(sig_addr),\n"
        "            'inst_va': hex(inst_addr),\n"
        "            'offset_size': best['size'],\n"
        "            'matched_expected': bool(preferred),\n"
        "        })\n"
    )

    try:
        offset_result = await session.call_tool(
            name="py_eval",
            arguments={"code": py_code}
        )
        offset_data = parse_mcp_result(offset_result)
    except Exception as e:
        if debug:
            print(f"    Preprocess: py_eval error: {e}")
        return None

    offset_info = None
    if isinstance(offset_data, dict):
        stderr_text = offset_data.get("stderr", "")
        if stderr_text and debug:
            print("    Preprocess: py_eval stderr:")
            print(stderr_text.strip())
        result_str = offset_data.get("result", "")
        if result_str:
            try:
                offset_info = json.loads(result_str)
            except (json.JSONDecodeError, TypeError):
                pass

    if not isinstance(offset_info, dict) or "offset" not in offset_info:
        if debug:
            print(f"    Preprocess: could not resolve struct offset at {match_addr}")
        return None

    try:
        offset_int = _parse_int_field(offset_info["offset"], "offset")
    except Exception:
        if debug:
            print(f"    Preprocess: invalid parsed offset at {match_addr}")
        return None

    new_data = {
        "struct_name": struct_name,
        "member_name": member_name,
        "offset": hex(offset_int),
        "offset_sig": offset_sig,
    }

    raw_size = old_data.get("size")
    if raw_size is not None:
        try:
            size_value = _parse_int_field(raw_size, "size")
            if size_value > 0:
                new_data["size"] = size_value
        except Exception:
            if debug:
                print(f"    Preprocess: invalid size in {os.path.basename(old_path)}")

    if offset_sig_disp > 0:
        new_data["offset_sig_disp"] = offset_sig_disp

    if debug:
        print(
            "    Preprocess: reused offset_sig at "
            f"{match_addr} for {os.path.basename(new_path)}"
        )

    return new_data


async def preprocess_index_based_vfunc_via_mcp(
    session,
    target_func_name,
    target_output,
    old_yaml_map,
    new_binary_dir,
    platform,
    image_base,
    base_vfunc_name,
    inherit_vtable_class,
    generate_func_sig=True,
    debug=False,
):
    """Resolve an inherited virtual function by base-class slot + vtable lookup.

    Reads the base vfunc YAML referenced by *base_vfunc_name*, then reads
    ``{inherit_vtable_class}_vtable.{platform}.yaml`` to look up the function
    address at the resolved slot index.

    ``base_vfunc_name`` may refer to a stem in the current module directory
    (for example ``"CBaseEntity_Touch"``) or a sibling module artifact under
    the same ``bin/{gamever}`` root (for example
    ``"../server/CFlattenedSerializers_CreateFieldChangedEventQueue"``).

    The base slot is taken from ``vfunc_index`` when present, or derived from
    ``vfunc_offset`` when only the offset exists. If both fields are present,
    they must agree; misaligned offsets and inconsistent metadata are rejected.

    Each target function should specify its own *base_vfunc_name* that maps
    directly to the correct vtable slot. This avoids fragile relative-offset
    calculations that break when the engine inserts new virtual functions
    between existing ones.

    If an old YAML exists for the target, its ``func_sig`` is reused.  Otherwise
    (or when no old YAML is available), a new ``func_sig`` is generated via
    ``preprocess_gen_func_sig_via_mcp`` when *generate_func_sig* is True.

    Args:
        session: Active MCP ClientSession.
        target_func_name: Human-readable name for debug messages.
        target_output: Full path to the expected output YAML.
        old_yaml_map: Mapping from new output path to old version path (may be None).
        new_binary_dir: Directory containing per-binary YAML files.
        platform: ``"windows"`` or ``"linux"``.
        image_base: Binary image base address (int).
        base_vfunc_name: YAML stem of the base-class vfunc in the current
            module or a sibling module under the same ``bin/{gamever}`` root.
            The slot may come from ``vfunc_index`` directly or be derived from
            ``vfunc_offset``.
        inherit_vtable_class: Class name whose vtable is looked up
            (e.g. ``"CTriggerPush"``).
        generate_func_sig: Whether to generate a new func_sig when none can be
            reused from old YAML (default True).
        debug: Enable debug output.

    Returns:
        Dict with function YAML data ready for ``write_func_yaml``, or None on
        failure.
    """
    if yaml is None:
        if debug:
            print("    Preprocess: PyYAML is required for index-based vfunc preprocessing")
        return None

    def _read_yaml(path):
        try:
            with open(path, "r", encoding="utf-8") as f:
                return yaml.safe_load(f)
        except Exception:
            return None

    def _parse_int(value):
        if isinstance(value, int):
            return value
        if isinstance(value, str):
            raw = value.strip()
            if not raw:
                raise ValueError("empty integer string")
            return int(raw, 0)
        return int(value)

    def _resolve_related_yaml_path(binary_dir, artifact_stem, platform_name):
        expanded = f"{artifact_stem}.{platform_name}.yaml"
        module_dir = Path(binary_dir).resolve()
        gamever_dir = module_dir.parent.resolve()
        candidate = (module_dir / expanded).resolve()
        if os.path.commonpath([str(candidate), str(gamever_dir)]) != str(gamever_dir):
            raise ValueError(f"artifact path escapes gamever root: {artifact_stem}")
        return str(candidate)

    def _extract_vfunc_index(data):
        raw_index = data.get("vfunc_index")
        raw_offset = data.get("vfunc_offset")

        if raw_index is None and raw_offset is None:
            raise ValueError("missing vfunc_index/vfunc_offset")

        parsed_index = _parse_int(raw_index) if raw_index is not None else None
        parsed_offset = _parse_int(raw_offset) if raw_offset is not None else None

        if parsed_offset is not None:
            if parsed_offset % 8 != 0:
                raise ValueError("vfunc_offset is not 8-byte aligned")
            offset_index = parsed_offset // 8
            if parsed_index is None:
                parsed_index = offset_index
            elif parsed_index != offset_index:
                raise ValueError("vfunc_index/vfunc_offset mismatch")

        return parsed_index

    # 1. Read base vfunc YAML to get vfunc_index
    try:
        base_vfunc_path = _resolve_related_yaml_path(
            new_binary_dir,
            base_vfunc_name,
            platform,
        )
    except ValueError:
        if debug:
            print(
                "    Preprocess: invalid base vfunc artifact path: "
                f"{base_vfunc_name}"
            )
        return None

    base_vfunc_data = _read_yaml(base_vfunc_path)
    if not isinstance(base_vfunc_data, dict):
        if debug:
            print(
                "    Preprocess: failed to read base vfunc YAML: "
                f"{os.path.basename(base_vfunc_path)}"
            )
        return None

    try:
        base_index = _extract_vfunc_index(base_vfunc_data)
    except Exception:
        if debug:
            print(
                "    Preprocess: invalid vfunc slot metadata in "
                f"{os.path.basename(base_vfunc_path)}"
            )
        return None

    # 2. Read inherit-class vtable YAML
    try:
        vtable_path = _resolve_related_yaml_path(
            new_binary_dir,
            f"{inherit_vtable_class}_vtable",
            platform,
        )
    except ValueError:
        if debug:
            print(
                "    Preprocess: invalid vtable artifact path: "
                f"{inherit_vtable_class}_vtable"
            )
        return None
    vtable_data = _read_yaml(vtable_path)
    if not isinstance(vtable_data, dict):
        if debug:
            print(
                "    Preprocess: failed to read vtable YAML: "
                f"{os.path.basename(vtable_path)}"
            )
        return None

    raw_entries = vtable_data.get("vtable_entries", {})
    if not isinstance(raw_entries, dict):
        if debug:
            print(
                "    Preprocess: invalid vtable_entries in "
                f"{inherit_vtable_class}_vtable YAML"
            )
        return None

    vtable_entries = {}
    for idx, addr in raw_entries.items():
        try:
            vtable_entries[int(idx)] = str(addr)
        except (TypeError, ValueError):
            if debug:
                print(f"    Preprocess: invalid vtable entry index: {idx}")
            return None

    # 3. Look up target function address
    target_index = base_index
    target_addr_hex = vtable_entries.get(target_index)
    if not target_addr_hex:
        if debug:
            print(
                f"    Preprocess: {inherit_vtable_class} vtable missing index "
                f"{target_index} for {target_func_name}"
            )
        return None

    # 4. Query function info via py_eval
    py_code = (
        "import idaapi, json\n"
        f"addr = {target_addr_hex}\n"
        "f = idaapi.get_func(addr)\n"
        "if f and f.start_ea == addr:\n"
        "    result = json.dumps({'func_va': hex(f.start_ea), "
        "'func_size': hex(f.end_ea - f.start_ea)})\n"
        "else:\n"
        "    result = json.dumps(None)\n"
    )

    try:
        result = await session.call_tool(
            name="py_eval",
            arguments={"code": py_code},
        )
        result_data = parse_mcp_result(result)
    except Exception:
        if debug:
            print(f"    Preprocess: py_eval error for {target_func_name}")
        return None

    func_info = None
    if isinstance(result_data, dict):
        result_str = result_data.get("result", "")
        if result_str:
            try:
                func_info = json.loads(result_str)
            except (json.JSONDecodeError, TypeError):
                pass

    if not isinstance(func_info, dict):
        if debug:
            print(f"    Preprocess: failed to query function info for {target_func_name}")
        return None

    func_va_hex = func_info.get("func_va")
    func_size_hex = func_info.get("func_size")
    if not func_va_hex or not func_size_hex:
        if debug:
            print(f"    Preprocess: incomplete function info for {target_func_name}")
        return None

    try:
        func_va_int = int(str(func_va_hex), 16)
    except (TypeError, ValueError):
        if debug:
            print(f"    Preprocess: invalid func_va for {target_func_name}: {func_va_hex}")
        return None

    # 5. Build func_name from base_vfunc_name + inherit_vtable_class
    func_name = target_func_name  # fallback
    base_vtable_name = base_vfunc_data.get("vtable_name")
    if base_vtable_name and base_vfunc_name.startswith(base_vtable_name + "_"):
        method_suffix = base_vfunc_name[len(base_vtable_name) + 1:]
        func_name = f"{inherit_vtable_class}_{method_suffix}"

    # 6. Build payload
    payload = {
        "func_name": func_name,
        "func_va": str(func_va_hex),
        "func_rva": hex(func_va_int - image_base),
        "func_size": str(func_size_hex),
        "vtable_name": inherit_vtable_class,
        "vfunc_offset": hex(target_index * 8),
        "vfunc_index": target_index,
    }

    # 7. Try to reuse old func_sig
    old_path = (old_yaml_map or {}).get(target_output)
    old_func_sig = None
    if old_path and os.path.exists(old_path):
        old_data = _read_yaml(old_path)
        if isinstance(old_data, dict):
            sig = old_data.get("func_sig")
            if sig:
                old_func_sig = str(sig)

    if old_func_sig:
        payload["func_sig"] = old_func_sig
    elif generate_func_sig:
        gen_data = await preprocess_gen_func_sig_via_mcp(
            session=session,
            func_va=func_va_int,
            image_base=image_base,
            debug=debug,
        )
        if gen_data and gen_data.get("func_sig"):
            payload["func_sig"] = gen_data["func_sig"]
        elif debug:
            print(
                f"    Preprocess: func_sig generation failed for "
                f"{target_func_name} at {func_va_hex}"
            )

    return payload


def _read_yaml_file(path):
    """Read YAML file and return loaded object, or None on failure."""
    if yaml is None:
        return None
    try:
        with open(path, "r", encoding="utf-8") as f:
            return yaml.safe_load(f)
    except Exception:
        return None


def _parse_int_value(value):
    """Parse int-like value (int/str/number-like) with base auto-detection."""
    if isinstance(value, int):
        return value
    if isinstance(value, str):
        raw = value.strip()
        if not raw:
            raise ValueError("empty integer string")
        return int(raw, 0)
    return int(value)


def _parse_func_start_set_from_py_eval(eval_data, debug=False):
    """Parse py_eval JSON payload, or return None on invalid payload."""
    if not isinstance(eval_data, dict):
        return None

    stderr_text = eval_data.get("stderr", "")
    if stderr_text and debug:
        print("    Preprocess: py_eval stderr:")
        print(stderr_text.strip())

    result_str = eval_data.get("result", "")
    if not result_str:
        return None

    try:
        parsed = json.loads(result_str)
    except (json.JSONDecodeError, TypeError):
        return None

    if not isinstance(parsed, list):
        return None

    func_starts = set()
    for item in parsed:
        try:
            func_starts.add(_parse_int_value(item))
        except Exception:
            continue
    return func_starts


async def _collect_xref_func_starts_for_string(session, xref_string, debug=False):
    """
    Collect function-start addresses that reference the given string literal.

    ``FULLMATCH:`` prefixes switch the string matcher from substring mode to
    exact-string mode.

    Returns:
        Set[int]: Function start addresses, or None on collection failure.
    """
    if not isinstance(xref_string, str) or not xref_string:
        return set()

    search_str = xref_string
    match_expr = "search_str in current_str"
    if xref_string.startswith("FULLMATCH:"):
        search_str = xref_string[len("FULLMATCH:") :]
        if not search_str:
            return set()
        match_expr = "current_str == search_str"

    py_code = (
        "import idautils, idaapi, json\n"
        f"search_str = {json.dumps(search_str)}\n"
        "func_starts = set()\n"
        "for s in idautils.Strings():\n"
        "    current_str = str(s)\n"
        f"    if {match_expr}:\n"
        "        for xref in idautils.XrefsTo(s.ea, 0):\n"
        "            f = idaapi.get_func(xref.frm)\n"
        "            if f:\n"
        "                func_starts.add(f.start_ea)\n"
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
            print(f"    Preprocess: py_eval error for xref string search: {e}")
        return None

    return _parse_func_start_set_from_py_eval(eval_data, debug=debug)


async def _collect_xref_func_starts_for_ea(session, target_ea, debug=False):
    """
    Collect function-start addresses that reference the specified target address.

    Returns:
        Set[int]: Function start addresses, or None on py_eval failure.
    """
    try:
        target_ea_int = _parse_int_value(target_ea)
    except Exception:
        return set()

    py_code = (
        "import idautils, idaapi, json\n"
        f"target_ea = {target_ea_int}\n"
        "func_starts = set()\n"
        "for xref in idautils.XrefsTo(target_ea, 0):\n"
        "    f = idaapi.get_func(xref.frm)\n"
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
            print(f"    Preprocess: py_eval error for xref ea search: {e}")
        return None

    return _parse_func_start_set_from_py_eval(eval_data, debug=debug)


async def _collect_xref_func_starts_for_signature(
    session, xref_signature, debug=False
):
    """
    Collect function-start addresses that contain bytes matched by a signature.

    Returns:
        Set[int]: Function start addresses, or None on py_eval failure.
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
        return None

    return _parse_func_start_set_from_py_eval(eval_data, debug=debug)


async def _get_func_basic_info_via_mcp(session, func_va, image_base, debug=False):
    """
    Resolve function basic info via IDA py_eval.

    Returns:
        Dict with func_va/func_rva/func_size, or None on failure.
    """
    try:
        func_va_int = _parse_int_value(func_va)
        image_base_int = _parse_int_value(image_base)
    except Exception:
        return None

    py_code = (
        "import idaapi, json\n"
        f"target_ea = {func_va_int}\n"
        "f = idaapi.get_func(target_ea)\n"
        "if f and f.start_ea == target_ea:\n"
        "    result = json.dumps({\n"
        "        'func_va': hex(f.start_ea),\n"
        "        'func_size': hex(f.end_ea - f.start_ea)\n"
        "    })\n"
        "else:\n"
        "    result = json.dumps(None)\n"
    )

    try:
        eval_result = await session.call_tool(
            name="py_eval",
            arguments={"code": py_code},
        )
        eval_data = parse_mcp_result(eval_result)
    except Exception as e:
        if debug:
            print(f"    Preprocess: py_eval error while reading func info: {e}")
        return None

    if not isinstance(eval_data, dict):
        return None

    stderr_text = eval_data.get("stderr", "")
    if stderr_text and debug:
        print("    Preprocess: py_eval stderr:")
        print(stderr_text.strip())

    result_str = eval_data.get("result", "")
    if not result_str:
        return None

    try:
        func_info = json.loads(result_str)
    except (json.JSONDecodeError, TypeError):
        return None

    if not isinstance(func_info, dict):
        return None

    try:
        resolved_va = _parse_int_value(func_info.get("func_va"))
        func_size = _parse_int_value(func_info.get("func_size"))
    except Exception:
        return None

    if resolved_va != func_va_int or func_size <= 0:
        return None

    return {
        "func_va": hex(resolved_va),
        "func_rva": hex(resolved_va - image_base_int),
        "func_size": hex(func_size),
    }


async def _preprocess_direct_func_sig_via_mcp(
    session,
    new_path,
    image_base,
    platform,
    func_name=None,
    direct_func_va=None,
    direct_vtable_class=None,
    direct_vfunc_offset=None,
    direct_vcall_inst_va=None,
    require_func_sig=False,
    require_vfunc_sig=False,
    vfunc_sig_max_match=1,
    normalized_mangled_class_names=None,
    debug=False,
):
    resolved_func_va = None
    vfunc_index = None
    vfunc_offset = None
    vtable_name = None

    if func_name is None:
        func_name = os.path.basename(new_path).rsplit(".", 2)[0]

    if direct_func_va is not None:
        try:
            resolved_func_va = _parse_int_value(direct_func_va)
        except Exception:
            return None

    if direct_vtable_class is not None:
        vtable_data = await preprocess_vtable_via_mcp(
            session=session,
            class_name=direct_vtable_class,
            image_base=image_base,
            platform=platform,
            debug=debug,
            symbol_aliases=_get_mangled_class_aliases(
                normalized_mangled_class_names,
                direct_vtable_class,
            ),
        )
        if not isinstance(vtable_data, dict):
            return None

        if direct_vfunc_offset is None:
            return None

        try:
            offset_value = _parse_int_value(direct_vfunc_offset)
        except Exception:
            return None
        if offset_value < 0 or offset_value % 8 != 0:
            return None

        vfunc_index = offset_value // 8
        try:
            raw_entry = vtable_data.get("vtable_entries", {}).get(vfunc_index)
            resolved_from_vtable = _parse_int_value(raw_entry)
        except Exception:
            return None

        if resolved_func_va is not None and resolved_func_va != resolved_from_vtable:
            return None

        resolved_func_va = resolved_from_vtable
        vfunc_offset = hex(offset_value)
        vtable_name = direct_vtable_class

    if resolved_func_va is None:
        return None

    payload = {
        "func_name": func_name,
        "func_va": hex(resolved_func_va),
        "func_rva": hex(resolved_func_va - image_base),
    }

    if hasattr(session, "call_tool"):
        basic_data = await _get_func_basic_info_via_mcp(
            session=session,
            func_va=resolved_func_va,
            image_base=image_base,
            debug=debug,
        )
        if isinstance(basic_data, dict):
            payload.update(basic_data)

        if require_vfunc_sig:
            if direct_vcall_inst_va is None or direct_vfunc_offset is None:
                if debug:
                    print(
                        "    Preprocess: missing vcall instruction metadata while "
                        f"generating vfunc_sig for {func_name}"
                    )
                return None
            sig_data = await preprocess_gen_vfunc_sig_via_mcp(
                session=session,
                inst_va=direct_vcall_inst_va,
                vfunc_offset=direct_vfunc_offset,
                max_match_count=vfunc_sig_max_match,
                debug=debug,
            )
            if not isinstance(sig_data, dict) or not sig_data.get("vfunc_sig"):
                if debug:
                    print(
                        "    Preprocess: failed to generate direct vfunc_sig for "
                        f"{func_name}"
                    )
                return None
            payload["vfunc_sig"] = str(sig_data["vfunc_sig"])
            payload["vfunc_sig_max_match"] = int(
                sig_data.get("vfunc_sig_max_match", vfunc_sig_max_match)
            )
            if sig_data.get("vfunc_sig_disp") not in (None, 0, "0", "0x0"):
                payload["vfunc_sig_disp"] = sig_data["vfunc_sig_disp"]

        if require_func_sig:
            gen_data = await preprocess_gen_func_sig_via_mcp(
                session=session,
                func_va=resolved_func_va,
                image_base=image_base,
                debug=debug,
            )
            if not isinstance(gen_data, dict) or not gen_data.get("func_sig"):
                if debug:
                    print(
                        "    Preprocess: failed to generate direct func_sig for "
                        f"{func_name}"
                    )
                return None
            payload["func_sig"] = gen_data["func_sig"]

    if vtable_name is not None:
        payload["vtable_name"] = vtable_name
    if vfunc_offset is not None:
        payload["vfunc_offset"] = vfunc_offset
    if vfunc_index is not None:
        payload["vfunc_index"] = vfunc_index

    return payload


async def _preprocess_direct_gv_sig_via_mcp(
    session,
    new_path,
    image_base,
    gv_name=None,
    direct_gv_va=None,
    gv_access_inst_va=None,
    debug=False,
):
    try:
        resolved_gv_va = _parse_int_value(direct_gv_va)
    except Exception:
        return None

    payload = await preprocess_gen_gv_sig_via_mcp(
        session=session,
        gv_va=resolved_gv_va,
        image_base=image_base,
        gv_access_inst_va=gv_access_inst_va,
        debug=debug,
    )
    if not isinstance(payload, dict):
        return None

    if gv_name is None:
        gv_name = os.path.basename(new_path).rsplit(".", 2)[0]
    payload["gv_name"] = gv_name
    return payload


async def _preprocess_direct_struct_offset_sig_via_mcp(
    session,
    new_path,
    image_base,
    struct_member_name=None,
    struct_name=None,
    member_name=None,
    offset=None,
    offset_inst_va=None,
    size=None,
    old_path=None,
    debug=False,
):
    metadata = _load_struct_member_metadata_from_yaml(old_path)

    resolved_struct_name = str(
        struct_name
        or metadata.get("struct_name", "")
        or ""
    ).strip()
    resolved_member_name = str(
        member_name
        or metadata.get("member_name", "")
        or ""
    ).strip()

    if not resolved_struct_name or not resolved_member_name:
        if debug:
            print(
                "    Preprocess: missing struct_name/member_name for direct "
                f"struct offset generation of {struct_member_name or new_path}"
            )
        return None

    resolved_size = metadata.get("size")
    if resolved_size is None and size is not None:
        try:
            parsed_size = _parse_int_value(size)
        except Exception:
            parsed_size = None
        if isinstance(parsed_size, int) and parsed_size > 0:
            resolved_size = parsed_size

    payload = await preprocess_gen_struct_offset_sig_via_mcp(
        session=session,
        struct_name=resolved_struct_name,
        member_name=resolved_member_name,
        offset=offset,
        offset_inst_va=offset_inst_va,
        image_base=image_base,
        size=resolved_size,
        debug=debug,
    )
    if not isinstance(payload, dict):
        return None

    if "offset_sig_disp" not in payload:
        payload["offset_sig_disp"] = 0

    return payload


async def preprocess_gen_struct_offset_sig_via_mcp(
    session,
    struct_name,
    member_name,
    offset,
    offset_inst_va,
    image_base,
    size=None,
    min_sig_bytes=8,
    max_sig_bytes=96,
    max_instructions=64,
    extra_wildcard_offsets=None,
    debug=False,
):
    _ = image_base

    try:
        offset_value = _parse_int_value(offset)
        offset_inst_va_int = _parse_int_value(offset_inst_va)
        min_sig_bytes = max(1, int(min_sig_bytes))
        max_sig_bytes = max(1, int(max_sig_bytes))
        max_instructions = max(1, int(max_instructions))
    except Exception:
        return None

    resolved_size = None
    if size is not None:
        try:
            parsed_size = _parse_int_value(size)
        except Exception:
            parsed_size = None
        if isinstance(parsed_size, int) and parsed_size > 0:
            resolved_size = parsed_size

    extra_wildcards = set()
    for item in extra_wildcard_offsets or []:
        try:
            parsed = _parse_int_value(item)
        except Exception:
            continue
        if parsed >= 0:
            extra_wildcards.add(parsed)

    py_code = (
        "import idaapi, ida_bytes, idautils, ida_ua, json\n"
        f"target_inst = {offset_inst_va_int}\n"
        f"max_sig_bytes = {max_sig_bytes}\n"
        f"max_instructions = {max_instructions}\n"
        "func = idaapi.get_func(target_inst)\n"
        "if not func:\n"
        "    result = json.dumps([])\n"
        "else:\n"
        "    cursor = target_inst\n"
        "    total = 0\n"
        "    insts = []\n"
        "    while cursor < func.end_ea and len(insts) < max_instructions and total < max_sig_bytes:\n"
        "        insn = idautils.DecodeInstruction(cursor)\n"
        "        if not insn or insn.size <= 0:\n"
        "            break\n"
        "        raw = ida_bytes.get_bytes(cursor, insn.size)\n"
        "        if not raw:\n"
        "            break\n"
        "        wild = set()\n"
        "        for op in insn.ops:\n"
        "            op_type = int(op.type)\n"
        "            if op_type == int(idaapi.o_void):\n"
        "                continue\n"
        "            offb = int(getattr(op, 'offb', 0))\n"
        "            offo = int(getattr(op, 'offo', 0))\n"
        "            dtype_size = ida_ua.get_dtype_size(getattr(op, 'dtype', getattr(op, 'dtyp', 0)))\n"
        "            if dtype_size <= 0:\n"
        "                dtype_size = 4\n"
        "            if op_type in (int(idaapi.o_imm), int(idaapi.o_displ), int(idaapi.o_mem), int(idaapi.o_near), int(idaapi.o_far)):\n"
        "                if offb > 0 and offb < insn.size:\n"
        "                    for idx in range(offb, min(insn.size, offb + dtype_size)):\n"
        "                        wild.add(idx)\n"
        "                if offo > 0 and offo < insn.size:\n"
        "                    for idx in range(offo, min(insn.size, offo + dtype_size)):\n"
        "                        wild.add(idx)\n"
        "        b0 = raw[0]\n"
        "        if b0 in (0xE8, 0xE9, 0xEB):\n"
        "            for idx in range(1, insn.size):\n"
        "                wild.add(idx)\n"
        "        elif b0 == 0x0F and insn.size >= 2 and (raw[1] & 0xF0) == 0x80:\n"
        "            for idx in range(2, insn.size):\n"
        "                wild.add(idx)\n"
        "        elif 0x70 <= b0 <= 0x7F:\n"
        "            for idx in range(1, insn.size):\n"
        "                wild.add(idx)\n"
        "        insts.append({'size': insn.size, 'bytes': raw.hex(), 'wild': sorted(wild)})\n"
        "        cursor += insn.size\n"
        "        total += insn.size\n"
        "    result = json.dumps([{'offset_inst_va': hex(target_inst), 'insts': insts}] if insts else [])\n"
    )
    try:
        result = await session.call_tool(
            name="py_eval",
            arguments={"code": py_code},
        )
        result_data = parse_mcp_result(result)
    except Exception:
        return None

    candidate_infos = None
    if isinstance(result_data, dict):
        result_str = result_data.get("result", "")
        if result_str:
            try:
                candidate_infos = json.loads(result_str)
            except (json.JSONDecodeError, TypeError):
                candidate_infos = None

    if not isinstance(candidate_infos, list) or not candidate_infos:
        return None

    for candidate in candidate_infos:
        try:
            inst_va = _parse_int_value(candidate.get("offset_inst_va"))
            insts = candidate.get("insts", [])
        except Exception:
            continue
        if inst_va != offset_inst_va_int or not isinstance(insts, list) or not insts:
            continue

        sig_tokens = []
        inst_boundaries = []
        malformed = False
        for inst in insts:
            try:
                inst_size = int(inst.get("size", 0))
                inst_hex = str(inst.get("bytes", ""))
                inst_wild = {int(item) for item in inst.get("wild", [])}
            except Exception:
                malformed = True
                break
            if inst_size <= 0 or len(inst_hex) != inst_size * 2:
                malformed = True
                break

            inst_bytes = [
                int(inst_hex[idx:idx + 2], 16)
                for idx in range(0, len(inst_hex), 2)
            ]
            base_offset = len(sig_tokens)
            for rel_idx, value in enumerate(inst_bytes):
                abs_offset = base_offset + rel_idx
                if rel_idx in inst_wild or abs_offset in extra_wildcards:
                    sig_tokens.append("??")
                else:
                    sig_tokens.append(f"{value:02X}")
            inst_boundaries.append(len(sig_tokens))

        if malformed:
            continue

        for prefix_len in inst_boundaries:
            if prefix_len < min_sig_bytes:
                continue
            prefix_tokens = sig_tokens[:prefix_len]
            if all(token == "??" for token in prefix_tokens):
                continue

            candidate_sig = " ".join(prefix_tokens)
            try:
                fb_result = await session.call_tool(
                    name="find_bytes",
                    arguments={"patterns": [candidate_sig], "limit": 2},
                )
                fb_data = parse_mcp_result(fb_result)
            except Exception:
                return None

            if not isinstance(fb_data, list) or not fb_data:
                continue
            entry = fb_data[0]
            matches = entry.get("matches", [])
            match_count = entry.get("n", len(matches))
            if match_count != 1 or not matches:
                continue

            try:
                match_addr = _parse_int_value(matches[0])
            except Exception:
                continue
            if match_addr != offset_inst_va_int:
                continue

            payload = {
                "struct_name": struct_name,
                "member_name": member_name,
                "offset": hex(offset_value),
                "offset_sig": candidate_sig,
                "offset_sig_disp": 0,
            }
            if resolved_size is not None:
                payload["size"] = resolved_size
            return payload

    if debug:
        print(
            "    Preprocess: failed to generate struct offset sig for "
            f"{struct_name}.{member_name}"
        )
    return None


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
    """
    Resolve target function by intersecting candidate sets from:
    - string xrefs
    - byte signatures mapped to containing function starts
    - xrefs to dependency function addresses from current-version YAML files.
    - vtable entries (when ``vtable_class`` is specified)

    Additionally, ``exclude_funcs`` can provide function names whose
    ``func_va`` values are loaded from current-version YAML files in
    ``new_binary_dir`` and removed from the intersection result before
    enforcing the uniqueness check.

    ``exclude_strings`` uses the same string-matching semantics as
    ``xref_strings``. For each configured string, collect the containing
    function-start set from its xrefs, union these sets, and subtract the
    result from the already-intersected positive candidate set. Empty
    exclude-string matches are treated as an empty exclusion set rather
    than a hard failure. ``FULLMATCH:`` prefixes switch string matching
    from substring mode to exact-string mode.

    When ``vtable_class`` is given, the vtable YAML file
    ``{vtable_class}_vtable.{platform}.yaml`` is loaded from
    ``new_binary_dir`` and all vtable entry addresses are added as an
    additional candidate set, so only functions that are virtual
    functions of that class survive the intersection.
    """
    dep_func_names = xref_funcs or []
    excluded_func_names = exclude_funcs or []
    if dep_func_names or excluded_func_names or vtable_class:
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

    # Collect candidates from vtable entries
    if vtable_class:
        vtable_yaml_path = os.path.join(
            new_binary_dir, f"{vtable_class}_vtable.{platform}.yaml"
        )
        vtable_data = _read_yaml_file(vtable_yaml_path)
        if not isinstance(vtable_data, dict):
            if debug:
                print(
                    f"    Preprocess: vtable YAML missing or invalid: "
                    f"{os.path.basename(vtable_yaml_path)}"
                )
            return None

        vtable_entries = vtable_data.get("vtable_entries", {})
        vtable_addr_set = set()
        for _idx, addr in vtable_entries.items():
            try:
                vtable_addr_set.add(int(str(addr), 16))
            except (TypeError, ValueError):
                continue

        if not vtable_addr_set:
            if debug:
                print(
                    f"    Preprocess: empty vtable entries for "
                    f"{vtable_class}"
                )
            return None

        if debug:
            print(
                f"    Preprocess: vtable {vtable_class} has "
                f"{len(vtable_addr_set)} entries as candidate set"
            )
        candidate_sets.append(vtable_addr_set)

    for xref_string in (xref_strings or []):
        addr_set = await _collect_xref_func_starts_for_string(
            session=session, xref_string=xref_string, debug=debug
        )
        if addr_set is None:
            if debug:
                short = str(xref_string)[:80]
                print(f"    Preprocess: failed to collect string xref: {short}")
            return None
        if not addr_set:
            if debug:
                short = str(xref_string)[:80]
                print(f"    Preprocess: empty candidate set for string xref: {short}")
            return None
        candidate_sets.append(addr_set)

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
                    f"    Preprocess: empty candidate set for signature xref: {short}"
                )
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

    excluded_func_addrs = set()
    for excluded_func_name in excluded_func_names:
        excluded_yaml_path = os.path.join(
            new_binary_dir, f"{excluded_func_name}.{platform}.yaml"
        )
        excluded_data = _read_yaml_file(excluded_yaml_path)
        if not isinstance(excluded_data, dict):
            if debug:
                print(
                    f"    Preprocess: excluded func YAML missing or invalid: "
                    f"{os.path.basename(excluded_yaml_path)}"
                )
            return None

        try:
            excluded_func_va = _parse_int_value(excluded_data.get("func_va"))
        except Exception:
            if debug:
                print(
                    f"    Preprocess: invalid func_va in excluded func YAML: "
                    f"{os.path.basename(excluded_yaml_path)}"
                )
            return None
        excluded_func_addrs.add(excluded_func_va)

    if not candidate_sets:
        if debug:
            print(
                f"    Preprocess: no xref candidate sources configured for {func_name}"
            )
        return None

    common_funcs = set(candidate_sets[0])
    for addr_set in candidate_sets[1:]:
        common_funcs = common_funcs & addr_set

    excluded_string_func_addrs = set()
    for excluded_string in (exclude_strings or []):
        addr_set = await _collect_xref_func_starts_for_string(
            session=session,
            xref_string=excluded_string,
            debug=debug,
        )
        if addr_set is None:
            if debug:
                short = str(excluded_string)[:80]
                print(
                    f"    Preprocess: failed to collect exclude string xref: {short}"
                )
            return None
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

    if len(common_funcs) != 1:
        if debug:
            print(
                f"    Preprocess: xref intersection yielded {len(common_funcs)} "
                f"function(s) for {func_name} (need exactly 1)"
            )
        return None

    target_va = next(iter(common_funcs))

    sig_data = await preprocess_gen_func_sig_via_mcp(
        session=session,
        func_va=target_va,
        image_base=image_base,
        debug=debug,
    )

    if isinstance(sig_data, dict) and sig_data.get("func_sig"):
        sig_data["func_name"] = func_name
        return sig_data

    basic_data = await _get_func_basic_info_via_mcp(
        session=session,
        func_va=target_va,
        image_base=image_base,
        debug=debug,
    )
    if not isinstance(basic_data, dict):
        return None

    basic_data["func_name"] = func_name
    return basic_data


# ---------------------------------------------------------------------------
# Common preprocess_skill template
# ---------------------------------------------------------------------------


async def _rename_func_in_ida(session, func_va_hex, func_name, debug=False):
    """Best-effort rename of a function address in IDA via MCP rename tool."""
    if not func_va_hex or not func_name:
        return
    try:
        await session.call_tool(
            name="rename",
            arguments={"batch": {"func": {"addr": str(func_va_hex), "name": func_name}}},
        )
        if debug:
            print(f"    Preprocess: renamed func {func_va_hex} -> {func_name}")
    except Exception as e:
        if debug:
            print(f"    Preprocess: failed to rename func {func_va_hex} -> {func_name}: {e}")


async def _rename_gv_in_ida(session, gv_va_hex, gv_name, debug=False):
    """Best-effort rename of a global variable address in IDA via py_eval."""
    if not gv_va_hex or not gv_name:
        return
    try:
        gv_va_int = int(gv_va_hex, 16)
        await session.call_tool(
            name="py_eval",
            arguments={"code": f"import idc; idc.set_name({gv_va_int}, \"{gv_name}\", idc.SN_NOWARN)"},
        )
        if debug:
            print(f"    Preprocess: renamed gv {gv_va_hex} -> {gv_name}")
    except Exception as e:
        if debug:
            print(f"    Preprocess: failed to rename gv {gv_va_hex} -> {gv_name}: {e}")


async def _try_preprocess_func_without_llm(
    *,
    session,
    target_output,
    old_path,
    image_base,
    new_binary_dir,
    platform,
    func_name,
    func_xrefs_map,
    vtable_relations_map,
    normalized_mangled_class_names,
    debug=False,
):
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
        if debug:
            print(f"    Preprocess: trying func_xrefs fallback for {func_name}")
        xref_vtable_class = None
        if func_name in vtable_relations_map:
            xref_vtable_class = vtable_relations_map[func_name]
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

    return func_data


def _can_probe_future_func_fast_path(
    *,
    func_name,
    func_xrefs_map,
    new_binary_dir,
    platform,
    debug=False,
):
    xref_spec = (func_xrefs_map or {}).get(func_name)
    if not isinstance(xref_spec, dict):
        return True

    dependency_func_names = list(xref_spec.get("xref_funcs") or []) + list(
        xref_spec.get("exclude_funcs") or []
    )
    if not dependency_func_names:
        return True

    if not new_binary_dir:
        if debug:
            print(
                f"    Preprocess: skip fast-path probing for {func_name}, "
                "new_binary_dir unavailable for xref dependency check"
            )
        return False

    try:
        new_binary_dir_path = os.fspath(new_binary_dir)
    except Exception:
        if debug:
            print(
                f"    Preprocess: skip fast-path probing for {func_name}, "
                "invalid new_binary_dir for xref dependency check"
            )
        return False

    for dependency_func_name in dependency_func_names:
        dependency_yaml_path = os.path.join(
            new_binary_dir_path,
            f"{dependency_func_name}.{platform}.yaml",
        )
        if not os.path.isfile(dependency_yaml_path):
            if debug:
                print(
                    f"    Preprocess: skip fast-path probing for {func_name}, "
                    f"xref dependency YAML not ready: {dependency_yaml_path}"
                )
            return False

    return True


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
    generate_yaml_desired_fields=None,
    llm_decompile_specs=None,
    llm_config=None,
    mangled_class_names=None,
    debug=False,
):
    """Reusable preprocess_skill implementation for func/vfunc, gv, patch, struct-member, vtable, inherit-vfunc, func-xref, and vtable-relation targets.

    Handles any combination of the eight target types in a single call:
    - ``func_names``: func/vfunc targets via ``preprocess_func_sig_via_mcp``
      (which already supports vfunc_sig fallback internally).
    - ``gv_names``: global-variable targets via ``preprocess_gv_sig_via_mcp``.
    - ``patch_names``: patch targets via ``preprocess_patch_via_mcp``.
    - ``struct_member_names``: struct-member offset targets via
      ``preprocess_struct_offset_sig_via_mcp``.
    - ``vtable_class_names``: vtable targets via ``preprocess_vtable_via_mcp``.
    - ``mangled_class_names``: optional mapping from canonical vtable class
      names to explicit mangled symbol aliases. These aliases are tried before
      auto-derived vtable symbols and RTTI fallback.
    - ``inherit_vfuncs``: inherited virtual function targets resolved by
      base-class vfunc_index + vtable lookup via
      ``preprocess_index_based_vfunc_via_mcp``.  Each element is a tuple of
      ``(target_func_name, inherit_vtable_class, base_vfunc_name)`` or
      ``(target_func_name, inherit_vtable_class, base_vfunc_name, generate_func_sig)``.
      When *generate_func_sig* is omitted it defaults to ``True``.
      For each target, ``preprocess_func_sig_via_mcp`` is attempted first
      (reusing an existing ``func_sig`` from old YAML); the index-based
      fallback is used only when that fails.
    - ``func_xrefs``: locate functions via unified xref fallback through
      ``preprocess_func_xrefs_via_mcp``. Each element is a tuple of
      ``(func_name, xref_strings_list, xref_signatures_list, xref_funcs_list,``
      ``exclude_funcs_list, exclude_strings_list)``. ``xref_strings`` provides string
      cross-reference constraints, ``xref_signatures`` provides byte-pattern
      constraints mapped to containing function starts, and ``xref_funcs``
      provides dependency function names whose addresses are read only from
      current-version YAML files in ``new_binary_dir``. ``exclude_funcs``
      provides function names whose YAML ``func_va`` values are removed from
      the intersection set before uniqueness check. Used as a fallback when
      ``preprocess_func_sig_via_mcp`` fails for a func target that has a
      matching func-xref entry, or as the sole resolution method for func
      targets that only appear in this list.
    - ``func_vtable_relations``: enrich located function YAML with vtable
      metadata.  Each element is a tuple of ``(func_name, vtable_class)``.
    - ``generate_yaml_desired_fields``: required list of
      ``(symbol_name, desired_field_names)`` tuples that defines the exact YAML
      payload fields to emit per symbol.

    Args:
        session: Active MCP ClientSession.
        expected_outputs: List of expected output YAML paths.
        old_yaml_map: Mapping from new output path to old version path.
        new_binary_dir: Directory for new version outputs.
        platform: "windows" or "linux".
        image_base: Binary image base address (int).
        func_names: List of function/vfunc target names (may be empty/None).
        gv_names: List of global-variable target names (may be empty/None).
        patch_names: List of patch target names (may be empty/None).
        struct_member_names: List of struct-member target names (may be empty/None).
        vtable_class_names: List of class names for vtable lookup, or None.
        mangled_class_names: Mapping from canonical vtable class name to
            explicit mangled aliases for vtable lookup (may be empty/None).
        inherit_vfuncs: List of inherited vfunc specs (may be empty/None).
        func_xrefs: List of
            (func_name, xref_strings_list, xref_signatures_list,
            xref_funcs_list, exclude_funcs_list, exclude_strings_list) tuples for unified
            xref-based function lookup (may be empty/None). Dependency and
            exclude function addresses are read only from current-version
            YAML files.
        func_vtable_relations: List of (func_name, vtable_class) tuples for
            enriching function YAML with vtable metadata (may be empty/None).
        generate_yaml_desired_fields: Required desired output fields per
            symbol name.
        llm_decompile_specs: Optional LLM decompile spec tuples of
            (func_name, prompt_path, reference_yaml_path).
        llm_config: Optional LLM config dict for llm_decompile fallback.
        debug: Enable debug output.

    Returns:
        True if all targets were successfully preprocessed, False otherwise.
    """
    func_names = func_names or []
    gv_names = gv_names or []
    patch_names = patch_names or []
    struct_member_names = struct_member_names or []
    vtable_class_names = vtable_class_names or []
    inherit_vfuncs = inherit_vfuncs or []
    func_xrefs = func_xrefs or []
    func_vtable_relations = func_vtable_relations or []
    llm_decompile_specs = llm_decompile_specs or []
    normalized_mangled_class_names = _normalize_mangled_class_names(
        mangled_class_names,
        debug=debug,
    )
    if normalized_mangled_class_names is None:
        return False
    desired_fields_map = _normalize_generate_yaml_desired_fields(
        generate_yaml_desired_fields,
        debug=debug,
    )
    if desired_fields_map is None:
        return False
    llm_decompile_specs_map = _build_llm_decompile_specs_map(
        llm_decompile_specs,
        debug=debug,
    )
    if llm_decompile_specs_map is None:
        return False

    func_xrefs_map = {}
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

        if not isinstance(xref_signatures, (tuple, list)):
            if debug:
                print(
                    f"    Preprocess: invalid xref_signatures type for "
                    f"{func_name}: {type(xref_signatures).__name__}"
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
        if any(not isinstance(item, str) or not item for item in xref_strings):
            if debug:
                print(f"    Preprocess: invalid xref_strings values for {func_name}")
            return False

        if any(not isinstance(item, str) or not item for item in xref_signatures):
            if debug:
                print(
                    f"    Preprocess: invalid xref_signatures values for {func_name}"
                )
            return False

        if any(not isinstance(item, str) or not item for item in xref_funcs):
            if debug:
                print(f"    Preprocess: invalid xref_funcs values for {func_name}")
            return False

        if any(not isinstance(item, str) or not item for item in exclude_funcs):
            if debug:
                print(f"    Preprocess: invalid exclude_funcs values for {func_name}")
            return False

        if any(not isinstance(item, str) or not item for item in exclude_strings):
            if debug:
                print(
                    f"    Preprocess: invalid exclude_strings values for {func_name}"
                )
            return False

        if not xref_strings and not xref_signatures and not xref_funcs:
            if debug:
                print(f"    Preprocess: empty func_xrefs spec for {func_name}")
            return False

        func_xrefs_map[func_name] = {
            "xref_strings": xref_strings,
            "xref_signatures": xref_signatures,
            "xref_funcs": xref_funcs,
            "exclude_funcs": exclude_funcs,
            "exclude_strings": exclude_strings,
        }

    target_kind_map = _build_target_kind_map(
        func_names,
        gv_names,
        patch_names,
        struct_member_names,
        vtable_class_names,
        inherit_vfuncs,
        func_xrefs_map,
        debug=debug,
    )
    if target_kind_map is None:
        return False

    for symbol_name, desired_field_spec in desired_fields_map.items():
        target_kind = target_kind_map.get(symbol_name)
        if target_kind is None:
            if debug:
                print(f"    Preprocess: unknown desired-fields symbol: {symbol_name}")
            return False
        desired_fields = desired_field_spec["desired_output_fields"]
        allowed_fields = TARGET_KIND_TO_FIELD_SET[target_kind]
        invalid_fields = [
            field_name for field_name in desired_fields
            if field_name not in allowed_fields
        ]
        if invalid_fields:
            if debug:
                print(
                    f"    Preprocess: invalid desired fields for {symbol_name}: "
                    f"{invalid_fields}"
                )
            return False

    for symbol_name in target_kind_map:
        if symbol_name not in desired_fields_map:
            if debug:
                print(
                    f"    Preprocess: missing desired-fields for target symbol: "
                    f"{symbol_name}"
                )
            return False

    # Build vtable-relation lookup: func_name -> vtable_class
    vtable_relations_map = {}
    for spec in func_vtable_relations:
        if not isinstance(spec, (tuple, list)) or len(spec) != 2:
            if debug:
                print(f"    Preprocess: invalid func_vtable_relations spec: {spec}")
            return False
        func_name, vtable_class = spec
        if not isinstance(func_name, str) or not func_name:
            if debug:
                print(f"    Preprocess: invalid func_vtable_relations target: {func_name}")
            return False
        if not isinstance(vtable_class, str) or not vtable_class:
            if debug:
                print(f"    Preprocess: invalid func_vtable_relations class: {vtable_class}")
            return False
        vtable_relations_map[func_name] = vtable_class

    pending_func_renames = []
    pending_gv_renames = []

    # --- vtable targets ---
    for vtable_class in vtable_class_names:
        target_filename = f"{vtable_class}_vtable.{platform}.yaml"
        target_outputs = [
            path for path in expected_outputs
            if os.path.basename(path) == target_filename
        ]

        if len(target_outputs) != 1:
            if debug:
                print(
                    f"    Preprocess: expected exactly one output named {target_filename}, "
                    f"got {len(target_outputs)}"
                )
            return False

        vtable_data = await preprocess_vtable_via_mcp(
            session=session,
            class_name=vtable_class,
            image_base=image_base,
            platform=platform,
            debug=debug,
            symbol_aliases=_get_mangled_class_aliases(
                normalized_mangled_class_names,
                vtable_class,
            ),
        )
        if vtable_data is None:
            return False

        payload = _assemble_symbol_payload(
            vtable_class,
            "vtable",
            vtable_data,
            desired_fields_map,
            debug=debug,
        )
        if payload is None:
            return False
        write_vtable_yaml(target_outputs[0], payload)
        if debug:
            print(f"    Preprocess: generated {target_filename}")

    # --- inherit-vfunc targets ---
    if inherit_vfuncs:
        iv_expected_by_filename = {}
        for spec in inherit_vfuncs:
            func_name = spec[0]
            iv_expected_by_filename[f"{func_name}.{platform}.yaml"] = spec

        iv_matched = {}
        for path in expected_outputs:
            basename = os.path.basename(path)
            matched_spec = iv_expected_by_filename.get(basename)
            if matched_spec is not None:
                iv_matched[matched_spec[0]] = path

        missing_iv = [s[0] for s in inherit_vfuncs if s[0] not in iv_matched]
        if missing_iv:
            if debug:
                print(
                    "    Preprocess: expected outputs missing for "
                    f"{', '.join(missing_iv)}"
                )
            return False

        for spec in inherit_vfuncs:
            func_name = spec[0]
            vtable_class = spec[1]
            base_vfunc_name = spec[2]
            gen_func_sig = spec[3] if len(spec) > 3 else True

            target_output = iv_matched[func_name]
            old_path = (old_yaml_map or {}).get(target_output)

            # Try reusing old func_sig first (fast path).
            func_data = None
            if old_path:
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

            # Fallback: resolve via base-class vfunc_index + vtable lookup.
            if func_data is None:
                func_data = await preprocess_index_based_vfunc_via_mcp(
                    session=session,
                    target_func_name=func_name,
                    target_output=target_output,
                    old_yaml_map=old_yaml_map,
                    new_binary_dir=new_binary_dir,
                    platform=platform,
                    image_base=image_base,
                    base_vfunc_name=base_vfunc_name,
                    inherit_vtable_class=vtable_class,
                    generate_func_sig=gen_func_sig,
                    debug=debug,
                )
            if func_data is None:
                if debug:
                    print(f"    Preprocess: failed to locate {func_name}")
                return False

            payload = _assemble_symbol_payload(
                func_name,
                "func",
                func_data,
                desired_fields_map,
                debug=debug,
            )
            if payload is None:
                return False
            write_func_yaml(target_output, payload)
            pending_func_renames.append((func_data.get("func_va"), func_name))
            if debug:
                print(f"    Preprocess: generated {func_name}.{platform}.yaml")

    # --- func/vfunc + gv + patch + struct-member targets ---
    # Merge func_names with func-xref-only targets so that functions that
    # appear exclusively in func_xrefs (and not in func_names) are also
    # processed through the func pipeline.
    xref_only_names = [
        name for name in func_xrefs_map if name not in func_names
    ]
    all_func_names = list(func_names) + xref_only_names

    if not all_func_names and not gv_names and not patch_names and not struct_member_names:
        for func_va_hex, func_name in pending_func_renames:
            await _rename_func_in_ida(session, func_va_hex, func_name, debug)
        for gv_va_hex, gv_name in pending_gv_renames:
            await _rename_gv_in_ida(session, gv_va_hex, gv_name, debug)
        return True

    # Build expected filename -> (kind, name) mapping
    expected_by_filename = {
        f"{func_name}.{platform}.yaml": ("func", func_name)
        for func_name in all_func_names
    }
    for gv_name in gv_names:
        expected_by_filename[f"{gv_name}.{platform}.yaml"] = ("gv", gv_name)
    for patch_name in patch_names:
        expected_by_filename[f"{patch_name}.{platform}.yaml"] = ("patch", patch_name)
    for struct_member_name in struct_member_names:
        expected_by_filename[f"{struct_member_name}.{platform}.yaml"] = ("struct_member", struct_member_name)

    # Match expected outputs
    matched_func_outputs = {}
    matched_gv_outputs = {}
    matched_patch_outputs = {}
    matched_struct_outputs = {}
    for path in expected_outputs:
        basename = os.path.basename(path)
        item = expected_by_filename.get(basename)
        if item is None:
            continue
        kind, name = item
        if kind == "func":
            matched_func_outputs[name] = path
        elif kind == "gv":
            matched_gv_outputs[name] = path
        elif kind == "patch":
            matched_patch_outputs[name] = path
        elif kind == "struct_member":
            matched_struct_outputs[name] = path

    # Validate all expected outputs are present
    missing_func = [n for n in all_func_names if n not in matched_func_outputs]
    missing_gv = [n for n in gv_names if n not in matched_gv_outputs]
    missing_patch = [n for n in patch_names if n not in matched_patch_outputs]
    missing_struct = [n for n in struct_member_names if n not in matched_struct_outputs]
    if missing_func or missing_gv or missing_patch or missing_struct:
        if debug:
            missing = missing_func + missing_gv + missing_patch + missing_struct
            print(
                "    Preprocess: expected outputs missing for "
                f"{', '.join(missing)}"
            )
        return False

    llm_request_cache = {}
    llm_result_by_symbol_name = {}
    fast_path_attempted = {}
    fast_path_results = {}
    gv_fast_path_attempted = {}
    gv_fast_path_results = {}
    struct_fast_path_attempted = {}
    struct_fast_path_results = {}

    async def _ensure_gv_fast_path(gv_name):
        if gv_name in gv_fast_path_attempted:
            return gv_fast_path_results.get(gv_name)

        target_output = matched_gv_outputs.get(gv_name)
        gv_fast_path_attempted[gv_name] = True
        if target_output is None:
            gv_fast_path_results[gv_name] = None
            return None

        gv_old_path = (old_yaml_map or {}).get(target_output)
        gv_fast_path_results[gv_name] = await preprocess_gv_sig_via_mcp(
            session=session,
            new_path=target_output,
            old_path=gv_old_path,
            image_base=image_base,
            new_binary_dir=new_binary_dir,
            platform=platform,
            debug=debug,
        )
        return gv_fast_path_results.get(gv_name)

    async def _collect_llm_symbol_name_list(seed_symbol_name, llm_cache_key):
        llm_symbol_name_list = []

        for candidate_func_name in all_func_names:
            if candidate_func_name not in llm_decompile_specs_map:
                continue
            if candidate_func_name in llm_result_by_symbol_name:
                continue
            candidate_target_output = matched_func_outputs.get(candidate_func_name)
            if candidate_target_output is None:
                continue
            if candidate_func_name not in fast_path_attempted:
                if not _can_probe_future_func_fast_path(
                    func_name=candidate_func_name,
                    func_xrefs_map=func_xrefs_map,
                    new_binary_dir=new_binary_dir,
                    platform=platform,
                    debug=debug,
                ):
                    continue
                candidate_old_path = (old_yaml_map or {}).get(candidate_target_output)
                fast_path_results[candidate_func_name] = await _try_preprocess_func_without_llm(
                    session=session,
                    target_output=candidate_target_output,
                    old_path=candidate_old_path,
                    image_base=image_base,
                    new_binary_dir=new_binary_dir,
                    platform=platform,
                    func_name=candidate_func_name,
                    func_xrefs_map=func_xrefs_map,
                    vtable_relations_map=vtable_relations_map,
                    normalized_mangled_class_names=normalized_mangled_class_names,
                    debug=debug,
                )
                fast_path_attempted[candidate_func_name] = True
            if fast_path_results.get(candidate_func_name) is not None:
                continue
            if candidate_func_name not in llm_request_cache:
                llm_request_cache[candidate_func_name] = _prepare_llm_decompile_request(
                    candidate_func_name,
                    llm_decompile_specs_map,
                    llm_config,
                    platform=platform,
                    debug=debug,
                )
            candidate_request = llm_request_cache.get(candidate_func_name)
            if (
                _build_llm_decompile_request_cache_key(candidate_request)
                == llm_cache_key
            ):
                llm_symbol_name_list.append(candidate_func_name)

        for candidate_gv_name in gv_names:
            if candidate_gv_name not in llm_decompile_specs_map:
                continue
            if candidate_gv_name in llm_result_by_symbol_name:
                continue
            candidate_target_output = matched_gv_outputs.get(candidate_gv_name)
            if candidate_target_output is None:
                continue
            if candidate_gv_name not in gv_fast_path_attempted:
                await _ensure_gv_fast_path(candidate_gv_name)
            if gv_fast_path_results.get(candidate_gv_name) is not None:
                continue
            if candidate_gv_name not in llm_request_cache:
                llm_request_cache[candidate_gv_name] = _prepare_llm_decompile_request(
                    candidate_gv_name,
                    llm_decompile_specs_map,
                    llm_config,
                    platform=platform,
                    debug=debug,
                )
            candidate_request = llm_request_cache.get(candidate_gv_name)
            if (
                _build_llm_decompile_request_cache_key(candidate_request)
                == llm_cache_key
            ):
                llm_symbol_name_list.append(candidate_gv_name)

        for candidate_struct_name in struct_member_names:
            if candidate_struct_name not in llm_decompile_specs_map:
                continue
            if candidate_struct_name in llm_result_by_symbol_name:
                continue
            candidate_target_output = matched_struct_outputs.get(candidate_struct_name)
            if candidate_target_output is None:
                continue
            if candidate_struct_name not in struct_fast_path_attempted:
                candidate_old_path = (old_yaml_map or {}).get(candidate_target_output)
                struct_fast_path_results[candidate_struct_name] = await preprocess_struct_offset_sig_via_mcp(
                    session=session,
                    new_path=candidate_target_output,
                    old_path=candidate_old_path,
                    image_base=image_base,
                    new_binary_dir=new_binary_dir,
                    platform=platform,
                    debug=debug,
                )
                struct_fast_path_attempted[candidate_struct_name] = True
            if struct_fast_path_results.get(candidate_struct_name) is not None:
                continue
            if candidate_struct_name not in llm_request_cache:
                llm_request_cache[candidate_struct_name] = _prepare_llm_decompile_request(
                    candidate_struct_name,
                    llm_decompile_specs_map,
                    llm_config,
                    platform=platform,
                    debug=debug,
                )
            candidate_request = llm_request_cache.get(candidate_struct_name)
            if (
                _build_llm_decompile_request_cache_key(candidate_request)
                == llm_cache_key
            ):
                llm_symbol_name_list.append(candidate_struct_name)

        if not llm_symbol_name_list:
            llm_symbol_name_list = [seed_symbol_name]
        return llm_symbol_name_list

    # Process func/vfunc targets
    for func_name in all_func_names:
        target_output = matched_func_outputs[func_name]
        old_path = (old_yaml_map or {}).get(target_output)
        desired_field_spec = desired_fields_map.get(func_name)
        if desired_field_spec is None:
            if debug:
                print(f"    Preprocess: missing desired-fields entry for {func_name}")
            return False
        desired_fields = desired_field_spec["desired_output_fields"]
        generation_options = desired_field_spec["generation_options"]
        desired_fields_set = set(desired_fields)
        can_use_direct_func_fallback = "vfunc_sig" not in desired_fields_set

        if func_name not in fast_path_attempted:
            fast_path_results[func_name] = await _try_preprocess_func_without_llm(
                session=session,
                target_output=target_output,
                old_path=old_path,
                image_base=image_base,
                new_binary_dir=new_binary_dir,
                platform=platform,
                func_name=func_name,
                func_xrefs_map=func_xrefs_map,
                vtable_relations_map=vtable_relations_map,
                normalized_mangled_class_names=normalized_mangled_class_names,
                debug=debug,
            )
            fast_path_attempted[func_name] = True
        func_data = fast_path_results.get(func_name)

        if func_data is None and func_name in llm_decompile_specs_map:
            if func_name not in llm_request_cache:
                llm_request_cache[func_name] = _prepare_llm_decompile_request(
                    func_name,
                    llm_decompile_specs_map,
                    llm_config,
                    platform=platform,
                    debug=debug,
                )
            llm_request = llm_request_cache.get(func_name)
            llm_cache_key = _build_llm_decompile_request_cache_key(llm_request)

            if func_name in llm_result_by_symbol_name:
                llm_result = llm_result_by_symbol_name[func_name]
            elif llm_request is None or llm_cache_key is None:
                llm_result = _empty_llm_decompile_result()
            else:
                llm_symbol_name_list = await _collect_llm_symbol_name_list(
                    func_name,
                    llm_cache_key,
                )

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
                            symbol_name_list=llm_symbol_name_list,
                            disasm_code=llm_target_detail["disasm_code"],
                            procedure=llm_target_detail["procedure"],
                            disasm_for_reference=llm_request["disasm_for_reference"],
                            procedure_for_reference=llm_request["procedure_for_reference"],
                            prompt_template=llm_request["prompt_template"],
                            platform=platform,
                            temperature=llm_request.get("temperature"),
                            debug=debug,
                        )
                except Exception:
                    llm_result = _empty_llm_decompile_result()
                for symbol_name in llm_symbol_name_list:
                    llm_result_by_symbol_name[symbol_name] = llm_result
            if can_use_direct_func_fallback:
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
            if func_data is None and can_use_direct_func_fallback:
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
            vtable_class = None
            if func_data is None and func_name in vtable_relations_map:
                vtable_class = vtable_relations_map[func_name]
            for entry in llm_result.get("found_vcall", []):
                if vtable_class is None:
                    break
                if entry.get("func_name") != func_name:
                    continue
                func_data = await _preprocess_direct_func_sig_via_mcp(
                    session=session,
                    new_path=target_output,
                    image_base=image_base,
                    platform=platform,
                    func_name=func_name,
                    direct_vtable_class=vtable_class,
                    direct_vfunc_offset=entry.get("vfunc_offset"),
                    direct_vcall_inst_va=entry.get("insn_va"),
                    require_func_sig="func_sig" in desired_fields_set,
                    require_vfunc_sig="vfunc_sig" in desired_fields_set,
                    vfunc_sig_max_match=generation_options.get("vfunc_sig_max_match", 1),
                    normalized_mangled_class_names=normalized_mangled_class_names,
                    debug=debug,
                )
                if func_data is not None:
                    break
            if func_data is None:
                fallback_vtable_name = None
                if func_name in vtable_relations_map:
                    fallback_vtable_name = vtable_relations_map[func_name]
                func_data = await _build_enriched_slot_only_vfunc_payload_via_mcp(
                    session=session,
                    func_name=func_name,
                    llm_result=llm_result,
                    vtable_name=fallback_vtable_name,
                    vfunc_sig_max_match=generation_options.get("vfunc_sig_max_match", 1),
                    require_vfunc_sig="vfunc_sig" in desired_fields_set,
                    require_vtable_name="vtable_name" in desired_fields_set,
                    debug=debug,
                )

        if func_data is None:
            if debug:
                print(f"    Preprocess: failed to locate {func_name}")
            return False

        # Enrich with vtable metadata if a vtable relation is defined.
        if "vtable_name" in desired_fields_set and func_name in vtable_relations_map:
            func_data["vtable_name"] = vtable_relations_map[func_name]

        need_vfunc_slot = bool({"vfunc_offset", "vfunc_index"} & desired_fields_set)
        need_compute_slot = need_vfunc_slot and (
            "vfunc_offset" not in func_data or "vfunc_index" not in func_data
        )
        if need_compute_slot:
            vtable_class = vtable_relations_map.get(func_name) or func_data.get("vtable_name")
            if not isinstance(vtable_class, str) or not vtable_class:
                if debug:
                    print(
                        f"    Preprocess: missing vtable class for slot enrichment of {func_name}"
                    )
                return False

            func_va_hex = func_data.get("func_va")
            try:
                func_va_int = int(str(func_va_hex), 16)
            except (TypeError, ValueError):
                if debug:
                    print(
                        f"    Preprocess: invalid func_va for slot enrichment of "
                        f"{func_name}: {func_va_hex}"
                    )
                return False

            vtable_data = await preprocess_vtable_via_mcp(
                session=session,
                class_name=vtable_class,
                image_base=image_base,
                platform=platform,
                debug=debug,
                symbol_aliases=_get_mangled_class_aliases(
                    normalized_mangled_class_names,
                    vtable_class,
                ),
            )
            if vtable_data is None:
                if debug:
                    print(
                        f"    Preprocess: failed to look up {vtable_class} "
                        f"vtable for {func_name}"
                    )
                return False

            vtable_entries = vtable_data.get("vtable_entries", {})
            matched_index = None
            for idx, addr in vtable_entries.items():
                try:
                    if int(str(addr), 16) == func_va_int:
                        matched_index = int(idx)
                        break
                except (TypeError, ValueError):
                    continue

            if matched_index is None:
                if debug:
                    print(
                        f"    Preprocess: {func_name} at {func_va_hex} "
                        f"not found in {vtable_class} vtable entries"
                    )
                return False

            func_data["vfunc_offset"] = hex(matched_index * 8)
            func_data["vfunc_index"] = matched_index
            if debug:
                print(
                    f"    Preprocess: {func_name} matched "
                    f"{vtable_class} vtable index {matched_index} "
                    f"(offset {hex(matched_index * 8)})"
                )

        payload = _assemble_symbol_payload(
            func_name,
            "func",
            func_data,
            desired_fields_map,
            debug=debug,
        )
        if payload is None:
            return False
        write_func_yaml(target_output, payload)
        pending_func_renames.append((func_data.get("func_va"), func_name))
        if debug:
            print(f"    Preprocess: generated {func_name}.{platform}.yaml")

    # Process gv targets
    for gv_name in gv_names:
        target_output = matched_gv_outputs[gv_name]
        if gv_name not in gv_fast_path_attempted:
            await _ensure_gv_fast_path(gv_name)
        gv_data = gv_fast_path_results.get(gv_name)

        if gv_data is None and gv_name in llm_decompile_specs_map:
            if gv_name not in llm_request_cache:
                llm_request_cache[gv_name] = _prepare_llm_decompile_request(
                    gv_name,
                    llm_decompile_specs_map,
                    llm_config,
                    platform=platform,
                    debug=debug,
                )
            llm_request = llm_request_cache.get(gv_name)
            llm_cache_key = _build_llm_decompile_request_cache_key(llm_request)

            if gv_name in llm_result_by_symbol_name:
                llm_result = llm_result_by_symbol_name[gv_name]
            elif llm_request is None or llm_cache_key is None:
                llm_result = _empty_llm_decompile_result()
            else:
                llm_symbol_name_list = await _collect_llm_symbol_name_list(
                    gv_name,
                    llm_cache_key,
                )
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
                            symbol_name_list=llm_symbol_name_list,
                            disasm_code=llm_target_detail["disasm_code"],
                            procedure=llm_target_detail["procedure"],
                            disasm_for_reference=llm_request["disasm_for_reference"],
                            procedure_for_reference=llm_request["procedure_for_reference"],
                            prompt_template=llm_request["prompt_template"],
                            platform=platform,
                            temperature=llm_request.get("temperature"),
                            debug=debug,
                        )
                except Exception:
                    llm_result = _empty_llm_decompile_result()
                for symbol_name in llm_symbol_name_list:
                    llm_result_by_symbol_name[symbol_name] = llm_result

            for entry in llm_result.get("found_gv", []):
                if entry.get("gv_name") != gv_name:
                    continue
                direct_gv_va = await _resolve_direct_gv_target_via_mcp(
                    session,
                    entry.get("insn_va"),
                    debug=debug,
                )
                if direct_gv_va is None:
                    continue
                gv_data = await _preprocess_direct_gv_sig_via_mcp(
                    session=session,
                    new_path=target_output,
                    image_base=image_base,
                    gv_name=gv_name,
                    direct_gv_va=direct_gv_va,
                    gv_access_inst_va=entry.get("insn_va"),
                    debug=debug,
                )
                if gv_data is not None:
                    break

        if gv_data is None:
            if debug:
                print(f"    Preprocess: failed to locate {gv_name}")
            return False

        payload = _assemble_symbol_payload(
            gv_name,
            "gv",
            gv_data,
            desired_fields_map,
            debug=debug,
        )
        if payload is None:
            return False
        write_gv_yaml(target_output, payload)
        pending_gv_renames.append((gv_data.get("gv_va"), gv_name))
        if debug:
            print(f"    Preprocess: generated {gv_name}.{platform}.yaml")

    # Process patch targets
    for patch_name in patch_names:
        target_output = matched_patch_outputs[patch_name]
        patch_old_path = (old_yaml_map or {}).get(target_output)

        patch_data = await preprocess_patch_via_mcp(
            session=session,
            new_path=target_output,
            old_path=patch_old_path,
            image_base=image_base,
            new_binary_dir=new_binary_dir,
            platform=platform,
            debug=debug,
        )

        if patch_data is None:
            if debug:
                print(f"    Preprocess: failed to locate {patch_name}")
            return False

        payload = _assemble_symbol_payload(
            patch_name,
            "patch",
            patch_data,
            desired_fields_map,
            debug=debug,
        )
        if payload is None:
            return False
        write_patch_yaml(target_output, payload)
        if debug:
            print(f"    Preprocess: generated {patch_name}.{platform}.yaml")

    # Process struct-member targets
    for struct_member_name in struct_member_names:
        target_output = matched_struct_outputs[struct_member_name]
        struct_old_path = (old_yaml_map or {}).get(target_output)
        if struct_member_name not in struct_fast_path_attempted:
            struct_fast_path_results[struct_member_name] = await preprocess_struct_offset_sig_via_mcp(
                session=session,
                new_path=target_output,
                old_path=struct_old_path,
                image_base=image_base,
                new_binary_dir=new_binary_dir,
                platform=platform,
                debug=debug,
            )
            struct_fast_path_attempted[struct_member_name] = True
        struct_data = struct_fast_path_results.get(struct_member_name)

        if struct_data is None and struct_member_name in llm_decompile_specs_map:
            if struct_member_name not in llm_request_cache:
                llm_request_cache[struct_member_name] = _prepare_llm_decompile_request(
                    struct_member_name,
                    llm_decompile_specs_map,
                    llm_config,
                    platform=platform,
                    debug=debug,
                )
            llm_request = llm_request_cache.get(struct_member_name)
            llm_cache_key = _build_llm_decompile_request_cache_key(llm_request)

            if struct_member_name in llm_result_by_symbol_name:
                llm_result = llm_result_by_symbol_name[struct_member_name]
            elif llm_request is None or llm_cache_key is None:
                llm_result = _empty_llm_decompile_result()
            else:
                llm_symbol_name_list = await _collect_llm_symbol_name_list(
                    struct_member_name,
                    llm_cache_key,
                )
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
                            symbol_name_list=llm_symbol_name_list,
                            disasm_code=llm_target_detail["disasm_code"],
                            procedure=llm_target_detail["procedure"],
                            disasm_for_reference=llm_request["disasm_for_reference"],
                            procedure_for_reference=llm_request["procedure_for_reference"],
                            prompt_template=llm_request["prompt_template"],
                            platform=platform,
                            temperature=llm_request.get("temperature"),
                            debug=debug,
                        )
                except Exception:
                    llm_result = _empty_llm_decompile_result()
                for symbol_name in llm_symbol_name_list:
                    llm_result_by_symbol_name[symbol_name] = llm_result

            struct_metadata = _load_struct_member_metadata_from_yaml(struct_old_path)
            expected_struct_name = str(struct_metadata.get("struct_name", "")).strip()
            expected_member_name = str(struct_metadata.get("member_name", "")).strip()
            for entry in llm_result.get("found_struct_offset", []):
                entry_struct_name = str(entry.get("struct_name", "")).strip()
                entry_member_name = str(entry.get("member_name", "")).strip()
                if expected_struct_name and expected_member_name:
                    if (
                        entry_struct_name != expected_struct_name
                        or entry_member_name != expected_member_name
                    ):
                        continue
                else:
                    if (
                        _build_struct_member_symbol_name(
                            entry_struct_name,
                            entry_member_name,
                        )
                        != struct_member_name
                    ):
                        continue

                struct_data = await _preprocess_direct_struct_offset_sig_via_mcp(
                    session=session,
                    new_path=target_output,
                    image_base=image_base,
                    struct_member_name=struct_member_name,
                    struct_name=entry_struct_name,
                    member_name=entry_member_name,
                    offset=entry.get("offset"),
                    offset_inst_va=entry.get("insn_va"),
                    old_path=struct_old_path,
                    debug=debug,
                    size=entry.get("size"),
                )
                if struct_data is not None:
                    break

        if struct_data is None:
            if debug:
                print(f"    Preprocess: failed to locate {struct_member_name}")
            return False

        payload = _assemble_symbol_payload(
            struct_member_name,
            "struct_member",
            struct_data,
            desired_fields_map,
            debug=debug,
        )
        if payload is None:
            return False
        write_struct_offset_yaml(target_output, payload)
        if debug:
            print(f"    Preprocess: generated {struct_member_name}.{platform}.yaml")

    for func_va_hex, func_name in pending_func_renames:
        await _rename_func_in_ida(session, func_va_hex, func_name, debug)
    for gv_va_hex, gv_name in pending_gv_renames:
        await _rename_gv_in_ida(session, gv_va_hex, gv_name, debug)

    return True
