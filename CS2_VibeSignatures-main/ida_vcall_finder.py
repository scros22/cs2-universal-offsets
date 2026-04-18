#!/usr/bin/env python3

from __future__ import annotations

import json
import re
import time
from collections.abc import Mapping, Sequence
from pathlib import Path
from typing import Any

import yaml
from ida_analyze_util import build_remote_text_export_py_eval, parse_mcp_result
from ida_llm_utils import (
    call_llm_text,
    create_openai_client,
    normalize_optional_temperature,
    require_nonempty_text,
)


VCALL_FINDER_DIRNAME = "vcall_finder"

PROMPT_TEMPLATE = """You are a reverse engineering expert. I have disassembly outputs and procedure code of the same function.

**Disassembly**

```c
{disasm_code}
```

**Procedure code**

```c
{procedure}
```

Please collect all virtual function calls for "{object_name}" and output those calls as YAML

Example:

```yaml
found_vcall:
  - insn_va: 0x12345678
    insn_disasm: call    [rax+68h]
    vfunc_offset: 0x68
  - insn_va: 0x12345680
    insn_disasm: call    rax
    vfunc_offset: 0x80
```

If there are no virtual function calls for "{object_name}" found, output an empty YAML.
"""


class LiteralDumper(yaml.SafeDumper):
    pass


def _literal_str_representer(dumper, value):
    style = "|" if "\n" in value else None
    return dumper.represent_scalar("tag:yaml.org,2002:str", value, style=style)


LiteralDumper.add_representer(str, _literal_str_representer)

_require_nonempty_text = require_nonempty_text


def _require_mapping(value: Any, name: str) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise TypeError(f"{name} must be a mapping, got {type(value).__name__}")
    return value


def _read_required_text(data: Mapping[str, Any], key: str, context: str) -> str:
    if key not in data:
        raise KeyError(f"{context} missing required key '{key}'")
    text = str(data[key]).strip()
    if not text:
        raise ValueError(f"{context} key '{key}' cannot be empty")
    return text


def _read_optional_text(data: Mapping[str, Any], key: str) -> str:
    value = data.get(key, "")
    if value is None:
        return ""
    return str(value)


def _parse_yaml_mapping(text: str) -> Mapping[str, Any] | None:
    try:
        parsed = yaml.load(text, Loader=yaml.BaseLoader)
    except yaml.YAMLError:
        return None
    if parsed is None:
        return {}
    if isinstance(parsed, Mapping):
        return parsed
    return None


def build_vcall_root(base_dir: str | Path = VCALL_FINDER_DIRNAME) -> Path:
    return Path(base_dir)


def build_vcall_detail_path(
    base_dir: str | Path,
    gamever: str,
    object_name: str,
    module_name: str,
    platform: str,
    func_name: str,
) -> Path:
    gamever_component = _normalize_safe_path_component(gamever, "gamever")
    object_component = _normalize_safe_path_component(object_name, "object_name")
    module_component = _normalize_safe_path_component(module_name, "module_name")
    platform_component = _normalize_safe_path_component(platform, "platform")
    func_component = _normalize_safe_path_component(func_name, "func_name")
    return (
        Path(base_dir)
        / gamever_component
        / object_component
        / module_component
        / platform_component
        / f"{func_component}.yaml"
    )

# `vcall_finder/14141b/g_pNetworkMessages.txt` for example
def build_vcall_summary_path(base_dir: str | Path, gamever: str, object_name: str) -> Path:
    gamever_component = _normalize_safe_path_component(gamever, "gamever")
    object_component = _normalize_safe_path_component(object_name, "object_name")
    return Path(base_dir) / gamever_component / f"{object_component}.txt"


def write_vcall_detail_yaml(path: str | Path, detail: Mapping[str, Any]) -> None:
    detail_data = _require_mapping(detail, "detail")
    payload = {
        "object_name": _read_required_text(detail_data, "object_name", "detail"),
        "module": _read_required_text(detail_data, "module", "detail"),
        "platform": _read_required_text(detail_data, "platform", "detail"),
        "func_name": _read_required_text(detail_data, "func_name", "detail"),
        "func_va": _read_required_text(detail_data, "func_va", "detail"),
        "disasm_code": _read_optional_text(detail_data, "disasm_code"),
        "procedure": _read_optional_text(detail_data, "procedure"),
    }
    _write_yaml_mapping(path, payload)


