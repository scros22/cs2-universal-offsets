#!/usr/bin/env python3
"""Shared preprocess helpers for RegisterConCommand-like skills."""

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


def _build_registerconcommand_py_eval(
    platform,
    command_name,
    help_string,
    search_window_before_call,
    search_window_after_xref,
):
    params = json.dumps(
        {
            "platform": platform,
            "command_name": command_name,
            "help_string": help_string,
            "search_window_before_call": search_window_before_call,
            "search_window_after_xref": search_window_after_xref,
        }
    )
    body_lines = [
        "import idaapi, idautils, idc, ida_bytes",
        "platform = params['platform']",
        f"search_window_before_call = {int(search_window_before_call)}",
        f"search_window_after_xref = {int(search_window_after_xref)}",
        "command_name = params['command_name']",
        "help_string = params['help_string']",
        "candidates = []",
        "seen_calls = set()",
        "seen_candidates = set()",
        "handler_slot_addr = None",
        "slot_value_addr = None",
        "reg_names_linux = [('rsi', 'esi'), ('rdx', 'edx'), ('r8', 'r8d')]",
        "reg_names_windows = [('rdx', 'edx'), ('r8', 'r8d'), ('r9', 'r9d')]",
        "def _scan_exact_strings(target_text):",
        "    if not target_text:",
        "        return []",
        "    hits = []",
        "    for item in idautils.Strings():",
        "        try:",
        "            if str(item) == target_text:",
        "                hits.append(int(item.ea))",
        "        except Exception:",
        "            pass",
        "    return hits",
        "def _read_string(ea):",
        "    if ea in (None, 0, idaapi.BADADDR):",
        "        return None",
        "    try:",
        "        raw = idc.get_strlit_contents(ea, -1, idc.STRTYPE_C)",
        "    except Exception:",
        "        raw = None",
        "    if raw is None:",
        "        return None",
        "    if isinstance(raw, bytes):",
        "        return raw.decode('utf-8', errors='ignore')",
        "    return str(raw)",
        "def _is_registerconcommand_call(ea):",
        "    if idc.print_insn_mnem(ea) not in ('call', 'jmp'):",
        "        return False",
        "    operand = idc.print_operand(ea, 0) or ''",
        "    line = idc.generate_disasm_line(ea, 0) or ''",
        "    return 'RegisterConCommand' in operand or 'RegisterConCommand' in line",
        "def _prev_head_in_window(start_ea, min_ea):",
        "    cur = idc.prev_head(start_ea, min_ea)",
        "    while cur != idaapi.BADADDR and cur >= min_ea:",
        "        yield cur",
        "        next_cur = idc.prev_head(cur, min_ea)",
        "        if next_cur == cur:",
        "            break",
        "        cur = next_cur",
        "def _expand_reg_names(reg_name):",
        "    names = []",
        "    raw = (reg_name or '').lower()",
        "    if raw:",
        "        names.append(raw)",
        "        if raw.startswith('r') and len(raw) == 3 and raw[1:].isdigit():",
        "            names.append(raw + 'd')",
        "        elif raw.startswith('r') and len(raw) == 3 and raw[1].isalpha():",
        "            names.append('e' + raw[1:])",
        "        elif raw.startswith('e') and len(raw) == 3:",
        "            names.append('r' + raw[1:])",
        "    return tuple(dict.fromkeys(names))",
        "def _recover_register_value(call_ea, reg_names):",
        "    min_ea = max(0, call_ea - search_window_before_call)",
        "    for cur in _prev_head_in_window(call_ea, min_ea):",
        "        mnem = idc.print_insn_mnem(cur)",
        "        op0 = (idc.print_operand(cur, 0) or '').lower()",
        "        if mnem not in ('mov', 'lea') or op0 not in reg_names:",
        "            continue",
        "        op_type = int(idc.get_operand_type(cur, 1))",
        "        if op_type in (int(idaapi.o_imm), int(idaapi.o_mem), int(idaapi.o_near), int(idaapi.o_far), int(idaapi.o_displ)):",
        "            value = idc.get_operand_value(cur, 1)",
        "            if value not in (None, idaapi.BADADDR):",
        "                return value",
        "    return None",
        "def _recover_stack_slot(call_ea, reg_names):",
        "    min_ea = max(0, call_ea - search_window_before_call)",
        "    for cur in _prev_head_in_window(call_ea, min_ea):",
        "        mnem = idc.print_insn_mnem(cur)",
        "        op0 = (idc.print_operand(cur, 0) or '').lower()",
        "        if mnem != 'lea' or op0 not in reg_names:",
        "            continue",
        "        if int(idc.get_operand_type(cur, 1)) == int(idaapi.o_displ):",
        "            value = idc.get_operand_value(cur, 1)",
        "            if value not in (None, idaapi.BADADDR):",
        "                return value",
        "    return None",
        "def _recover_slot_value(call_ea, slot_addr):",
        "    if slot_addr in (None, 0, idaapi.BADADDR):",
        "        return None",
        "    min_ea = max(0, call_ea - search_window_before_call)",
        "    for cur in _prev_head_in_window(call_ea, min_ea):",
        "        if idc.print_insn_mnem(cur) != 'mov':",
        "            continue",
        "        if int(idc.get_operand_type(cur, 0)) != int(idaapi.o_displ):",
        "            continue",
        "        if idc.get_operand_value(cur, 0) != slot_addr:",
        "            continue",
        "        op1_type = int(idc.get_operand_type(cur, 1))",
        "        if op1_type in (int(idaapi.o_imm), int(idaapi.o_mem), int(idaapi.o_near), int(idaapi.o_far)):",
        "            value = idc.get_operand_value(cur, 1)",
        "            if value not in (None, idaapi.BADADDR):",
        "                return value",
        "        reg_name = idc.print_operand(cur, 1) or ''",
        "        if reg_name:",
        "            value = _recover_register_value(cur, _expand_reg_names(reg_name))",
        "            if value not in (None, idaapi.BADADDR):",
        "                return value",
        "    return None",
        "def _append_candidate(command_value, help_value, handler_va):",
        "    if handler_va in (None, 0, idaapi.BADADDR):",
        "        return",
        "    key = (command_value, help_value, int(handler_va))",
        "    if key in seen_candidates:",
        "        return",
        "    seen_candidates.add(key)",
        "    candidates.append({'command_name': command_value, 'help_string': help_value, 'handler_va': hex(int(handler_va))})",
        "def _analyze_call(call_ea):",
        "    if platform == 'windows':",
        "        command_addr = _recover_register_value(call_ea, reg_names_windows[0])",
        "        handler_slot_addr = _recover_stack_slot(call_ea, reg_names_windows[1])",
        "        help_addr = _recover_register_value(call_ea, reg_names_windows[2])",
        "        slot_value_addr = _recover_slot_value(call_ea, handler_slot_addr)",
        "        handler_va = slot_value_addr",
        "    else:",
        "        command_addr = _recover_register_value(call_ea, reg_names_linux[0])",
        "        handler_va = _recover_register_value(call_ea, reg_names_linux[1])",
        "        help_addr = _recover_register_value(call_ea, reg_names_linux[2])",
        "    command_value = _read_string(command_addr)",
        "    help_value = _read_string(help_addr)",
        "    if command_name is not None and command_value != command_name:",
        "        return",
        "    if help_string is not None and help_value != help_string:",
        "        return",
        "    _append_candidate(command_value, help_value, handler_va)",
        "command_string_addrs = _scan_exact_strings(command_name)",
        "help_string_addrs = _scan_exact_strings(help_string)",
        "seed_string_addrs = []",
        "seed_string_addrs.extend(command_string_addrs)",
        "seed_string_addrs.extend(help_string_addrs)",
        "xref_heads = set()",
        "for string_ea in seed_string_addrs:",
        "    for xref in idautils.XrefsTo(string_ea, 0):",
        "        xref_ea = int(xref.frm)",
        "        if not idc.is_code(ida_bytes.get_full_flags(xref_ea)):",
        "            continue",
        "        xref_heads.add(xref_ea)",
        "for xref_ea in sorted(xref_heads):",
        "    search_end = xref_ea + search_window_after_xref",
        "    cur = xref_ea",
        "    while cur != idaapi.BADADDR and cur <= search_end:",
        "        if _is_registerconcommand_call(cur):",
        "            if cur not in seen_calls:",
        "                seen_calls.add(cur)",
        "                _analyze_call(cur)",
        "            break",
        "        next_cur = idc.next_head(cur, search_end + 1)",
        "        if next_cur in (idaapi.BADADDR, cur):",
        "            break",
        "        cur = next_cur",
        "return candidates",
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
            "    result = json.dumps({",
            "        'ok': True,",
            "        'candidates': _collect_candidates(params),",
            "    })",
            "except Exception:",
            "    result = json.dumps({",
            "        'ok': False,",
            "        'traceback': traceback.format_exc(),",
            "    })",
        ]
    )
    return "\n".join(lines) + "\n"


