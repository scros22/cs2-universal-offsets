#!/usr/bin/env python3
"""Shared preprocess helpers for DEFINE_INPUTFUNC-like skills."""

import json
import os

from ida_analyze_util import (
    parse_mcp_result,
    preprocess_gen_func_sig_via_mcp,
    write_func_yaml,
)


def _normalize_requested_fields(generate_yaml_desired_fields, target_name, debug=False):
    if not generate_yaml_desired_fields:
        if debug:
            print("    Preprocess: missing generate_yaml_desired_fields")
        return None

    desired_map = {}
    for symbol_name, fields in generate_yaml_desired_fields:
        desired_map[symbol_name] = list(fields)

    fields = desired_map.get(target_name)
    if not fields:
        if debug:
            print(f"    Preprocess: missing desired fields for {target_name}")
        return None
    return fields


def _resolve_output_path(expected_outputs, target_name, platform, debug=False):
    filename = f"{target_name}.{platform}.yaml"
    matches = [
        path for path in expected_outputs if os.path.basename(path) == filename
    ]
    if len(matches) != 1:
        if debug:
            print(f"    Preprocess: expected exactly one output for {filename}")
        return None
    return matches[0]


def _normalize_addr(value):
    if value is None or isinstance(value, bool):
        return None
    try:
        if isinstance(value, str):
            raw = value.strip()
            if not raw:
                return None
            return hex(int(raw, 0))
        return hex(int(value))
    except (TypeError, ValueError):
        return None


def _normalize_segment_names(allowed_segment_names):
    if isinstance(allowed_segment_names, str):
        values = [allowed_segment_names]
    else:
        try:
            values = list(allowed_segment_names)
        except TypeError:
            return None
    normalized = tuple(str(value) for value in values if isinstance(value, str) and value)
    if not normalized:
        return None
    return normalized


def _build_func_payload(target_name, requested_fields, func_info, extra_fields):
    merged = {"func_name": target_name}
    merged.update(func_info)
    merged.update(extra_fields)

    payload = {}
    for field in requested_fields:
        if field not in merged:
            raise KeyError(field)
        payload[field] = merged[field]
    return payload


def _build_define_inputfunc_py_eval(
    input_name,
    handler_ptr_offset=0x10,
    allowed_segment_names=(".data",),
):
    normalized_segments = _normalize_segment_names(allowed_segment_names)
    if normalized_segments is None:
        normalized_segments = ()
    params = json.dumps(
        {
            "input_name": input_name,
            "handler_ptr_offset": int(handler_ptr_offset),
            "allowed_segment_names": list(normalized_segments),
        }
    )
    body_lines = [
        "import idaapi, idautils, idc, ida_bytes",
        "input_name = params['input_name']",
        f"handler_ptr_offset = {int(handler_ptr_offset)}",
        "allowed_segment_names = set(params['allowed_segment_names'])",
        "string_eas = []",
        "items = []",
        "def _seg_name(ea):",
        "    seg = idaapi.getseg(ea)",
        "    if not seg:",
        "        return None",
        "    return idc.get_segm_name(seg.start_ea)",
        "for item in idautils.Strings():",
        "    try:",
        "        if str(item) == input_name:",
        "            string_eas.append(hex(int(item.ea)))",
        "    except Exception:",
        "        pass",
        "if len(string_eas) == 1:",
        "    string_ea = int(string_eas[0], 16)",
        "    for xref in idautils.XrefsTo(string_ea, 0):",
        "        xref_from = int(xref.frm)",
        "        xref_seg_name = _seg_name(xref_from)",
        "        if xref_seg_name not in allowed_segment_names:",
        "            continue",
        "        handler_ptr_ea = xref_from + handler_ptr_offset",
        "        try:",
        "            handler_va = int(ida_bytes.get_qword(handler_ptr_ea))",
        "        except Exception:",
        "            continue",
        "        handler_seg_name = _seg_name(handler_va)",
        "        if handler_seg_name == '.text':",
        "            items.append({",
        "                'string_ea': hex(string_ea),",
        "                'xref_from': hex(xref_from),",
        "                'xref_seg_name': xref_seg_name,",
        "                'handler_ptr_ea': hex(handler_ptr_ea),",
        "                'handler_va': hex(handler_va),",
        "                'handler_seg_name': handler_seg_name,",
        "            })",
        "return {'string_eas': string_eas, 'items': items}",
    ]
    lines = [
        "import json, traceback",
        f"params = json.loads({params!r})",
        "def _collect_candidates(params):",
    ]
    lines.extend(f"    {line}" for line in body_lines)
    lines.extend(
        [
            "try:",
            "    collected = _collect_candidates(params)",
            "    result = json.dumps({",
            "        'ok': True,",
            "        'string_eas': collected['string_eas'],",
            "        'items': collected['items'],",
            "    })",
            "except Exception:",
            "    result = json.dumps({",
            "        'ok': False,",
            "        'traceback': traceback.format_exc(),",
            "    })",
        ]
    )
    return "\n".join(lines) + "\n"


