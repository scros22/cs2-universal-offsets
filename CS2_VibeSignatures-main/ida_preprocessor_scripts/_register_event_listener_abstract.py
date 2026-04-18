#!/usr/bin/env python3
"""Shared preprocess helpers for RegisterEventListener_Abstract-like skills."""

import json
import os

try:
    import yaml
except ImportError:
    yaml = None

from ida_analyze_util import (
    parse_mcp_result,
    preprocess_gen_func_sig_via_mcp,
    write_func_yaml,
)


_REGISTER_EVENT_LISTENER_PY_EVAL_TEMPLATE = r"""
def _run():
    import json
    import traceback
    try:
        import ida_hexrays
    except Exception:
        return {'ok': False, 'error': 'ida_hexrays unavailable'}
    try:
        import idaapi, ida_bytes, idautils, idc
        params = json.loads(__PARAMS_JSON__)
        platform = params['platform']
        source_func_va = params['source_func_va']
        anchor_event_name = params['anchor_event_name']
        search_window_after_anchor = int(params['search_window_after_anchor'])
        search_window_before_call = int(params['search_window_before_call'])
        _DIRECT_VALUE_TYPES = (
            int(idaapi.o_imm),
            int(idaapi.o_mem),
            int(idaapi.o_near),
            int(idaapi.o_far),
            int(idaapi.o_displ),
        )
        def _scan_exact_strings(target_text):
            hits = []
            if not target_text:
                return hits
            for item in idautils.Strings():
                try:
                    if str(item) == target_text:
                        hits.append(int(item.ea))
                except Exception:
                    pass
            return hits
        def _read_string(ea):
            if ea in (None, 0, idaapi.BADADDR):
                return None
            try:
                raw = idc.get_strlit_contents(ea, -1, idc.STRTYPE_C)
            except Exception:
                raw = None
            if raw is None:
                return None
            return raw.decode('utf-8', errors='ignore') if isinstance(raw, bytes) else str(raw)
        def _prev_heads(start_ea, min_ea):
            cur = idc.prev_head(start_ea, min_ea)
            while cur != idaapi.BADADDR and cur >= min_ea:
                yield cur
                next_cur = idc.prev_head(cur, min_ea)
                if next_cur == cur:
                    break
                cur = next_cur
        def _operand_type(ea, operand_index):
            try:
                return int(idc.get_operand_type(ea, operand_index))
            except Exception:
                return -1
        def _expand_reg_names(reg_name):
            names = []
            raw = (reg_name or '').lower()
            if raw:
                names.append(raw)
                if raw.startswith('r') and len(raw) == 3 and raw[1:].isdigit():
                    names.append(raw + 'd')
                elif raw.startswith('r') and len(raw) == 3 and raw[1].isalpha():
                    names.append('e' + raw[1:])
                elif raw.startswith('e') and len(raw) == 3:
                    names.append('r' + raw[1:])
            return tuple(dict.fromkeys(names))
        def _recover_register_value(before_ea, reg_names, depth=0):
            if depth > 4:
                return None
            normalized_names = tuple(
                dict.fromkeys((name or '').lower() for name in reg_names if name)
            )
            if not normalized_names:
                return None
            min_ea = max(0, before_ea - search_window_before_call)
            for cur in _prev_heads(before_ea, min_ea):
                mnem = idc.print_insn_mnem(cur)
                op0 = (idc.print_operand(cur, 0) or '').lower()
                if mnem not in ('mov', 'lea') or op0 not in normalized_names:
                    continue
                op1_type = _operand_type(cur, 1)
                if op1_type in _DIRECT_VALUE_TYPES:
                    value = idc.get_operand_value(cur, 1)
                    if value not in (None, idaapi.BADADDR):
                        return int(value)
                if op1_type == int(idaapi.o_reg):
                    src_reg = idc.print_operand(cur, 1) or ''
                    if not src_reg:
                        continue
                    value = _recover_register_value(
                        cur,
                        _expand_reg_names(src_reg),
                        depth + 1,
                    )
                    if value not in (None, idaapi.BADADDR):
                        return int(value)
            return None
        def _resolve_operand_value(before_ea, ea, operand_index):
            op_type = _operand_type(ea, operand_index)
            if op_type in _DIRECT_VALUE_TYPES:
                value = idc.get_operand_value(ea, operand_index)
                if value not in (None, idaapi.BADADDR):
                    return int(value)
                return None
            if op_type == int(idaapi.o_reg):
                reg_name = idc.print_operand(ea, operand_index) or ''
                if reg_name:
                    value = _recover_register_value(
                        before_ea,
                        _expand_reg_names(reg_name),
                    )
                    if value not in (None, idaapi.BADADDR):
                        return int(value)
            return None
        def _resolve_call_callee(call_ea):
            if idc.print_insn_mnem(call_ea) not in ('call', 'jmp'):
                return None
            value = idc.get_operand_value(call_ea, 0)
            return None if value in (None, 0, idaapi.BADADDR) else int(value)
        def _recover_temp_base(call_ea):
            reg_name = 'rdx' if platform == 'windows' else 'rsi'
            value = _recover_register_value(call_ea, _expand_reg_names(reg_name))
            return None if value in (None, 0, idaapi.BADADDR) else int(value)
        def _recover_event_name(call_ea):
            min_ea = max(0, call_ea - search_window_before_call)
            for cur in _prev_heads(call_ea, min_ea):
                mnem = idc.print_insn_mnem(cur)
                op0 = (idc.print_operand(cur, 0) or '').lower()
                if mnem == 'push':
                    text = _read_string(_resolve_operand_value(cur, cur, 0))
                    if text:
                        return text
                if mnem == 'mov' and ('rsp' in op0 or 'esp' in op0):
                    text = _read_string(_resolve_operand_value(cur, cur, 1))
                    if text:
                        return text
            return None
        def _is_probable_function_entry(value):
            if value in (None, 0, idaapi.BADADDR):
                return False
            try:
                func = idaapi.get_func(int(value))
            except Exception:
                func = None
            if func is not None:
                try:
                    return int(func.start_ea) == int(value)
                except Exception:
                    pass
            try:
                return bool(idc.is_code(ida_bytes.get_full_flags(int(value))))
            except Exception:
                return False
        def _recover_callback_va(call_ea, temp_base):
            inferred_temp_base = None
            temp_callback_slot = (
                int(temp_base) + 8
                if temp_base not in (None, 0, idaapi.BADADDR)
                else None
            )
            min_ea = max(0, call_ea - search_window_before_call)
            for cur in _prev_heads(call_ea, min_ea):
                if idc.print_insn_mnem(cur) != 'mov':
                    continue
                if _operand_type(cur, 0) != int(idaapi.o_displ):
                    continue
                slot = idc.get_operand_value(cur, 0)
                if temp_callback_slot is not None and slot != temp_callback_slot:
                    continue
                value = _resolve_operand_value(cur, cur, 1)
                if value in (None, 0, idaapi.BADADDR):
                    continue
                if temp_callback_slot is None:
                    if slot in (None, 0, idaapi.BADADDR):
                        continue
                    if not _is_probable_function_entry(value):
                        continue
                    temp_callback_slot = int(slot)
                    inferred_temp_base = int(slot) - 8
                return int(value), temp_callback_slot, inferred_temp_base
            return None, temp_callback_slot, inferred_temp_base
        class _CallVisitor(ida_hexrays.ctree_visitor_t):
            def __init__(self):
                ida_hexrays.ctree_visitor_t.__init__(self, ida_hexrays.CV_FAST)
                self.call_pairs = []
            def visit_expr(self, expr):
                if expr.op != ida_hexrays.cot_call:
                    return 0
                cur = expr.x
                while cur is not None and getattr(cur, 'op', None) in (
                    ida_hexrays.cot_cast,
                    ida_hexrays.cot_ref,
                ):
                    next_expr = getattr(cur, 'x', None)
                    if next_expr is None or next_expr == cur:
                        break
                    cur = next_expr
                callee_ea = None
                if cur is not None and getattr(cur, 'op', None) == ida_hexrays.cot_obj:
                    value = int(cur.obj_ea)
                    if value not in (None, 0, idaapi.BADADDR):
                        callee_ea = int(value)
                if callee_ea not in (None, 0, idaapi.BADADDR):
                    self.call_pairs.append((int(expr.ea), int(callee_ea)))
                return 0
        try:
            source_func_va = int(str(source_func_va), 0)
        except Exception:
            return {'ok': False, 'error': 'invalid source_func_va'}
        source_func = idaapi.get_func(source_func_va)
        if source_func is None:
            return {'ok': False, 'error': 'source function not found'}
        items = []
        seen_calls = set()
        anchor_calls = []
        for string_ea in _scan_exact_strings(anchor_event_name):
            for xref in idautils.XrefsTo(string_ea, 0):
                xref_ea = int(xref.frm)
                if not source_func.start_ea <= xref_ea < source_func.end_ea:
                    continue
                if not idc.is_code(ida_bytes.get_full_flags(xref_ea)):
                    continue
                max_ea = min(xref_ea + search_window_after_anchor, source_func.end_ea - 1)
                if max_ea < xref_ea:
                    continue
                cur = xref_ea
                while cur != idaapi.BADADDR and cur <= max_ea:
                    callee = _resolve_call_callee(cur)
                    if callee is not None:
                        anchor_calls.append((int(cur), int(callee)))
                        break
                    next_cur = idc.next_head(cur, max_ea + 1)
                    if next_cur in (idaapi.BADADDR, cur):
                        break
                    cur = next_cur
        if not anchor_calls:
            return {'ok': False, 'error': 'anchor callee not found'}
        unique_callees = sorted({callee for _, callee in anchor_calls})
        if len(unique_callees) != 1:
            return {'ok': False, 'error': 'anchor callee is not unique'}
        register_func_va_int = unique_callees[0]
        cfunc = ida_hexrays.decompile(source_func.start_ea)
        if cfunc is None:
            return {'ok': False, 'error': 'failed to decompile source function'}
        pseudocode = '\n'.join(line.line for line in cfunc.get_pseudocode())
        if anchor_event_name not in pseudocode:
            return {'ok': False, 'error': 'anchor string missing in pseudocode'}
        anchor_call_eas = {ea for ea, _ in anchor_calls}
        visitor = _CallVisitor()
        visitor.apply_to(cfunc.body, None)
        pseudo_anchor_callees = sorted(
            {
                callee
                for ea, callee in visitor.call_pairs
                if ea in anchor_call_eas
            }
        )
        if not pseudo_anchor_callees:
            return {'ok': False, 'error': 'anchor callee missing in pseudocode'}
        if len(pseudo_anchor_callees) != 1:
            return {'ok': False, 'error': 'anchor callee is not unique in pseudocode'}
        if pseudo_anchor_callees[0] != register_func_va_int:
            return {'ok': False, 'error': 'anchor callee mismatch between asm and pseudocode'}
        for xref in sorted(idautils.XrefsTo(register_func_va_int, 0), key=lambda item: int(item.frm)):
            call_ea = int(xref.frm)
            if call_ea in seen_calls:
                continue
            if not source_func.start_ea <= call_ea < source_func.end_ea:
                continue
            if _resolve_call_callee(call_ea) != register_func_va_int:
                continue
            seen_calls.add(call_ea)
            event_name = _recover_event_name(call_ea)
            temp_base = _recover_temp_base(call_ea)
            callback_va, callback_slot, inferred_temp_base = _recover_callback_va(call_ea, temp_base)
            if temp_base in (None, 0, idaapi.BADADDR):
                temp_base = inferred_temp_base
            if (
                not event_name
                or temp_base in (None, 0, idaapi.BADADDR)
                or callback_va in (None, 0, idaapi.BADADDR)
                or callback_slot in (None, 0, idaapi.BADADDR)
            ):
                continue
            items.append({
                'event_name': event_name,
                'callback_va': hex(int(callback_va)),
                'call_ea': hex(call_ea),
                'temp_base': hex(int(temp_base)),
                'temp_callback_slot': hex(int(callback_slot)),
            })
        return {'ok': True, 'register_func_va': hex(register_func_va_int), 'items': items}
    except Exception:
        return {'ok': False, 'traceback': traceback.format_exc()}
try:
    import json
    import traceback
    result = json.dumps(_run())
except Exception:
    import json
    import traceback
    result = json.dumps({'ok': False, 'traceback': traceback.format_exc()})
"""

