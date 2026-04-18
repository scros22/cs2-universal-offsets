#!/usr/bin/env python3

from __future__ import annotations

import argparse
import asyncio
import importlib
import json
import os
from collections.abc import Mapping, Sequence
from contextlib import AsyncExitStack, asynccontextmanager
from pathlib import Path
from typing import Any

import yaml

try:
    import httpx
    from mcp import ClientSession
    from mcp.client.streamable_http import streamable_http_client
except ImportError:
    httpx = None
    ClientSession = None
    streamable_http_client = None

from ida_analyze_util import parse_mcp_result


class ReferenceGenerationError(RuntimeError):
    pass


class LiteralDumper(yaml.SafeDumper):
    pass


def _literal_str_representer(dumper: yaml.Dumper, value: str) -> yaml.Node:
    style = "|" if "\n" in value else None
    return dumper.represent_scalar("tag:yaml.org,2002:str", value, style=style)


LiteralDumper.add_representer(str, _literal_str_representer)


def _normalize_non_empty_text(value: Any) -> str | None:
    if not isinstance(value, str):
        return None
    text = value.strip()
    return text or None


def _normalize_address_text(value: Any, *, require_string: bool = False) -> str | None:
    if require_string:
        text = _normalize_non_empty_text(value)
        if text is None:
            return None
        try:
            int(text, 0)
        except (TypeError, ValueError):
            return None
        return text

    if isinstance(value, str):
        text = value.strip()
        if not text:
            return None
        try:
            int(text, 0)
        except (TypeError, ValueError):
            return None
        return text

    if isinstance(value, int):
        return hex(value)

    return None


def _validate_reference_yaml_payload(payload: Mapping[str, Any]) -> dict[str, str]:
    func_name = _normalize_non_empty_text(payload.get("func_name"))
    func_va = _normalize_address_text(payload.get("func_va"))
    disasm_code = _normalize_non_empty_text(payload.get("disasm_code"))
    procedure_raw = payload.get("procedure", "")

    if func_name is None or func_va is None or disasm_code is None:
        raise ReferenceGenerationError("invalid reference YAML payload")

    if procedure_raw is None:
        procedure = ""
    elif isinstance(procedure_raw, str):
        procedure = procedure_raw
    else:
        raise ReferenceGenerationError("invalid reference YAML payload")

    return {
        "func_name": func_name,
        "func_va": func_va,
        "disasm_code": disasm_code,
        "procedure": procedure,
    }