async def _call_py_eval_json(session, code, debug=False, error_label="py_eval"):
    try:
        result = await session.call_tool(name="py_eval", arguments={"code": code})
        result_data = parse_mcp_result(result)
    except Exception:
        if debug:
            print(f"    Preprocess: {error_label} error")
        return None
    if isinstance(result_data, dict):
        raw = result_data.get("result", "")
    elif result_data is not None:
        raw = str(result_data)
    else:
        raw = ""
    if not raw:
        return None
    try:
        return json.loads(raw)
    except (TypeError, json.JSONDecodeError):
        if debug:
            print(f"    Preprocess: invalid JSON result from {error_label}")
        return None


async def _collect_define_inputfunc_candidates(
    session,
    input_name,
    handler_ptr_offset=0x10,
    allowed_segment_names=(".data",),
    debug=False,
):
    code = _build_define_inputfunc_py_eval(
        input_name=input_name,
        handler_ptr_offset=handler_ptr_offset,
        allowed_segment_names=allowed_segment_names,
    )
    parsed = await _call_py_eval_json(
        session=session,
        code=code,
        debug=debug,
        error_label="py_eval collecting DEFINE_INPUTFUNC candidates",
    )
    if not isinstance(parsed, dict) or parsed.get("ok") is not True:
        if debug and isinstance(parsed, dict):
            traceback_text = parsed.get("traceback")
            if isinstance(traceback_text, str) and traceback_text.strip():
                print(traceback_text.rstrip())
        return None

    string_eas = parsed.get("string_eas")
    items = parsed.get("items")
    if not isinstance(string_eas, list) or len(string_eas) != 1:
        return None
    if not isinstance(items, list) or not items:
        return None

    normalized_items = []
    required_keys = {
        "string_ea",
        "xref_from",
        "xref_seg_name",
        "handler_ptr_ea",
        "handler_va",
        "handler_seg_name",
    }
    for item in items:
        if not isinstance(item, dict) or not required_keys.issubset(item):
            return None
        if item.get("handler_seg_name") != ".text":
            return None
        normalized = dict(item)
        for key in ("string_ea", "xref_from", "handler_ptr_ea", "handler_va"):
            addr = _normalize_addr(normalized.get(key))
            if addr is None:
                return None
            normalized[key] = addr
        if not isinstance(normalized.get("xref_seg_name"), str):
            return None
        normalized_items.append(normalized)

    normalized_string_ea = _normalize_addr(string_eas[0])
    if normalized_string_ea is None:
        return None

    return {"string_eas": [normalized_string_ea], "items": normalized_items}