async def _collect_registerconcommand_candidates(
    session,
    platform,
    command_name,
    help_string,
    search_window_before_call,
    search_window_after_xref,
    debug=False,
):
    code = _build_registerconcommand_py_eval(
        platform=platform,
        command_name=command_name,
        help_string=help_string,
        search_window_before_call=search_window_before_call,
        search_window_after_xref=search_window_after_xref,
    )
    try:
        result = await session.call_tool(
            name="py_eval",
            arguments={"code": code},
        )
        payload = parse_mcp_result(result)
    except Exception:
        if debug:
            print("    Preprocess: py_eval collecting RegisterConCommand candidates failed")
        return []

    raw = None
    if isinstance(payload, dict):
        raw = payload.get("result", "")
    elif payload is not None:
        raw = str(payload)

    if not raw:
        return []

    try:
        parsed = json.loads(raw)
    except (TypeError, json.JSONDecodeError):
        if debug:
            print("    Preprocess: invalid RegisterConCommand candidate JSON")
        return []

    if not isinstance(parsed, dict):
        return []

    if parsed.get("ok") is False:
        if debug:
            print("    Preprocess: RegisterConCommand py_eval traceback follows")
            traceback_text = parsed.get("traceback")
            if isinstance(traceback_text, str) and traceback_text.strip():
                print(traceback_text.rstrip())
            else:
                print("    Preprocess: missing traceback text in py_eval result")
        return []

    candidates = parsed.get("candidates", [])
    if not isinstance(candidates, list):
        return []

    required_keys = {"command_name", "help_string", "handler_va"}
    for item in candidates:
        if not isinstance(item, dict):
            return []
        if not required_keys.issubset(item):
            return []
        if not isinstance(item["command_name"], str):
            return []
        if not isinstance(item["help_string"], str):
            return []
        handler_va = item["handler_va"]
        if isinstance(handler_va, bool):
            return []
        if _normalize_handler_va(handler_va) is None:
            return []
    return candidates


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
    try:
        result = await session.call_tool(
            name="py_eval",
            arguments={"code": fi_code},
        )
        result_data = parse_mcp_result(result)
    except Exception:
        if debug:
            print(f"    Preprocess: py_eval querying func info failed for {handler_va}")
        return None

    raw = None
    if isinstance(result_data, dict):
        raw = result_data.get("result", "")
    elif result_data is not None:
        raw = str(result_data)

    if not raw:
        return None

    try:
        data = json.loads(raw)
    except (TypeError, json.JSONDecodeError):
        if debug:
            print(f"    Preprocess: invalid func info JSON for {handler_va}")
        return None

    if not isinstance(data, dict):
        return None
    if "func_va" not in data or "func_size" not in data:
        return None
    return {
        "func_va": data["func_va"],
        "func_size": data["func_size"],
    }


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