def _read_yaml(path):
    try:
        with open(path, "r", encoding="utf-8") as handle:
            return yaml.safe_load(handle)
    except Exception:
        return None

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
    matches = [path for path in expected_outputs if os.path.basename(path) == filename]
    if len(matches) != 1:
        if debug:
            print(f"    Preprocess: expected exactly one output for {filename}")
        return None
    return matches[0]

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

def _build_register_event_listener_py_eval(
    platform,
    source_func_va,
    anchor_event_name,
    search_window_after_anchor,
    search_window_before_call,
):
    params = json.dumps(
        {
            "platform": platform,
            "source_func_va": source_func_va,
            "anchor_event_name": anchor_event_name,
            "search_window_after_anchor": search_window_after_anchor,
            "search_window_before_call": search_window_before_call,
        }
    )
    return _REGISTER_EVENT_LISTENER_PY_EVAL_TEMPLATE.replace(
        "__PARAMS_JSON__", repr(params)
    ).lstrip()

async def _collect_register_event_listener_candidates(
    session,
    platform,
    source_func_va,
    anchor_event_name,
    search_window_after_anchor,
    search_window_before_call,
    debug=False,
):
    code = _build_register_event_listener_py_eval(
        platform=platform,
        source_func_va=source_func_va,
        anchor_event_name=anchor_event_name,
        search_window_after_anchor=search_window_after_anchor,
        search_window_before_call=search_window_before_call,
    )
    parsed = await _call_py_eval_json(
        session=session,
        code=code,
        debug=debug,
        error_label="py_eval collecting RegisterEventListener candidates",
    )
    if not isinstance(parsed, dict):
        return None
    if parsed.get("ok") is not True:
        if debug:
            error = parsed.get("error") or parsed.get("traceback")
            if isinstance(error, str) and error.strip():
                print(error.rstrip())
            else:
                print("    Preprocess: RegisterEventListener candidate collection failed")
        return None
    register_func_va = parsed.get("register_func_va")
    items = parsed.get("items")
    if not isinstance(register_func_va, str) or not register_func_va:
        return None
    if not isinstance(items, list):
        return None
    required_keys = {
        "event_name",
        "callback_va",
        "call_ea",
        "temp_base",
        "temp_callback_slot",
    }
    for item in items:
        if not isinstance(item, dict):
            return None
        if not required_keys.issubset(item):
            return None
        for key in required_keys:
            value = item.get(key)
            if not isinstance(value, str) or not value:
                return None
    return {"register_func_va": register_func_va, "items": items}