async def _query_func_info(session, handler_va, debug=False):
    fi_code = (
        "import idaapi, json\n"
        f"addr = {handler_va}\n"
        "f = idaapi.get_func(addr)\n"
        "if f and f.start_ea == addr:\n"
        "    result = json.dumps({'func_va': hex(f.start_ea), "
        "'func_size': hex(f.end_ea - f.start_ea)})\n"
        "else:\n"
        "    result = json.dumps(None)\n"
    )
    parsed = await _call_py_eval_json(
        session=session,
        code=fi_code,
        debug=debug,
        error_label=f"py_eval querying func info for {handler_va}",
    )
    if not isinstance(parsed, dict):
        return None
    if "func_va" not in parsed or "func_size" not in parsed:
        return None
    return {"func_va": parsed["func_va"], "func_size": parsed["func_size"]}


async def _rename_func_best_effort(session, func_va, func_name, debug=False):
    if not func_va or not func_name:
        return
    try:
        await session.call_tool(
            name="rename",
            arguments={"batch": {"func": {"addr": str(func_va), "name": str(func_name)}}},
        )
    except Exception:
        if debug:
            print(f"    Preprocess: failed to rename {func_name} (non-fatal)")


async def preprocess_define_inputfunc_skill(
    session,
    expected_outputs,
    platform,
    image_base,
    target_name,
    input_name,
    generate_yaml_desired_fields,
    handler_ptr_offset=0x10,
    allowed_segment_names=(".data",),
    rename_to=None,
    debug=False,
):
    if not isinstance(target_name, str) or not target_name:
        return False
    if not isinstance(input_name, str) or not input_name:
        return False
    try:
        handler_ptr_offset = int(handler_ptr_offset)
    except (TypeError, ValueError):
        return False
    if handler_ptr_offset < 0:
        return False
    allowed_segment_names = _normalize_segment_names(allowed_segment_names)
    if allowed_segment_names is None:
        return False
    try:
        image_base_int = int(str(image_base), 0)
    except (TypeError, ValueError):
        return False

    requested_fields = _normalize_requested_fields(
        generate_yaml_desired_fields,
        target_name,
        debug=debug,
    )
    if requested_fields is None:
        return False
    output_path = _resolve_output_path(
        expected_outputs,
        target_name,
        platform,
        debug=debug,
    )
    if output_path is None:
        return False

    candidates = await _collect_define_inputfunc_candidates(
        session=session,
        input_name=input_name,
        handler_ptr_offset=handler_ptr_offset,
        allowed_segment_names=allowed_segment_names,
        debug=debug,
    )
    if not isinstance(candidates, dict):
        return False

    items = candidates.get("items")
    if not isinstance(items, list):
        return False
    filtered_items = [
        item
        for item in items
        if item.get("xref_seg_name") in allowed_segment_names
        and item.get("handler_seg_name") == ".text"
    ]
    handler_values = sorted({item.get("handler_va") for item in filtered_items})
    if len(handler_values) != 1:
        if debug:
            print(
                f"    Preprocess: expected exactly one .text handler for {input_name}, got {len(handler_values)}"
            )
        return False

    handler_va = handler_values[0]
    func_info = await _query_func_info(session, handler_va, debug=debug)
    if not isinstance(func_info, dict):
        return False

    extra_fields = {}
    if "func_rva" in requested_fields:
        try:
            extra_fields["func_rva"] = hex(int(str(func_info["func_va"]), 0) - image_base_int)
        except (KeyError, TypeError, ValueError):
            return False
    if "func_sig" in requested_fields:
        sig_info = await preprocess_gen_func_sig_via_mcp(
            session=session,
            func_va=handler_va,
            image_base=image_base_int,
            debug=debug,
        )
        if not sig_info:
            return False
        try:
            extra_fields["func_sig"] = sig_info["func_sig"]
            extra_fields["func_rva"] = sig_info["func_rva"]
            extra_fields["func_size"] = sig_info["func_size"]
        except KeyError:
            return False

    try:
        payload = _build_func_payload(target_name, requested_fields, func_info, extra_fields)
    except KeyError:
        return False

    write_func_yaml(output_path, payload)
    await _rename_func_best_effort(
        session=session,
        func_va=handler_va,
        func_name=rename_to,
        debug=debug,
    )
    return True