def write_vcall_detail_found_vcalls(
    path: str | Path,
    detail: Mapping[str, Any],
    found_vcall: Sequence[Any] | None,
) -> None:
    detail_data = _require_mapping(detail, "detail")
    payload = {
        "object_name": _read_required_text(detail_data, "object_name", "detail"),
        "module": _read_required_text(detail_data, "module", "detail"),
        "platform": _read_required_text(detail_data, "platform", "detail"),
        "func_name": _read_required_text(detail_data, "func_name", "detail"),
        "func_va": _read_required_text(detail_data, "func_va", "detail"),
        "disasm_code": _read_optional_text(detail_data, "disasm_code"),
        "procedure": _read_optional_text(detail_data, "procedure"),
        "found_vcall": normalize_found_vcalls(found_vcall),
    }
    _write_yaml_mapping(path, payload)


def load_yaml_file(path: str | Path) -> dict[str, Any]:
    path = Path(path)
    if not path.exists():
        return {}
    try:
        with path.open("r", encoding="utf-8") as file_obj:
            parsed = yaml.safe_load(file_obj)
    except yaml.YAMLError as exc:
        raise ValueError(f"Failed to parse YAML at '{path}': {exc}") from exc

    if parsed is None:
        return {}
    if not isinstance(parsed, dict):
        raise ValueError(f"YAML root must be mapping at '{path}', got {type(parsed).__name__}")
    return parsed


def render_vcall_prompt(detail: Mapping[str, Any]) -> str:
    detail_data = _require_mapping(detail, "detail")
    return PROMPT_TEMPLATE.format(
        object_name=_read_required_text(detail_data, "object_name", "detail"),
        disasm_code=_read_optional_text(detail_data, "disasm_code"),
        procedure=_read_optional_text(detail_data, "procedure"),
    )


def normalize_found_vcalls(entries: Sequence[Any] | None) -> list[dict[str, str]]:
    if entries is None:
        return []
    if isinstance(entries, (str, bytes, bytearray)) or not isinstance(entries, Sequence):
        return []

    normalized: list[dict[str, str]] = []
    for entry in entries:
        if not isinstance(entry, Mapping):
            continue
        insn_va = str(entry.get("insn_va", "")).strip()
        insn_disasm = str(entry.get("insn_disasm", "")).strip()
        vfunc_offset = str(entry.get("vfunc_offset", "")).strip()
        if not (insn_va and insn_disasm and vfunc_offset):
            continue
        normalized.append(
            {
                "insn_va": insn_va,
                "insn_disasm": insn_disasm,
                "vfunc_offset": vfunc_offset,
            }
        )
    return normalized


def parse_llm_vcall_response(response_text: str | None) -> dict[str, list[dict[str, str]]]:
    response_text = (response_text or "").strip()
    if not response_text:
        return {"found_vcall": []}

    candidates: list[str] = []
    for match in re.finditer(r"```(?:yaml|yml)[ \t]*\n?(.*?)```", response_text, re.IGNORECASE | re.DOTALL):
        candidates.append(match.group(1).strip())
    if not candidates:
        for match in re.finditer(r"```[ \t]*\n(.*?)```", response_text, re.DOTALL):
            candidates.append(match.group(1).strip())

    if candidates:
        for yaml_text in candidates:
            if not yaml_text:
                continue
            parsed = _parse_yaml_mapping(yaml_text)
            if parsed is None:
                continue
            return {"found_vcall": normalize_found_vcalls(parsed.get("found_vcall", []))}
        return {"found_vcall": []}

    parsed = _parse_yaml_mapping(response_text)
    if parsed is None or not parsed:
        return {"found_vcall": []}

    return {"found_vcall": normalize_found_vcalls(parsed.get("found_vcall", []))}


def call_openai_for_vcalls(
    client,
    detail,
    model,
    *,
    temperature=None,
    debug=False,
    request_label="",
):
    model = _require_nonempty_text(model, "model")
    if debug:
        _print_vcall_debug(
            f"LLM request start {request_label} model='{model}'".rstrip(),
            debug,
        )

    started_at = time.monotonic()
    request_kwargs = {
        "model": model,
        "client": client,
        "messages": [
            {"role": "system", "content": "You are a reverse engineering expert."},
            {"role": "user", "content": render_vcall_prompt(detail)},
        ],
    }
    normalized_temperature = normalize_optional_temperature(temperature)
    if normalized_temperature is not None:
        request_kwargs["temperature"] = normalized_temperature
    content = call_llm_text(**request_kwargs)
    found_vcall = parse_llm_vcall_response(content)["found_vcall"]
    if debug:
        elapsed_seconds = time.monotonic() - started_at
        _print_vcall_debug(
            "LLM request done "
            f"{request_label} elapsed={elapsed_seconds:.2f}s "
            f"response_chars={len(content)} found_vcall={len(found_vcall)}",
            debug,
        )

    return found_vcall