def parse_args(argv: Sequence[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate reference YAML for IDA preprocess scripts")
    parser.add_argument(
        "-gamever",
        default=os.environ.get("CS2VIBE_GAMEVER"),
        help="Game version (default: CS2VIBE_GAMEVER env var); when omitted, infer from binary path",
    )
    parser.add_argument(
        "-module",
        help="Module name; when omitted, infer it from the current IDA binary path",
    )
    parser.add_argument(
        "-platform",
        choices=["windows", "linux"],
        help="Target platform; when omitted, infer it from the current IDA binary path",
    )
    parser.add_argument("-func_name", required=True, help="Function name (required)")
    parser.add_argument("-mcp_host", default="127.0.0.1", help="MCP host")
    parser.add_argument("-mcp_port", type=int, default=13337, help="MCP port")
    parser.add_argument("-ida_args", default="", help="Additional arguments for idalib-mcp")
    parser.add_argument("-debug", action="store_true", help="Enable debug output")
    parser.add_argument("-binary", default=None, help="Binary path for auto-start MCP mode")
    parser.add_argument(
        "-auto_start_mcp",
        action="store_true",
        help="Start IDA MCP automatically; must be used with -binary",
    )

    args = parser.parse_args(argv)

    if args.auto_start_mcp and not args.binary:
        parser.error("-auto_start_mcp requires -binary")
    if args.binary and not args.auto_start_mcp:
        parser.error("-binary requires -auto_start_mcp")

    return args


def infer_target_from_binary_path(binary_path: str) -> dict[str, str]:
    normalized_path = _normalize_non_empty_text(binary_path)
    if normalized_path is None:
        raise ReferenceGenerationError("IDA survey did not provide a binary path")

    path_parts = [
        part.strip()
        for part in normalized_path.replace("\\", "/").split("/")
        if part.strip() and part != "."
    ]
    bin_index = next(
        (index for index in range(len(path_parts) - 1, -1, -1) if path_parts[index].lower() == "bin"),
        -1,
    )
    if bin_index < 0 or bin_index + 3 >= len(path_parts):
        raise ReferenceGenerationError(
            f"unable to infer -gamever/-module/-platform from IDA binary path: {normalized_path}"
        )

    gamever = path_parts[bin_index + 1]
    module = path_parts[bin_index + 2]
    platform = _infer_platform_from_binary_name(path_parts[bin_index + 3])
    if platform is None:
        raise ReferenceGenerationError(f"unable to infer platform from IDA binary path: {normalized_path}")

    return {
        "gamever": gamever,
        "module": module,
        "platform": platform,
    }


def _infer_platform_from_binary_name(binary_name: str) -> str | None:
    suffixes = [suffix.lower() for suffix in Path(binary_name).suffixes]
    if ".dll" in suffixes:
        return "windows"
    if ".so" in suffixes:
        return "linux"
    return None


def load_yaml_mapping(path: str | Path) -> dict[str, Any]:
    yaml_path = Path(path)
    if not yaml_path.exists():
        return {}

    try:
        data = yaml.safe_load(yaml_path.read_text(encoding="utf-8"))
    except yaml.YAMLError as exc:
        raise ReferenceGenerationError(f"Failed to parse YAML: {yaml_path}") from exc

    if data is None:
        return {}
    if not isinstance(data, Mapping):
        raise ReferenceGenerationError(f"YAML root must be a mapping: {yaml_path}")

    return dict(data)


def build_reference_output_path(
    repo_root: str | Path,
    module: str,
    func_name: str,
    platform: str,
) -> Path:
    return (
        Path(repo_root)
        / "ida_preprocessor_scripts"
        / "references"
        / module
        / f"{func_name}.{platform}.yaml"
    )


def build_existing_yaml_path(
    repo_root: str | Path,
    gamever: str,
    module: str,
    func_name: str,
    platform: str,
) -> Path:
    return Path(repo_root) / "bin" / gamever / module / f"{func_name}.{platform}.yaml"


def load_existing_func_va(
    repo_root: str | Path,
    gamever: str,
    module: str,
    func_name: str,
    platform: str,
) -> str | None:
    existing_yaml_path = build_existing_yaml_path(repo_root, gamever, module, func_name, platform)
    existing_yaml_map = load_yaml_mapping(existing_yaml_path)
    if not existing_yaml_map:
        return None

    func_va = existing_yaml_map.get("func_va")
    return _normalize_address_text(func_va)


def load_symbol_aliases(
    repo_root: str | Path,
    module: str,
    func_name: str,
) -> list[str]:
    config_path = Path(repo_root) / "config.yaml"
    config_map = load_yaml_mapping(config_path)
    modules = config_map.get("modules")
    if not isinstance(modules, list):
        raise ReferenceGenerationError("config.yaml missing 'modules' list")

    for module_entry in modules:
        if not isinstance(module_entry, Mapping):
            continue
        module_name = str(module_entry.get("name", "")).strip()
        if module_name != module:
            continue

        symbols = module_entry.get("symbols")
        if not isinstance(symbols, list):
            raise ReferenceGenerationError(f"module '{module}' missing 'symbols' list")

        for symbol_entry in symbols:
            if not isinstance(symbol_entry, Mapping):
                continue
            symbol_name = str(symbol_entry.get("name", "")).strip()
            if symbol_name != func_name:
                continue

            ordered_aliases: list[str] = []
            seen: set[str] = set()

            def _append_alias(raw: Any) -> None:
                text = str(raw).strip()
                if not text or text in seen:
                    return
                seen.add(text)
                ordered_aliases.append(text)

            _append_alias(symbol_name)
            raw_alias = symbol_entry.get("alias")
            if isinstance(raw_alias, list):
                for alias in raw_alias:
                    _append_alias(alias)
            elif raw_alias is not None:
                _append_alias(raw_alias)

            if not ordered_aliases:
                raise ReferenceGenerationError(
                    f"symbol '{func_name}' in module '{module}' has no usable alias values"
                )
            return ordered_aliases

        raise ReferenceGenerationError(
            f"symbol '{func_name}' not found in module '{module}' within config.yaml"
        )

    raise ReferenceGenerationError(f"module '{module}' not found in config.yaml")


def _parse_py_eval_json_result(
    eval_result: Any,
    *,
    debug: bool = False,
) -> Any:
    parsed = parse_mcp_result(eval_result)
    if not isinstance(parsed, dict):
        raise ReferenceGenerationError("invalid py_eval response from IDA")

    stderr_text = str(parsed.get("stderr", "")).strip()
    if stderr_text and debug:
        print(f"py_eval stderr: {stderr_text}")

    result_str = parsed.get("result", "")
    if not result_str:
        raise ReferenceGenerationError("missing py_eval result from IDA")

    try:
        return json.loads(result_str)
    except (json.JSONDecodeError, TypeError) as exc:
        raise ReferenceGenerationError("invalid py_eval JSON payload from IDA") from exc


async def find_function_addr_by_names(
    session: Any,
    candidate_names: Sequence[str],
    *,
    debug: bool = False,
) -> str:
    ordered_candidates: list[str] = []
    seen_candidates: set[str] = set()
    for raw_name in candidate_names:
        text = str(raw_name).strip()
        if not text or text in seen_candidates:
            continue
        seen_candidates.add(text)
        ordered_candidates.append(text)

    if not ordered_candidates:
        raise ReferenceGenerationError("unable to locate function address via IDA")

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
        match_payload = _parse_py_eval_json_result(eval_result, debug=debug)
    except ReferenceGenerationError:
        raise
    except Exception as exc:
        raise ReferenceGenerationError("unable to locate function address via IDA") from exc

    if not isinstance(match_payload, list):
        raise ReferenceGenerationError("unable to locate function address via IDA")

    resolved_matches: list[str] = []
    seen_func_vas: set[str] = set()
    for item in match_payload:
        if not isinstance(item, Mapping):
            continue
        func_va = _normalize_address_text(item.get("func_va"))
        if func_va is None or func_va in seen_func_vas:
            continue
        seen_func_vas.add(func_va)
        resolved_matches.append(func_va)

    if not resolved_matches:
        raise ReferenceGenerationError("unable to locate function address via IDA")
    if len(resolved_matches) > 1:
        raise ReferenceGenerationError(
            f"ambiguous function address matches returned via IDA: {resolved_matches!r}"
        )
    return resolved_matches[0]


async def resolve_func_va(
    session: Any,
    *,
    repo_root: str | Path,
    gamever: str,
    module: str,
    platform: str,
    func_name: str,
    debug: bool,
) -> str:
    existing_func_va = load_existing_func_va(
        repo_root=repo_root,
        gamever=gamever,
        module=module,
        func_name=func_name,
        platform=platform,
    )
    if existing_func_va:
        return existing_func_va

    candidate_names = load_symbol_aliases(
        repo_root=repo_root,
        module=module,
        func_name=func_name,
    )
    return await find_function_addr_by_names(
        session,
        candidate_names,
        debug=debug,
    )


async def export_reference_payload_via_mcp(
    session: Any,
    *,
    func_name: str,
    func_va: str,
    debug: bool = False,
) -> dict[str, str]:
    normalized_input_func_va = _normalize_address_text(func_va)
    if normalized_input_func_va is None:
        raise ReferenceGenerationError("unable to export reference payload via IDA")

    func_va_int = int(normalized_input_func_va, 0)
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
        exported_payload = _parse_py_eval_json_result(eval_result, debug=debug)
    except ReferenceGenerationError:
        raise
    except Exception as exc:
        raise ReferenceGenerationError("unable to export reference payload via IDA") from exc

    if not isinstance(exported_payload, Mapping):
        raise ReferenceGenerationError("unable to export reference payload via IDA")

    resolved_func_va = _normalize_address_text(exported_payload.get("func_va"))
    disasm_code = _normalize_non_empty_text(exported_payload.get("disasm_code"))
    procedure = exported_payload.get("procedure", "")
    if resolved_func_va is None or disasm_code is None:
        raise ReferenceGenerationError("unable to export reference payload via IDA")

    if procedure is None:
        normalized_procedure = ""
    elif isinstance(procedure, str):
        normalized_procedure = procedure
    else:
        raise ReferenceGenerationError("unable to export reference payload via IDA")

    return {
        "func_name": func_name,
        "func_va": resolved_func_va,
        "disasm_code": disasm_code,
        "procedure": normalized_procedure,
    }


def write_reference_yaml(path: str | Path, payload: Mapping[str, Any]) -> None:
    minimal_payload = _validate_reference_yaml_payload(payload)
    output_path = Path(path)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(
        yaml.dump(
            minimal_payload,
            Dumper=LiteralDumper,
            sort_keys=False,
            allow_unicode=True,
        ),
        encoding="utf-8",
    )


async def check_mcp_health(host: str, port: int) -> bool:
    ida_analyze_bin = _load_ida_analyze_bin()
    return await ida_analyze_bin.check_mcp_health(host, port)


async def survey_binary_via_mcp(host: str, port: int) -> Any:
    ida_analyze_bin = _load_ida_analyze_bin()
    return await ida_analyze_bin.survey_binary_via_mcp(host, port, detail_level="minimal")


async def survey_binary_via_session(session: Any) -> Any:
    ida_analyze_bin = _load_ida_analyze_bin()
    return await ida_analyze_bin.survey_binary_via_session(session, detail_level="minimal")


def _load_ida_analyze_bin() -> Any:
    try:
        return importlib.import_module("ida_analyze_bin")
    except (ImportError, SystemExit) as exc:
        raise ReferenceGenerationError("failed to import ida_analyze_bin helpers") from exc


def start_idalib_mcp(
    binary_path: str,
    host: str,
    port: int,
    ida_args: str,
    debug: bool,
) -> Any:
    ida_analyze_bin = _load_ida_analyze_bin()
    return ida_analyze_bin.start_idalib_mcp(binary_path, host, port, ida_args, debug)


def quit_ida_gracefully(
    process: Any,
    host: str,
    port: int,
    *,
    debug: bool,
) -> None:
    ida_analyze_bin = _load_ida_analyze_bin()
    ida_analyze_bin.quit_ida_gracefully(process, host, port, debug=debug)


async def quit_ida_gracefully_async(
    process: Any,
    host: str,
    port: int,
    *,
    debug: bool,
) -> None:
    ida_analyze_bin = _load_ida_analyze_bin()
    await ida_analyze_bin.quit_ida_gracefully_async(process, host, port, debug=debug)


@asynccontextmanager
async def _open_mcp_session(host: str, port: int):
    if httpx is None or ClientSession is None or streamable_http_client is None:
        raise ReferenceGenerationError("MCP client dependencies are unavailable")

    server_url = f"http://{host}:{port}/mcp"

    async with AsyncExitStack() as stack:
        try:
            http_client = await stack.enter_async_context(
                httpx.AsyncClient(
                    follow_redirects=True,
                    timeout=httpx.Timeout(30.0, read=300.0),
                    trust_env=False,
                )
            )
            read_stream, write_stream, _ = await stack.enter_async_context(
                streamable_http_client(server_url, http_client=http_client)
            )
            session = await stack.enter_async_context(ClientSession(read_stream, write_stream))
            await session.initialize()
        except Exception as exc:
            raise ReferenceGenerationError(f"unable to open MCP session at {host}:{port}") from exc

        yield session


@asynccontextmanager
async def attach_existing_mcp_session(host: str, port: int, debug: bool):
    del debug
    healthy = await check_mcp_health(host, port)
    if not healthy:
        raise ReferenceGenerationError(f"MCP server is not reachable at {host}:{port}")

    async with _open_mcp_session(host, port) as session:
        yield session


@asynccontextmanager
async def autostart_mcp_session(
    binary_path: str,
    host: str,
    port: int,
    ida_args: str,
    debug: bool,
):
    process = start_idalib_mcp(binary_path, host, port, ida_args, debug)
    if process is None:
        raise ReferenceGenerationError(f"failed to start idalib-mcp for {binary_path}")

    try:
        async with _open_mcp_session(host, port) as session:
            yield session
    finally:
        await quit_ida_gracefully_async(process, host, port, debug=debug)


async def resolve_generation_target(
    *,
    session: Any,
    gamever: str | None,
    module: str | None,
    platform: str | None,
    host: str,
    port: int,
) -> dict[str, str]:
    resolved_target = {
        "gamever": _normalize_non_empty_text(gamever),
        "module": _normalize_non_empty_text(module),
        "platform": _normalize_non_empty_text(platform),
    }
    missing_keys = [key for key, value in resolved_target.items() if value is None]
    if not missing_keys:
        return {
            "gamever": resolved_target["gamever"],
            "module": resolved_target["module"],
            "platform": resolved_target["platform"],
        }

    survey_result = await survey_binary_via_session(session)
    if survey_result is None:
        survey_result = await survey_binary_via_mcp(host, port)
    if not isinstance(survey_result, Mapping):
        missing_flags = ", ".join(f"-{key}" for key in missing_keys)
        raise ReferenceGenerationError(
            f"missing {missing_flags}, and failed to survey the current IDA binary via MCP"
        )

    metadata = survey_result.get("metadata")
    if not isinstance(metadata, Mapping):
        raise ReferenceGenerationError("IDA survey result is missing metadata")

    inferred_target = infer_target_from_binary_path(str(metadata.get("path", "")))
    for key in missing_keys:
        resolved_target[key] = inferred_target[key]

    return {
        "gamever": resolved_target["gamever"],
        "module": resolved_target["module"],
        "platform": resolved_target["platform"],
    }


async def run_reference_generation(
    args: argparse.Namespace,
    repo_root: str | Path | None = None,
) -> Path:
    resolved_repo_root = Path(repo_root) if repo_root is not None else Path(__file__).resolve().parent

    if args.auto_start_mcp:
        session_manager = autostart_mcp_session(
            binary_path=args.binary,
            host=args.mcp_host,
            port=args.mcp_port,
            ida_args=args.ida_args,
            debug=args.debug,
        )
    else:
        session_manager = attach_existing_mcp_session(
            host=args.mcp_host,
            port=args.mcp_port,
            debug=args.debug,
        )

    async with session_manager as session:
        resolved_target = await resolve_generation_target(
            session=session,
            gamever=args.gamever,
            module=args.module,
            platform=args.platform,
            host=args.mcp_host,
            port=args.mcp_port,
        )
        func_va = await resolve_func_va(
            session,
            repo_root=resolved_repo_root,
            gamever=resolved_target["gamever"],
            module=resolved_target["module"],
            platform=resolved_target["platform"],
            func_name=args.func_name,
            debug=args.debug,
        )
        payload = await export_reference_payload_via_mcp(
            session,
            func_name=args.func_name,
            func_va=func_va,
            debug=args.debug,
        )
        output_path = build_reference_output_path(
            resolved_repo_root,
            resolved_target["module"],
            args.func_name,
            resolved_target["platform"],
        )
        write_reference_yaml(output_path, payload)
        return output_path


def main(argv: Sequence[str] | None = None) -> int:
    args = parse_args(argv)
    try:
        output_path = asyncio.run(run_reference_generation(args))
    except ReferenceGenerationError as exc:
        print(f"ERROR: {exc}")
        return 1

    print(f"Generated reference YAML: {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