async def _query_func_info(session, func_va, debug=False):
    code = (
        "import idaapi, json\n"
        f"addr = {func_va!r}\n"
        "addr = int(addr, 0) if isinstance(addr, str) else int(addr)\n"
        "f = idaapi.get_func(addr)\n"
        "if f and f.start_ea == addr:\n"
        "    result = json.dumps({'func_va': hex(f.start_ea), "
        "'func_size': hex(f.end_ea - f.start_ea)})\n"
        "else:\n"
        "    result = json.dumps(None)\n"
    )
    parsed = await _call_py_eval_json(
        session=session,
        code=code,
        debug=debug,
        error_label=f"py_eval querying func info for {func_va}",
    )
    if not isinstance(parsed, dict):
        return None
    if "func_va" not in parsed or "func_size" not in parsed:
        return None
    return {"func_va": parsed["func_va"], "func_size": parsed["func_size"]}


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


async def preprocess_register_event_listener_abstract_skill(
    session,
    expected_outputs,
    new_binary_dir,
    platform,
    image_base,
    source_yaml_stem,
    register_func_target_name,
    anchor_event_name,
    target_specs,
    generate_yaml_desired_fields,
    register_func_rename_to=None,
    allow_extra_events=True,
    search_window_after_anchor=64,
    search_window_before_call=64,
    debug=False,
):
    if not isinstance(target_specs, list) or not target_specs:
        if debug:
            print("    Preprocess: target_specs must be a non-empty list")
        return False
    try:
        image_base_int = int(str(image_base), 0)
    except (TypeError, ValueError):
        return False

    source_yaml = _read_yaml(
        os.path.join(new_binary_dir, f"{source_yaml_stem}.{platform}.yaml")
    )
    source_func_entry_va = (
        source_yaml.get("func_va") if isinstance(source_yaml, dict) else None
    )
    try:
        source_func_va = hex(int(str(source_func_entry_va), 0))
    except (TypeError, ValueError):
        if debug:
            print(f"    Preprocess: invalid source func_va in {source_yaml_stem}.{platform}.yaml")
        return False

    register_fields = _normalize_requested_fields(
        generate_yaml_desired_fields, register_func_target_name, debug=debug
    )
    register_output = _resolve_output_path(
        expected_outputs, register_func_target_name, platform, debug=debug
    )
    if register_fields is None or register_output is None:
        return False

    normalized_specs = []
    declared_events = set()
    declared_targets = set()
    for spec in target_specs:
        if not isinstance(spec, dict):
            return False
        target_name = spec.get("target_name")
        event_name = spec.get("event_name")
        rename_to = spec.get("rename_to")
        if (
            not isinstance(target_name, str)
            or not target_name
            or target_name in declared_targets
        ):
            return False
        if not isinstance(event_name, str) or not event_name or event_name in declared_events:
            return False
        requested_fields = _normalize_requested_fields(
            generate_yaml_desired_fields, target_name, debug=debug
        )
        output_path = _resolve_output_path(
            expected_outputs, target_name, platform, debug=debug
        )
        if requested_fields is None or output_path is None:
            return False
        normalized_specs.append(
            {
                "target_name": target_name,
                "event_name": event_name,
                "rename_to": str(rename_to) if rename_to else None,
                "requested_fields": requested_fields,
                "output_path": output_path,
            }
        )
        declared_events.add(event_name)
        declared_targets.add(target_name)

    candidates = await _collect_register_event_listener_candidates(
        session=session,
        platform=platform,
        source_func_va=source_func_va,
        anchor_event_name=anchor_event_name,
        search_window_after_anchor=search_window_after_anchor,
        search_window_before_call=search_window_before_call,
        debug=debug,
    )
    if not isinstance(candidates, dict):
        return False

    register_func_va = candidates.get("register_func_va")
    items = candidates.get("items")
    if not isinstance(register_func_va, str) or not register_func_va:
        return False
    if not isinstance(items, list):
        return False

    items_by_event = {}
    for item in items:
        items_by_event.setdefault(item["event_name"], []).append(item)

    extra_events = sorted(set(items_by_event) - declared_events)
    if extra_events and not allow_extra_events:
        if debug:
            print(f"    Preprocess: unexpected extra events: {', '.join(extra_events)}")
        return False
    for spec in normalized_specs:
        matches = items_by_event.get(spec["event_name"], [])
        if len(matches) != 1:
            if debug:
                print(
                    f"    Preprocess: expected exactly one match for {spec['event_name']}, got {len(matches)}"
                )
            return False
        spec["callback_va"] = matches[0].get("callback_va")

    async def _build_payload_for(target_name, requested_fields, func_va):
        func_info = await _query_func_info(session, func_va, debug=debug)
        if not isinstance(func_info, dict):
            return None
        extra_fields = {}
        if "func_rva" in requested_fields:
            try:
                extra_fields["func_rva"] = hex(int(str(func_info["func_va"]), 0) - image_base_int)
            except (KeyError, TypeError, ValueError):
                return None
        if "func_sig" in requested_fields:
            sig_info = await preprocess_gen_func_sig_via_mcp(
                session=session, func_va=func_va, image_base=image_base_int, debug=debug
            )
            if not sig_info:
                return None
            try:
                extra_fields["func_sig"] = sig_info["func_sig"]
                extra_fields["func_rva"] = sig_info["func_rva"]
                extra_fields["func_size"] = sig_info["func_size"]
            except KeyError:
                return None
        try:
            return _build_func_payload(target_name, requested_fields, func_info, extra_fields)
        except KeyError:
            return None

    register_payload = await _build_payload_for(
        register_func_target_name, register_fields, register_func_va
    )
    if register_payload is None:
        return False

    pending_writes = [(register_output, register_payload)]
    pending_renames = [(register_func_va, register_func_rename_to)]

    for spec in normalized_specs:
        payload = await _build_payload_for(
            spec["target_name"], spec["requested_fields"], spec["callback_va"]
        )
        if payload is None:
            return False
        pending_writes.append((spec["output_path"], payload))
        pending_renames.append((spec["callback_va"], spec["rename_to"]))

    for output_path, payload in pending_writes:
        write_func_yaml(output_path, payload)

    for func_va, func_name in pending_renames:
        await _rename_func_best_effort(
            session=session,
            func_va=func_va,
            func_name=func_name,
            debug=debug,
        )

    if debug and extra_events and allow_extra_events:
        print(f"    Preprocess: ignored undeclared events: {', '.join(extra_events)}")
    return True