def _aggregate_vcall_detail_file(
    *,
    client_ref,
    api_key,
    base_url,
    temperature,
    detail_path,
    summary_path,
    request_label,
    model,
    debug,
):
    _print_vcall_debug(f"loading detail YAML {request_label}", debug)
    try:
        detail = load_yaml_file(detail_path)
    except Exception as exc:
        _print_vcall_debug(
            f"failed to load detail YAML {request_label}: {exc!r}",
            debug,
        )
        return False

    if not detail:
        _print_vcall_debug(f"empty detail YAML skipped {request_label}", debug)
        return False

    try:
        has_cached_found_vcall, found_vcall = _read_cached_found_vcalls(detail)
        source_name = "cache" if has_cached_found_vcall else "llm"
        if has_cached_found_vcall:
            _print_vcall_debug(
                f"found cached found_vcall {request_label} entries={len(found_vcall)}, skip llm",
                debug,
            )
        else:
            llm_client = _get_or_create_llm_client(
                client_ref,
                api_key=api_key,
                base_url=base_url,
            )
            found_vcall = call_openai_for_vcalls(
                llm_client,
                detail,
                model,
                temperature=temperature,
                debug=debug,
                request_label=request_label,
            )
            try:
                write_vcall_detail_found_vcalls(detail_path, detail, found_vcall)
                _print_vcall_debug(
                    f"detail YAML updated with found_vcall {request_label} entries={len(found_vcall)}",
                    debug,
                )
            except Exception as exc:
                _print_vcall_debug(
                    f"failed to update detail YAML {request_label}: {exc!r}",
                    debug,
                )

        appended = append_vcall_summary_entries(summary_path, detail, found_vcall)
        if appended:
            _print_vcall_debug(
                "appended "
                f"{appended} summary entr{'y' if appended == 1 else 'ies'} "
                f"{request_label} source='{source_name}'",
                debug,
            )
        else:
            _print_vcall_debug(
                f"no vcall entries to append {request_label} source='{source_name}'",
                debug,
            )
        return True
    except Exception as exc:
        _print_vcall_debug(
            f"OpenAI aggregation failed for {request_label}: {exc!r}",
            debug,
        )
        return False


def _aggregate_vcall_detail_paths(
    *,
    client_ref,
    api_key,
    base_url,
    temperature,
    detail_paths,
    summary_path,
    model,
    debug,
):
    processed = 0
    failed = 0
    total_paths = len(detail_paths)

    for detail_index, detail_path in enumerate(detail_paths, start=1):
        request_label = f"[{detail_index}/{total_paths}] '{detail_path}'"
        success = _aggregate_vcall_detail_file(
            client_ref=client_ref,
            api_key=api_key,
            base_url=base_url,
            temperature=temperature,
            detail_path=detail_path,
            summary_path=summary_path,
            request_label=request_label,
            model=model,
            debug=debug,
        )
        if success:
            processed += 1
        else:
            failed += 1

    return processed, failed


def aggregate_vcall_results_for_object(
    *,
    base_dir,
    gamever,
    object_name,
    model,
    api_key=None,
    base_url=None,
    temperature=None,
    client=None,
    debug=False,
):
    summary_path = build_vcall_summary_path(base_dir, gamever, object_name)
    detail_root = summary_path.with_suffix("")
    detail_paths = sorted(detail_root.glob("*/*/*.yaml"))
    if not detail_paths:
        return {"status": "skipped", "processed": 0, "failed": 0}

    initialize_vcall_summary_stream(summary_path)
    client_ref = {"client": client}
    _print_vcall_debug(f"summary stream reset '{summary_path}'", debug)
    _print_vcall_debug(
        "OpenAI aggregation "
        f"object='{object_name}', detail_files={len(detail_paths)}, "
        f"model='{model}', base_url='{base_url or '<default>'}'",
        debug,
    )

    processed, failed = _aggregate_vcall_detail_paths(
        client_ref=client_ref,
        api_key=api_key,
        base_url=base_url,
        temperature=temperature,
        detail_paths=detail_paths,
        summary_path=summary_path,
        model=model,
        debug=debug,
    )

    _print_vcall_debug(
        "OpenAI aggregation summary "
        f"object='{object_name}', processed={processed}, failed={failed}",
        debug,
    )

    return {
        "status": _resolve_vcall_aggregation_status(processed, failed),
        "processed": processed,
        "failed": failed,
    }