def _normalize_handler_va(handler_va):
    if handler_va is None:
        return None
    try:
        if isinstance(handler_va, str):
            raw = handler_va.strip()
            if not raw:
                return None
            return hex(int(raw, 0))
        return hex(int(handler_va))
    except (TypeError, ValueError):
        return None


async def preprocess_registerconcommand_skill(
    session,
    expected_outputs,
    new_binary_dir,
    platform,
    image_base,
    target_name,
    generate_yaml_desired_fields,
    command_name=None,
    help_string=None,
    rename_to=None,
    expected_match_count=1,
    search_window_before_call=48,
    search_window_after_xref=24,
    debug=False,
):
    if command_name is None and help_string is None:
        if debug:
            print("    Preprocess: command_name/help_string cannot both be None")
        return False

    if expected_match_count != 1:
        if debug:
            print("    Preprocess: expected_match_count must be 1")
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

    candidates = await _collect_registerconcommand_candidates(
        session=session,
        platform=platform,
        command_name=command_name,
        help_string=help_string,
        search_window_before_call=search_window_before_call,
        search_window_after_xref=search_window_after_xref,
        debug=debug,
    )

    filtered = [
        item
        for item in candidates
        if (command_name is None or item.get("command_name") == command_name)
        and (help_string is None or item.get("help_string") == help_string)
    ]

    handler_values = sorted(
        {
            normalized
            for item in filtered
            for normalized in [_normalize_handler_va(item.get("handler_va"))]
            if normalized
        }
    )
    if len(handler_values) != 1:
        return False

    func_info = await _query_func_info(session, handler_values[0], debug=debug)
    if not isinstance(func_info, dict):
        return False

    extra_fields = {}
    if "func_rva" in requested_fields:
        try:
            extra_fields["func_rva"] = hex(int(func_info["func_va"], 16) - image_base)
        except (KeyError, ValueError, TypeError):
            return False

    if "func_sig" in requested_fields:
        sig_info = await preprocess_gen_func_sig_via_mcp(
            session=session,
            func_va=handler_values[0],
            image_base=image_base,
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
        payload = _build_func_payload(
            target_name, requested_fields, func_info, extra_fields
        )
    except KeyError:
        return False

    write_func_yaml(output_path, payload)
    return True