def initialize_vcall_summary_stream(path: str | Path) -> None:
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("", encoding="utf-8")


def build_vcall_summary_entries(
    detail: Mapping[str, Any],
    found_vcall: Sequence[Any] | None,
) -> list[dict[str, Any]]:
    detail_data = _require_mapping(detail, "detail")
    base_entry = {
        "object_name": _read_required_text(detail_data, "object_name", "detail"),
        "module": _read_required_text(detail_data, "module", "detail"),
        "platform": _read_required_text(detail_data, "platform", "detail"),
        "func_name": _read_required_text(detail_data, "func_name", "detail"),
        "func_va": _read_required_text(detail_data, "func_va", "detail"),
    }

    entries = []
    for vcall in normalize_found_vcalls(found_vcall):
        entry = dict(base_entry)
        entry.update(_require_mapping(vcall, "found_vcall[]"))
        entries.append(entry)

    return entries


def append_vcall_summary_entries(
    path: str | Path,
    detail: Mapping[str, Any],
    found_vcall: Sequence[Any] | None,
) -> int:
    entries = build_vcall_summary_entries(detail, found_vcall)
    if not entries:
        return 0

    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("a", encoding="utf-8") as file_obj:
        for entry in entries:
            yaml.dump(
                entry,
                file_obj,
                Dumper=LiteralDumper,
                sort_keys=False,
                allow_unicode=True,
                explicit_start=True,
            )

    return len(entries)


def _write_yaml_mapping(path: str | Path, payload: Mapping[str, Any]) -> None:
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8") as file_obj:
        yaml.dump(
            dict(payload),
            file_obj,
            Dumper=LiteralDumper,
            sort_keys=False,
            allow_unicode=True,
        )


def _print_vcall_debug(message: str, debug: bool) -> None:
    if not debug:
        return
    print(f"    vcall_finder: {message}", flush=True)


def _read_cached_found_vcalls(detail: Mapping[str, Any]) -> tuple[bool, list[dict[str, str]]]:
    detail_data = _require_mapping(detail, "detail")
    if "found_vcall" not in detail_data:
        return False, []
    return True, normalize_found_vcalls(detail_data.get("found_vcall"))


def _get_or_create_llm_client(
    client_ref: dict[str, Any],
    *,
    api_key: str | None,
    base_url: str | None,
):
    llm_client = client_ref.get("client")
    if llm_client is None:
        llm_client = create_openai_client(
            api_key=api_key,
            base_url=base_url,
            api_key_required_message="-llm_apikey is required when -vcall_finder is enabled",
        )
        client_ref["client"] = llm_client
    return llm_client


def _resolve_vcall_aggregation_status(processed: int, failed: int) -> str:
    if failed:
        return "failed"
    if processed:
        return "success"
    return "skipped"


def _normalize_safe_path_component(value: Any, name: str) -> str:
    text = _require_nonempty_text(value, name)

    normalized = text.replace("::", "_")
    normalized = normalized.replace("/", "_").replace("\\", "_")
    normalized = normalized.replace("..", "_")
    normalized = re.sub(r'[<>:"|?*\x00-\x1f]', "_", normalized)
    normalized = normalized.strip().strip(".")
    normalized = re.sub(r"_+", "_", normalized)

    if not normalized or normalized in {".", ".."}:
        normalized = "_"

    windows_reserved = {
        "CON",
        "PRN",
        "AUX",
        "NUL",
        "COM1",
        "COM2",
        "COM3",
        "COM4",
        "COM5",
        "COM6",
        "COM7",
        "COM8",
        "COM9",
        "LPT1",
        "LPT2",
        "LPT3",
        "LPT4",
        "LPT5",
        "LPT6",
        "LPT7",
        "LPT8",
        "LPT9",
    }
    base_name = normalized.split(".", 1)[0].upper()
    if base_name in windows_reserved:
        normalized = f"{normalized}_"

    return normalized


def _parse_int_value(value: Any, name: str) -> int:
    if isinstance(value, bool):
        raise TypeError(f"{name} must be an integer-like value, got bool")
    if isinstance(value, int):
        return value
    text = _require_nonempty_text(value, name)
    try:
        return int(text, 0)
    except ValueError as exc:
        raise ValueError(f"{name} must be a valid integer literal, got '{text}'") from exc


def _has_nonempty_error_marker(value: Any) -> bool:
    if value is None:
        return False
    if isinstance(value, bool):
        return value
    if isinstance(value, (str, bytes, bytearray)):
        return bool(str(value).strip())
    if isinstance(value, Mapping):
        return bool(value)
    if isinstance(value, Sequence):
        return not isinstance(value, (str, bytes, bytearray)) and bool(value)
    return True


def _is_error_payload_mapping(payload: Mapping[str, Any], expected_keys: Sequence[str] | None) -> bool:
    del expected_keys
    error_keys = ("error", "errors", "isError", "message", "stderr", "traceback", "exception")
    return any(
        key in payload and _has_nonempty_error_marker(payload.get(key))
        for key in error_keys
    )


def _format_object_scope(gamever: str, module_name: str, platform: str, object_name: str) -> str:
    return (
        f"gamever='{gamever}', module='{module_name}', "
        f"platform='{platform}', object='{object_name}'"
    )


def _format_function_scope(
    gamever: str,
    module_name: str,
    platform: str,
    object_name: str,
    func_name: str,
    func_va: str,
) -> str:
    return (
        f"{_format_object_scope(gamever, module_name, platform, object_name)}, "
        f"func='{func_name}', func_va='{func_va}'"
    )


def _parse_py_eval_json_payload(
    py_eval_result: Any,
    *,
    debug: bool,
    context: str,
    expected_keys: Sequence[str] | None = None,
) -> Any | None:
    parsed = parse_mcp_result(py_eval_result)

    payload: Any = parsed
    if isinstance(parsed, Mapping):
        stderr = str(parsed.get("stderr", "") or "").strip()
        if stderr and debug:
            print(f"    vcall_finder: {context} stderr:")
            print(stderr)
        if _is_error_payload_mapping(parsed, expected_keys):
            if debug:
                print(f"    vcall_finder: protocol error payload from {context}: {parsed}")
            return None
        if "result" in parsed:
            payload = parsed.get("result")

    if payload is None:
        return None
    if isinstance(payload, Mapping):
        if _is_error_payload_mapping(payload, expected_keys):
            if debug:
                print(f"    vcall_finder: error payload from {context}: {payload}")
            return None
        return payload
    if isinstance(payload, list):
        return payload

    text = str(payload).strip()
    if not text:
        return None
    try:
        decoded = json.loads(text)
    except json.JSONDecodeError:
        if debug:
            print(f"    vcall_finder: invalid JSON payload from {context}")
        return None
    if isinstance(decoded, Mapping):
        if _is_error_payload_mapping(decoded, expected_keys):
            if debug:
                print(f"    vcall_finder: decoded error payload from {context}: {decoded}")
            return None
    return decoded


def _is_valid_function_export_ack(
    export_ack: Any,
    *,
    detail_path: Path,
    debug: bool,
    function_scope: str,
) -> bool:
    if not isinstance(export_ack, Mapping):
        _print_vcall_debug(
            f"invalid function-export ack with {function_scope}: payload is not a mapping",
            debug,
        )
        return False

    if not bool(export_ack.get("ok")):
        _print_vcall_debug(
            f"invalid function-export ack with {function_scope}: ok is not truthy",
            debug,
        )
        return False

    try:
        output_path = _require_nonempty_text(export_ack.get("output_path"), "output_path")
    except (TypeError, ValueError) as exc:
        _print_vcall_debug(f"invalid function-export ack with {function_scope}: {exc}", debug)
        return False
    if output_path != str(detail_path):
        _print_vcall_debug(
            "invalid function-export ack with "
            f"{function_scope}: output_path mismatch ({output_path!r} != {str(detail_path)!r})",
            debug,
        )
        return False

    try:
        format_name = _require_nonempty_text(export_ack.get("format"), "format")
    except (TypeError, ValueError) as exc:
        _print_vcall_debug(f"invalid function-export ack with {function_scope}: {exc}", debug)
        return False
    if format_name != "yaml":
        _print_vcall_debug(
            f"invalid function-export ack with {function_scope}: format must be 'yaml', got {format_name!r}",
            debug,
        )
        return False

    try:
        bytes_written = _parse_int_value(export_ack.get("bytes_written"), "bytes_written")
    except (TypeError, ValueError) as exc:
        _print_vcall_debug(f"invalid function-export ack with {function_scope}: {exc}", debug)
        return False
    if bytes_written < 0:
        _print_vcall_debug(
            f"invalid function-export ack with {function_scope}: bytes_written must be non-negative, got {bytes_written}",
            debug,
        )
        return False

    return True


def build_object_xref_py_eval(object_name: str) -> str:
    object_name = _require_nonempty_text(object_name, "object_name")
    return (
        "import ida_funcs, ida_name, idaapi, idautils, json\n"
        f"object_name = {json.dumps(object_name)}\n"
        "object_ea = ida_name.get_name_ea(idaapi.BADADDR, object_name)\n"
        "if object_ea == idaapi.BADADDR:\n"
        "    result = json.dumps({'object_ea': None, 'functions': []})\n"
        "else:\n"
        "    seen = set()\n"
        "    functions = []\n"
        "    for xref in idautils.XrefsTo(object_ea, 0):\n"
        "        func = ida_funcs.get_func(xref.frm)\n"
        "        if func is None:\n"
        "            continue\n"
        "        func_start = int(func.start_ea)\n"
        "        if func_start in seen:\n"
        "            continue\n"
        "        seen.add(func_start)\n"
        "        func_name = ida_funcs.get_func_name(func_start) or f'sub_{func_start:X}'\n"
        "        functions.append({'func_name': func_name, 'func_va': hex(func_start)})\n"
        "    functions.sort(key=lambda item: int(item['func_va'], 16))\n"
        "    result = json.dumps({'object_ea': hex(object_ea), 'functions': functions})\n"
    )


def build_function_dump_export_py_eval(
    func_va: int | str,
    *,
    output_path: str | Path,
    object_name: str,
    module_name: str,
    platform: str,
) -> str:
    func_va_int = _parse_int_value(func_va, "func_va")
    output_path_str = str(Path(output_path).resolve())
    producer_code = (
        "import ida_funcs, ida_lines, ida_segment, idautils, idc\n"
        "try:\n"
        "    import ida_hexrays\n"
        "except Exception:\n"
        "    ida_hexrays = None\n"
        "try:\n"
        "    import yaml\n"
        "except Exception as exc:\n"
        "    raise RuntimeError('PyYAML is required for vcall_finder detail export') from exc\n"
        "class LiteralDumper(yaml.SafeDumper):\n"
        "    pass\n"
        "def _literal_str_representer(dumper, value):\n"
        "    style = '|' if '\\n' in value else None\n"
        "    return dumper.represent_scalar('tag:yaml.org,2002:str', value, style=style)\n"
        "LiteralDumper.add_representer(str, _literal_str_representer)\n"
        f"func_ea = {func_va_int}\n"
        f"object_name = {object_name!r}\n"
        f"module_name = {module_name!r}\n"
        f"platform_name = {platform!r}\n"
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
        "payload = {\n"
        "    'object_name': object_name,\n"
        "    'module': module_name,\n"
        "    'platform': platform_name,\n"
        "    'func_name': ida_funcs.get_func_name(func_start) or f'sub_{func_start:X}',\n"
        "    'func_va': hex(func_start),\n"
        "    'disasm_code': get_disasm(func_start),\n"
        "    'procedure': get_pseudocode(func_start),\n"
        "}\n"
        "payload_text = yaml.dump(\n"
        "    payload,\n"
        "    Dumper=LiteralDumper,\n"
        "    sort_keys=False,\n"
        "    allow_unicode=True,\n"
        ")\n"
    )
    return build_remote_text_export_py_eval(
        output_path=output_path_str,
        producer_code=producer_code,
        content_var="payload_text",
        format_name="yaml",
    )


async def export_object_xref_details_via_mcp(
    session: Any,
    *,
    output_root: str | Path,
    gamever: str,
    module_name: str,
    platform: str,
    object_name: str,
    debug: bool = False,
) -> dict[str, int]:
    gamever = _require_nonempty_text(gamever, "gamever")
    module_name = _require_nonempty_text(module_name, "module_name")
    platform = _require_nonempty_text(platform, "platform")
    object_name = _require_nonempty_text(object_name, "object_name")
    object_scope = _format_object_scope(gamever, module_name, platform, object_name)

    try:
        if debug:
            print(f"    vcall_finder: calling py_eval (object-xref) with {object_scope}")
        object_query_result = await session.call_tool(
            name="py_eval",
            arguments={"code": build_object_xref_py_eval(object_name)},
        )
    except Exception as exc:
        if debug:
            print(f"    vcall_finder: py_eval failed at object-xref step with {object_scope}: {exc!r}")
        return {
            "status": "failed",
            "exported_functions": 0,
            "failed_functions": 1,
            "skipped_functions": 0,
        }

    object_data = _parse_py_eval_json_payload(
        object_query_result,
        debug=debug,
        context=f"object xref query ({object_scope})",
        expected_keys=("object_ea", "functions"),
    )
    if not isinstance(object_data, Mapping):
        if debug:
            print(f"    vcall_finder: invalid object-xref payload with {object_scope}")
        return {
            "status": "failed",
            "exported_functions": 0,
            "failed_functions": 1,
            "skipped_functions": 0,
        }
    if not object_data.get("object_ea"):
        return {
            "status": "skipped",
            "exported_functions": 0,
            "failed_functions": 0,
            "skipped_functions": 1,
        }

    functions = object_data.get("functions", [])
    if isinstance(functions, (str, bytes, bytearray)) or not isinstance(functions, Sequence):
        return {
            "status": "failed",
            "exported_functions": 0,
            "failed_functions": 1,
            "skipped_functions": 0,
        }

    if not functions:
        return {
            "status": "skipped",
            "exported_functions": 0,
            "failed_functions": 0,
            "skipped_functions": 1,
        }

    exported_functions = 0
    failed_functions = 0
    skipped_functions = 0
    output_root_path = Path(output_root)

    for function in functions:
        if not isinstance(function, Mapping):
            if debug:
                print(f"    vcall_finder: invalid function entry for object '{object_name}': {function!r}")
            failed_functions += 1
            continue

        func_name = str(function.get("func_name", "")).strip()
        func_va_text = str(function.get("func_va", "")).strip()
        if not func_name or not func_va_text:
            if debug:
                print(
                    f"    vcall_finder: missing func_name/func_va in xref entry for '{object_name}': {function!r}"
                )
            failed_functions += 1
            continue

        detail_path = build_vcall_detail_path(
            output_root_path,
            gamever,
            object_name,
            module_name,
            platform,
            func_name,
        ).resolve()
        if detail_path.exists():
            skipped_functions += 1
            continue

        try:
            func_va_int = int(func_va_text, 0)
        except ValueError:
            if debug:
                print(
                    f"    vcall_finder: invalid func_va '{func_va_text}' in object '{object_name}'"
                )
            failed_functions += 1
            continue

        function_scope = _format_function_scope(
            gamever,
            module_name,
            platform,
            object_name,
            func_name,
            func_va_text,
        )
        try:
            if debug:
                print(f"    vcall_finder: calling py_eval (function-export) with {function_scope}")
            export_query_result = await session.call_tool(
                name="py_eval",
                arguments={
                    "code": build_function_dump_export_py_eval(
                        func_va_int,
                        output_path=detail_path,
                        object_name=object_name,
                        module_name=module_name,
                        platform=platform,
                    )
                },
            )
        except Exception as exc:
            if debug:
                print(
                    "    vcall_finder: py_eval failed at function-export step "
                    f"with {function_scope}: {exc!r}"
                )
            failed_functions += 1
            continue

        export_ack = _parse_py_eval_json_payload(
            export_query_result,
            debug=debug,
            context=f"function export ({function_scope})",
            expected_keys=("ok", "output_path", "bytes_written", "format"),
        )
        if not _is_valid_function_export_ack(
            export_ack,
            detail_path=detail_path,
            debug=debug,
            function_scope=function_scope,
        ):
            failed_functions += 1
            continue

        exported_functions += 1

    if failed_functions:
        status = "failed"
    elif exported_functions:
        status = "success"
    else:
        status = "skipped"

    return {
        "status": status,
        "exported_functions": exported_functions,
        "failed_functions": failed_functions,
        "skipped_functions": skipped_functions,
    }
