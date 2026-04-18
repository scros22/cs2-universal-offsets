#!/usr/bin/env python3
"""
IDA Binary Analysis Script for CS2_VibeSignatures

Analyzes CS2 binary files using IDA Pro MCP and Claude/Codex agents.
Sequentially processes modules and symbols defined in config.yaml.

Usage:
    python ida_analyze_bin.py -gamever=14134 [-platform=windows,linux] [-agent=codex]

    -gamever: Game version subdirectory name (required)
    -oldgamever: Old game version for signature reuse (default: gamever - 1)
    -configyaml: Path to config.yaml file (default: config.yaml)
    -bindir: Directory containing downloaded binaries (default: bin)
    -platform: Platforms to analyze, comma-separated (default: windows,linux)
    -agent: Agent to use for analysis: claude or codex (default: claude)
    -ida_args: Additional arguments for idalib-mcp (optional)
    -debug: Enable debug output

Requirements:
    uv sync
    uv (for running idalib-mcp)
    claude CLI or codex CLI

Output:
    bin/14134/engine/CServerSideClient_IsHearingClient.linux.yaml
    bin/14134/engine/CServerSideClient_IsHearingClient.windows.yaml
    ...and more
"""

import argparse
import inspect
import json
import os
import re

from dotenv import load_dotenv
load_dotenv()
import socket
import subprocess
import sys
import threading
import time
import uuid
from pathlib import Path

try:
    import yaml
    import asyncio
    import httpx
    from mcp import ClientSession
    from mcp.client.streamable_http import streamable_http_client
    from mcp.types import TextContent
except ImportError as e:
    print(f"Error: Missing required dependency: {e.name}")
    print("Please install required dependencies with: uv sync")
    sys.exit(1)

from ida_skill_preprocessor import preprocess_single_skill_via_mcp
from ida_vcall_finder import (
    aggregate_vcall_results_for_object,
    export_object_xref_details_via_mcp,
)
    
DEFAULT_CONFIG_FILE = "config.yaml"
DEFAULT_BIN_DIR = "bin"
DEFAULT_PLATFORM = "windows,linux"
DEFAULT_MODULES = "*"
DEFAULT_AGENT = "claude"
DEFAULT_LLM_MODEL = "gpt-4o"
DEFAULT_HOST = "127.0.0.1"
DEFAULT_PORT = 13337
MCP_STARTUP_TIMEOUT = 1200  # seconds to wait for MCP server
SKILL_TIMEOUT = 1200  # 10 minutes per skill
ERROR_MARKER_RE = re.compile(
    r"(?<![A-Za-z0-9])error(?![A-Za-z0-9])",
    re.IGNORECASE,
)
SURVEY_BINARY_PATH_PY_EVAL = (
    "import json\n"
    "path = ''\n"
    "try:\n"
    "    import ida_nalt\n"
    "    path = ida_nalt.get_input_file_path() or ''\n"
    "except Exception:\n"
    "    pass\n"
    "if not path:\n"
    "    try:\n"
    "        import idc\n"
    "        path = idc.get_input_file_path() or ''\n"
    "    except Exception:\n"
    "        pass\n"
    "result = json.dumps({'metadata': {'path': path}})\n"
)


def _output_contains_error_marker(*texts: str) -> bool:
    merged_output = "\n".join(text for text in texts if text)
    return bool(ERROR_MARKER_RE.search(merged_output))


def _parse_tool_json_content(result):
    if not result or not getattr(result, "content", None):
        return None

    item = result.content[0]
    raw = getattr(item, "text", None)
    if not isinstance(raw, str):
        raw = item.text if isinstance(item, TextContent) else str(item)
    try:
        return json.loads(raw)
    except (json.JSONDecodeError, TypeError):
        return None


async def _survey_binary_path_via_py_eval(session):
    try:
        result = await session.call_tool(
            name="py_eval",
            arguments={"code": SURVEY_BINARY_PATH_PY_EVAL},
        )
    except Exception:
        return None

    payload = _parse_tool_json_content(result)
    if not isinstance(payload, dict):
        return None

    result_text = payload.get("result", "")
    if not isinstance(result_text, str) or not result_text:
        return None

    try:
        parsed = json.loads(result_text)
    except (json.JSONDecodeError, TypeError):
        return None

    return parsed if isinstance(parsed, dict) else None


async def survey_binary_via_session(session, detail_level="minimal"):
    try:
        result = await session.call_tool(
            name="survey_binary",
            arguments={"detail_level": detail_level},
        )
    except Exception:
        return await _survey_binary_path_via_py_eval(session)

    parsed = _parse_tool_json_content(result)
    if isinstance(parsed, dict):
        return parsed

    return await _survey_binary_path_via_py_eval(session)

async def check_mcp_health(host=DEFAULT_HOST, port=DEFAULT_PORT):
    """
    Verify MCP server is alive and responsive via a lightweight py_eval call.

    Args:
        host: MCP server host
        port: MCP server port
    Returns:
        True if the server responded successfully, False otherwise
    """
    server_url = f"http://{host}:{port}/mcp"

    try:
        async with httpx.AsyncClient(
            follow_redirects=True,
            timeout=httpx.Timeout(10.0, read=15.0),
            trust_env=False,
        ) as http_client:
            async with streamable_http_client(server_url, http_client=http_client) as (read_stream, write_stream, _):
                async with ClientSession(read_stream, write_stream) as session:
                    await session.initialize()
                    await session.call_tool(name="py_eval", arguments={"code": "1"})
                    return True
    except Exception:
        return False


async def survey_binary_via_mcp(host=DEFAULT_HOST, port=DEFAULT_PORT, detail_level="minimal"):
    """
    Retrieve a compact overview of the binary currently loaded in IDA.

    Args:
        host: MCP server host
        port: MCP server port
        detail_level: 'standard' or 'minimal' (use 'minimal' for binaries with >10k functions)
    Returns:
        Parsed survey dict on success, None otherwise.

    Example result (detail_level='minimal'):
        {
            "metadata": {
                "path": "D:\\CS2_VibeSignatures\\bin\\14141c\\engine\\engine2.dll.i64",
                "module": "engine2.dll",
                "arch": "64",
                "base_address": "0x180000000",
                "image_size": "0x95c000",
                "md5": "11092707508ae325cfdbcfb5ff200423",
                "sha256": "2f48108e724bdb389d0102bc673d762405d39f222d84f4ac56ce86bbe4d61c28"
            },
            "statistics": {
                "total_functions": 15504,
                "named_functions": 317,
                "library_functions": 248,
                "unnamed_functions": 14939,
                "total_strings": 14531,
                "total_segments": 6
            },
            "segments": [
                {"name": ".text",  "start": "0x180001000", "end": "0x180481000", "size": "0x480000", "permissions": "rx"},
                {"name": ".idata", "start": "0x180481000", "end": "0x180482bf0", "size": "0x1bf0",   "permissions": "r"},
                {"name": ".rdata", "start": "0x180482bf0", "end": "0x180604000", "size": "0x181410", "permissions": "r"},
                {"name": ".data",  "start": "0x180604000", "end": "0x180916000", "size": "0x312000", "permissions": "rw"},
                {"name": ".pdata", "start": "0x180916000", "end": "0x180950000", "size": "0x3a000",  "permissions": "r"},
                {"name": "_RDATA", "start": "0x180950000", "end": "0x180951000", "size": "0x1000",   "permissions": "r"}
            ],
            "entrypoints": [
                {"addr": "0x1802523d0", "name": "BinaryProperties_GetValue", "ordinal": 1},
                {"addr": "0x180400100", "name": "CreateInterface",           "ordinal": 2},
                {"addr": "0x180219430", "name": "Source2Main",               "ordinal": 7},
                ...
            ],
            "_note": "Binary has 15504 functions; xref analysis was limited to the first 10000 for performance."
        }
    """
    server_url = f"http://{host}:{port}/mcp"

    try:
        async with httpx.AsyncClient(
            follow_redirects=True,
            timeout=httpx.Timeout(10.0, read=30.0),
            trust_env=False,
        ) as http_client:
            async with streamable_http_client(server_url, http_client=http_client) as (read_stream, write_stream, _):
                async with ClientSession(read_stream, write_stream) as session:
                    await session.initialize()
                    return await survey_binary_via_session(session, detail_level=detail_level)
    except Exception:
        return None


async def preprocess_single_vcall_object_via_mcp(
    host,
    port,
    output_root,
    gamever,
    module_name,
    platform,
    object_name,
    debug=False,
):
    """Export xref detail YAMLs for a single vcall_finder object via MCP."""
    server_url = f"http://{host}:{port}/mcp"

    async with httpx.AsyncClient(
        follow_redirects=True,
        timeout=httpx.Timeout(30.0, read=300.0),
        trust_env=False,
    ) as http_client:
        async with streamable_http_client(server_url, http_client=http_client) as (read_stream, write_stream, _):
            async with ClientSession(read_stream, write_stream) as session:
                await session.initialize()
                return await export_object_xref_details_via_mcp(
                    session,
                    output_root=output_root,
                    gamever=gamever,
                    module_name=module_name,
                    platform=platform,
                    object_name=object_name,
                    debug=debug,
                )


def ensure_mcp_available(process, binary_path, host, port, ida_args, debug):
    """
    Ensure idalib-mcp is running and responsive. Restart if necessary.

    Checks the process status first, then performs a real MCP health check.
    If the server is unresponsive, kills the old process and starts a new one.

    Args:
        process: Current subprocess.Popen object (may be None)
        binary_path: Path to binary file for restarting idalib-mcp
        host: MCP server host
        port: MCP server port
        ida_args: Additional arguments for idalib-mcp
        debug: Enable debug output

    Returns:
        Tuple of (new_process, ok) where new_process may be the same object
        if no restart was needed, and ok indicates whether MCP is available.
    """
    # Step 1: check if the process has already exited
    if process is not None and process.poll() is not None:
        if debug:
            print(f"  idalib-mcp process exited with code {process.returncode}")
        process = None

    # Step 2: if process appears alive, do a real MCP health check
    if process is not None:
        healthy = asyncio.run(check_mcp_health(host, port))
        if healthy:
            return process, True
        print("  MCP health check failed, restarting idalib-mcp...")
        quit_ida_gracefully(process, host, port, debug=debug)
        process = None

    # Step 3: restart idalib-mcp
    print("  Restarting idalib-mcp...")
    new_process = start_idalib_mcp(binary_path, host, port, ida_args, debug)
    if new_process is None:
        return None, False
    return new_process, True


async def quit_ida_via_mcp(host=DEFAULT_HOST, port=DEFAULT_PORT):
    """
    Gracefully quit IDA using MCP py_eval tool with idc.qexit(0).

    Args:
        host: MCP server host
        port: MCP server port
    Returns:
        True if successful, False otherwise
    """
    server_url = f"http://{host}:{port}/mcp"

    try:
        async with httpx.AsyncClient(
            follow_redirects=True,
            timeout=httpx.Timeout(30.0, read=300.0),
            trust_env=False,  # Bypass system proxy to avoid 502
        ) as http_client:
            async with streamable_http_client(server_url, http_client=http_client) as (read_stream, write_stream, _):
                async with ClientSession(read_stream, write_stream) as session:
                    await session.initialize()
                    await session.call_tool(name="py_eval", arguments={"code": "import idc; idc.qexit(0)"})
                    return True
    except Exception:
        return False


async def quit_ida_gracefully_async(process, host=DEFAULT_HOST, port=DEFAULT_PORT, debug=False):
    """
    Attempt to quit IDA gracefully via MCP, fall back to terminate if needed.

    Args:
        process: subprocess.Popen object
        host: MCP server host
        port: MCP server port
    """
    if process is None:
        return

    if process.poll() is not None:
        return  # Process already exited

    if debug:
        print("  Quitting IDA gracefully via MCP...")

    try:
        await asyncio.wait_for(quit_ida_via_mcp(host, port), timeout=5)
    except Exception:
        pass

    try:
        await asyncio.to_thread(process.wait, timeout=10)
        if debug:
            print("  IDA exited gracefully")
        return
    except subprocess.TimeoutExpired:
        if debug:
            print("  Warning: IDA did not exit after qexit, forcing kill...")

    if process.poll() is None:
        try:
            process.kill()
            await asyncio.to_thread(process.wait, timeout=5)
        except Exception:
            pass


def quit_ida_gracefully(process, host=DEFAULT_HOST, port=DEFAULT_PORT, debug=False):
    """
    Synchronous wrapper around quit_ida_gracefully_async.

    Args:
        process: subprocess.Popen object
        host: MCP server host
        port: MCP server port
    """
    if process is None:
        return

    if process.poll() is not None:
        return

    try:
        asyncio.get_running_loop()
    except RuntimeError:
        asyncio.run(quit_ida_gracefully_async(process, host, port, debug=debug))
        return

    raise RuntimeError(
        "quit_ida_gracefully() cannot run inside an active event loop; "
        "use await quit_ida_gracefully_async() instead"
    )



def resolve_oldgamever(gamever, bin_dir):
    """
    Resolve the best oldgamever by searching for the most recent existing version
    directory under bin_dir.

    Version ordering (descending):
        14141z > 14141y > ... > 14141b > 14141a > 14141 > 14140

    Args:
        gamever: Current game version string (e.g., "14142", "14141a")
        bin_dir: Base binary directory to check for existing version subdirectories

    Returns:
        Best matching oldgamever string, or None if no candidate directory exists
    """
    if not gamever:
        return None

    # Parse gamever into (base_number, optional_suffix)
    if gamever[-1].islower() and gamever[-1].isalpha():
        suffix = gamever[-1]
        base_str = gamever[:-1]
    else:
        suffix = None
        base_str = gamever

    try:
        base = int(base_str)
    except ValueError:
        return None

    # Generate candidates in descending version order
    candidates = []

    if suffix:
        # E.g., gamever="14141c" -> try 14141b, 14141a, 14141, 14140z..14140a, 14140
        for c in range(ord(suffix) - 1, ord('a') - 1, -1):
            candidates.append(f"{base}{chr(c)}")
        candidates.append(str(base))
        prev_base = base - 1
        for c in range(ord('z'), ord('a') - 1, -1):
            candidates.append(f"{prev_base}{chr(c)}")
        candidates.append(str(prev_base))
    else:
        # E.g., gamever="14142" -> try 14141z..14141a, 14141, 14140
        prev_base = base - 1
        for c in range(ord('z'), ord('a') - 1, -1):
            candidates.append(f"{prev_base}{chr(c)}")
        candidates.append(str(prev_base))
        candidates.append(str(prev_base - 1))

    # Return the first candidate whose directory exists
    for candidate in candidates:
        candidate_dir = os.path.join(bin_dir, candidate)
        if os.path.isdir(candidate_dir):
            return candidate

    return None


def parse_vcall_finder_filter(raw_value):
    """
    Parse vcall finder selector into normalized filter structure.

    Args:
        raw_value: Raw selector string from CLI, e.g. "*", "a,b", or None

    Returns:
        None if selector is not provided; otherwise:
        {"all": bool, "names": set[str]}

    Raises:
        ValueError: If selector is empty or has invalid format.
    """
    if raw_value is None:
        return None

    if not isinstance(raw_value, str):
        raise ValueError("selector must be a string")

    selector = raw_value.strip()
    if not selector:
        raise ValueError("selector cannot be empty")

    if selector == "*":
        return {"all": True, "names": set()}

    names = []
    for name in selector.split(","):
        normalized_name = name.strip()
        if not normalized_name:
            raise ValueError("selector contains empty object name")
        names.append(normalized_name)

    if "*" in names:
        raise ValueError("'*' cannot be combined with object names")

    return {"all": False, "names": set(names)}


def _parse_optional_llm_temperature(raw_value, parser):
    if raw_value is None:
        return None
    raw_text = str(raw_value).strip()
    if not raw_text:
        return None
    try:
        return float(raw_text)
    except ValueError:
        parser.error("Invalid LLM temperature: must be a number")


def parse_args():
    """Parse command line arguments."""
    parser = argparse.ArgumentParser(
        description="Analyze CS2 binary files using IDA Pro MCP and Claude/Codex agents"
    )
    parser.add_argument(
        "-configyaml",
        default=DEFAULT_CONFIG_FILE,
        help=f"Path to config.yaml file (default: {DEFAULT_CONFIG_FILE})"
    )
    parser.add_argument(
        "-bindir",
        default=DEFAULT_BIN_DIR,
        help=f"Directory containing downloaded binaries (default: {DEFAULT_BIN_DIR})"
    )
    parser.add_argument(
        "-gamever",
        default=os.environ.get("CS2VIBE_GAMEVER"),
        required="CS2VIBE_GAMEVER" not in os.environ,
        help="Game version subdirectory name (required, or set CS2VIBE_GAMEVER env var)"
    )
    parser.add_argument(
        "-platform",
        default=DEFAULT_PLATFORM,
        help=f"Platforms to analyze, comma-separated (default: {DEFAULT_PLATFORM})"
    )
    parser.add_argument(
        "-agent",
        default=os.environ.get("CS2VIBE_AGENT", DEFAULT_AGENT),
        help=f"Agent executable to use for analysis, e.g., claude, claude.cmd, codex, codex.cmd (default: {DEFAULT_AGENT}, or set CS2VIBE_AGENT env var)"
    )
    parser.add_argument(
        "-modules",
        default=DEFAULT_MODULES,
        help=f"Modules to analyze, comma-separated (default: {DEFAULT_MODULES} for all). E.g., server,engine"
    )
    parser.add_argument(
        "-vcall_finder",
        default=None,
        help="vcall_finder object selector: '*' for all, or comma-separated object names"
    )
    parser.add_argument(
        "-llm_model",
        default=os.environ.get("CS2VIBE_LLM_MODEL", DEFAULT_LLM_MODEL),
        help=f"OpenAI-compatible model for preprocessing and vcall_finder workflow (default: {DEFAULT_LLM_MODEL}, or set CS2VIBE_LLM_MODEL env var)"
    )
    parser.add_argument(
        "-llm_apikey",
        default=os.environ.get("CS2VIBE_LLM_APIKEY"),
        help="OpenAI-compatible API key used by preprocessing and vcall_finder aggregation (or set CS2VIBE_LLM_APIKEY env var)"
    )
    parser.add_argument(
        "-llm_baseurl",
        default=os.environ.get("CS2VIBE_LLM_BASEURL"),
        help="Optional OpenAI-compatible base URL used by preprocessing and vcall_finder aggregation (or set CS2VIBE_LLM_BASEURL env var)"
    )
    parser.add_argument(
        "-llm_temperature",
        default=os.environ.get("CS2VIBE_LLM_TEMPERATURE"),
        help="Optional OpenAI-compatible temperature used by preprocessing and vcall_finder aggregation (or set CS2VIBE_LLM_TEMPERATURE env var)"
    )
    parser.add_argument(
        "-ida_args",
        default="",
        help="Additional arguments for idalib-mcp (optional)"
    )
    parser.add_argument(
        "-debug",
        action="store_true",
        help="Enable debug output"
    )
    parser.add_argument(
        "-maxretry",
        type=int,
        default=3,
        help="Maximum number of retry attempts for skill execution (default: 3)"
    )
    parser.add_argument(
        "-oldgamever",
        default=None,
        help="Old game version for signature reuse (default: gamever - 1). Set to 'none' to disable."
    )

    args = parser.parse_args()

    # Parse platforms
    args.platforms = [p.strip() for p in args.platform.split(",") if p.strip()]
    valid_platforms = {"windows", "linux"}
    for p in args.platforms:
        if p not in valid_platforms:
            parser.error(f"Invalid platform: {p}. Must be one of: {', '.join(valid_platforms)}")

    # Parse modules filter
    if args.modules == "*":
        args.module_filter = None  # None means all modules
    else:
        args.module_filter = [m.strip() for m in args.modules.split(",") if m.strip()]

    # Parse vcall_finder selector
    try:
        args.vcall_finder_filter = parse_vcall_finder_filter(args.vcall_finder)
    except ValueError as e:
        parser.error(f"Invalid -vcall_finder: {e}")

    # Resolve oldgamever
    if args.oldgamever is None:
        args.oldgamever = resolve_oldgamever(args.gamever, args.bindir)
    elif args.oldgamever.lower() == "none":
        args.oldgamever = None

    args.llm_temperature = _parse_optional_llm_temperature(
        args.llm_temperature,
        parser,
    )

    return args


def _run_preprocess_single_skill_via_mcp(
    *,
    host,
    port,
    skill_name,
    expected_outputs,
    old_yaml_map,
    new_binary_dir,
    platform,
    debug,
    llm_model,
    llm_apikey,
    llm_baseurl,
    llm_temperature,
):
    preprocess_kwargs = {
        "host": host,
        "port": port,
        "skill_name": skill_name,
        "expected_outputs": expected_outputs,
        "old_yaml_map": old_yaml_map,
        "new_binary_dir": new_binary_dir,
        "platform": platform,
        "debug": debug,
        "llm_model": llm_model,
        "llm_apikey": llm_apikey,
        "llm_baseurl": llm_baseurl,
        "llm_temperature": llm_temperature,
    }

    try:
        return asyncio.run(preprocess_single_skill_via_mcp(**preprocess_kwargs))
    except TypeError as exc:
        signature = inspect.signature(preprocess_single_skill_via_mcp)
        if any(
            name in signature.parameters
            for name in ("llm_model", "llm_apikey", "llm_baseurl", "llm_temperature")
        ):
            raise
        if "unexpected keyword argument" not in str(exc):
            raise

        fallback_kwargs = dict(preprocess_kwargs)
        fallback_kwargs.pop("llm_model", None)
        fallback_kwargs.pop("llm_apikey", None)
        fallback_kwargs.pop("llm_baseurl", None)
        fallback_kwargs.pop("llm_temperature", None)
        return asyncio.run(preprocess_single_skill_via_mcp(**fallback_kwargs))


def parse_config(config_path):
    """
    Parse config.yaml and extract module information.

    Args:
        config_path: Path to config.yaml file

    Returns:
        List of module dictionaries containing name, paths, and skills
    """
    with open(config_path, "r", encoding="utf-8") as f:
        config = yaml.safe_load(f)

    modules = []
    for module in config.get("modules", []):
        name = module.get("name")
        if not name:
            print("  Warning: Skipping module without name")
            continue

        skills = []
        for skill in module.get("skills", []):
            skill_name = skill.get("name")
            if skill_name:
                skills.append({
                    "name": skill_name,
                    "expected_output": skill.get("expected_output", []),
                    "expected_input": skill.get("expected_input", []),
                    "prerequisite": skill.get("prerequisite", []) or [],
                    "max_retries": skill.get("max_retries"),  # None means use default
                    "platform": skill.get("platform"),  # None means all platforms
                })

        if "vcall_finder" not in module or module.get("vcall_finder") is None:
            raw_vcall_finder_objects = []
        else:
            raw_vcall_finder_objects = module.get("vcall_finder")
        if not isinstance(raw_vcall_finder_objects, list):
            raise ValueError(
                f"Invalid vcall_finder for module '{name}': expected list, got {type(raw_vcall_finder_objects).__name__}"
            )

        for object_name in raw_vcall_finder_objects:
            if not isinstance(object_name, str):
                raise ValueError(
                    f"Invalid vcall_finder entry for module '{name}': expected string, got {type(object_name).__name__}"
                )

        modules.append({
            "name": name,
            "path_windows": module.get("path_windows"),
            "path_linux": module.get("path_linux"),
            "vcall_finder_objects": raw_vcall_finder_objects,
            "skills": skills
        })

    return modules


def resolve_module_vcall_targets(module, selector):
    """
    Resolve module-level vcall_finder targets using declared module objects only.

    Args:
        module: Module dictionary from parse_config()
        selector: Parsed selector from parse_vcall_finder_filter()

    Returns:
        List of object names that exist in module["vcall_finder_objects"].
    """
    if "vcall_finder_objects" not in module or module.get("vcall_finder_objects") is None:
        declared_objects = []
    else:
        declared_objects = module.get("vcall_finder_objects")
    if not isinstance(declared_objects, list):
        raise ValueError(
            f"Invalid vcall_finder_objects for module '{module.get('name', '<unknown>')}': "
            f"expected list, got {type(declared_objects).__name__}"
        )

    for object_name in declared_objects:
        if not isinstance(object_name, str):
            raise ValueError(
                f"Invalid vcall_finder_objects entry for module '{module.get('name', '<unknown>')}': "
                f"expected string, got {type(object_name).__name__}"
            )

    if selector is None:
        return []

    if selector.get("all"):
        return [name for name in declared_objects if name]

    selected_names = selector.get("names", set())
    return [name for name in declared_objects if name and name in selected_names]


def topological_sort_skills(skills):
    """
    Perform topological sort on skills by building dependencies from
    expected_input and expected_output relations.

    Args:
        skills: List of skill dicts with 'name', 'expected_input', and
            'expected_output' keys. Legacy 'prerequisite' is accepted as fallback.

    Returns:
        List of skill names in topologically sorted order (dependencies first)
    """
    skill_names = {skill["name"] for skill in skills}

    def normalize_artifact_path(path):
        """Normalize artifact path for matching expected input/output."""
        return os.path.normcase(os.path.normpath(path))

    # output_path -> producer skill names
    producers_by_output = {}
    for skill in skills:
        producer_name = skill["name"]
        for output_path in skill.get("expected_output", []):
            if not output_path:
                continue
            normalized_output = normalize_artifact_path(output_path)
            output_name = normalize_artifact_path(os.path.basename(output_path))
            producers_by_output.setdefault(normalized_output, set()).add(producer_name)
            producers_by_output.setdefault(output_name, set()).add(producer_name)

    # Infer dependencies from expected_input files.
    # If a skill consumes an artifact produced by another skill, it depends on it.
    dependencies = {name: set() for name in skill_names}
    for skill in skills:
        consumer_name = skill["name"]
        for input_path in skill.get("expected_input", []):
            if not input_path:
                continue

            normalized_input = normalize_artifact_path(input_path)
            input_name = normalize_artifact_path(os.path.basename(input_path))

            inferred_prereqs = set(producers_by_output.get(normalized_input, set()))
            if not inferred_prereqs:
                inferred_prereqs.update(producers_by_output.get(input_name, set()))
            inferred_prereqs.discard(consumer_name)
            dependencies[consumer_name].update(inferred_prereqs)

        # Backward compatibility: retain explicit prerequisite links if configured.
        for prereq in skill.get("prerequisite", []) or []:
            if prereq in skill_names and prereq != consumer_name:
                dependencies[consumer_name].add(prereq)

    # Build in-degree count and adjacency list
    in_degree = {name: len(dependencies[name]) for name in skill_names}
    dependents = {name: [] for name in skill_names}  # prereq -> list of dependent skills
    for consumer_name, prereqs in dependencies.items():
        for prereq in prereqs:
            dependents[prereq].append(consumer_name)

    # Kahn's algorithm for topological sort
    queue = sorted(name for name in skill_names if in_degree[name] == 0)

    sorted_names = []
    while queue:
        current = queue.pop(0)
        sorted_names.append(current)

        for dependent in sorted(dependents[current]):
            in_degree[dependent] -= 1
            if in_degree[dependent] == 0:
                queue.append(dependent)
        queue.sort()

    # Check for cycles
    if len(sorted_names) != len(skill_names):
        remaining = skill_names - set(sorted_names)
        print(f"  Warning: Circular dependency detected among skills: {remaining}")
        # Append remaining skills in original order as fallback
        for skill in skills:
            if skill["name"] not in sorted_names:
                sorted_names.append(skill["name"])

    return sorted_names


def should_start_binary_processing(skills_to_process, vcall_targets):
    """Start IDA when either skills or vcall_finder still has work to do."""
    return bool(skills_to_process or vcall_targets)


def resolve_artifact_path(binary_dir, artifact_path, platform):
    """Resolve one artifact path under the current gamever root."""
    if not artifact_path:
        raise ValueError("artifact path is empty")

    expanded = artifact_path.replace("{platform}", platform)
    module_dir = Path(binary_dir).resolve()
    gamever_dir = module_dir.parent.resolve()
    candidate = (module_dir / expanded).resolve()

    if os.path.commonpath([str(candidate), str(gamever_dir)]) != str(gamever_dir):
        raise ValueError(f"artifact path escapes gamever root: {artifact_path}")

    return str(candidate)


def expand_expected_paths(binary_dir, paths, platform):
    """Expand {platform} placeholders and resolve artifact paths under a binary directory."""
    return [
        resolve_artifact_path(binary_dir, path, platform)
        for path in paths
    ]


def all_expected_outputs_exist(expected_outputs):
    """Return True when every expected output already exists on disk."""
    return bool(expected_outputs) and all(os.path.exists(path) for path in expected_outputs)


def get_binary_path(bin_dir, gamever, module_name, module_path):
    """
    Build binary file path.

    Args:
        bin_dir: Base binary directory
        gamever: Game version subdirectory
        module_name: Module name (e.g., "engine")
        module_path: Module path from config (e.g., "game/bin/win64/engine2.dll")

    Returns:
        Full path to binary file: {bin_dir}/{gamever}/{module_name}/{filename}
    """
    filename = Path(module_path).name
    return os.path.join(bin_dir, gamever, module_name, filename)


def wait_for_port(host, port, timeout=60):
    """
    Wait for a port to become available.

    Args:
        host: Host address
        port: Port number
        timeout: Maximum time to wait in seconds

    Returns:
        True if port is available, False if timeout
    """
    start = time.time()
    while time.time() - start < timeout:
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(1)
            result = sock.connect_ex((host, port))
            sock.close()
            if result == 0:
                return True
        except socket.error:
            pass
        time.sleep(2)
    return False


def start_idalib_mcp(binary_path, host=DEFAULT_HOST, port=DEFAULT_PORT, ida_args="", debug=False):
    """
    Start idalib-mcp as a background process.

    Args:
        binary_path: Path to binary file to analyze
        host: MCP server host
        port: MCP server port
        ida_args: Additional arguments for idalib-mcp
        debug: Enable debug output

    Returns:
        subprocess.Popen object if successful, None if failed
    """
    cmd = ["uv", "run", "idalib-mcp", "--unsafe", "--host", host, "--port", str(port)]

    if ida_args:
        cmd.extend(ida_args.split())

    cmd.append(binary_path)

    print(f"  Starting idalib-mcp: {' '.join(cmd)}")

    try:
        if debug:
            process = subprocess.Popen(cmd)
        else:
            process = subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE
            )

        # Wait for MCP server to be ready
        print(f"  Waiting for MCP server on {host}:{port}...")
        if not wait_for_port(host, port, timeout=MCP_STARTUP_TIMEOUT):
            print(f"  Error: MCP server failed to start within {MCP_STARTUP_TIMEOUT} seconds")
            process.kill()
            return None

        print(f"  MCP server is ready")
        return process

    except Exception as e:
        print(f"  Error starting idalib-mcp: {e}")
        return None


def _drain_text_stream(stream, chunks, forward_stream=None):
    try:
        for chunk in iter(stream.readline, ""):
            chunks.append(chunk)
            if forward_stream is not None:
                forward_stream.write(chunk)
                forward_stream.flush()
    finally:
        try:
            stream.close()
        except Exception:
            pass

def _run_process_with_stream_capture(cmd, *, agent_input=None, debug=False, timeout=SKILL_TIMEOUT):
    process = subprocess.Popen(
        cmd,
        stdin=subprocess.PIPE if agent_input is not None else None,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )

    if agent_input is not None and process.stdin is not None:
        process.stdin.write(agent_input)
        process.stdin.flush()
        process.stdin.close()

    stdout_chunks = []
    stderr_chunks = []
    stdout_thread = threading.Thread(
        target=_drain_text_stream,
        args=(process.stdout, stdout_chunks, sys.stdout if debug else None),
    )
    stderr_thread = threading.Thread(
        target=_drain_text_stream,
        args=(process.stderr, stderr_chunks, sys.stderr if debug else None),
    )
    stdout_thread.start()
    stderr_thread.start()

    try:
        process.wait(timeout=timeout)
    except subprocess.TimeoutExpired:
        process.kill()
        try:
            process.wait(timeout=1)
        except Exception:
            pass
        stdout_thread.join(timeout=1)
        stderr_thread.join(timeout=1)
        raise

    stdout_thread.join()
    stderr_thread.join()
    return subprocess.CompletedProcess(
        args=cmd,
        returncode=process.returncode,
        stdout="".join(stdout_chunks),
        stderr="".join(stderr_chunks),
    )


def run_skill(skill_name, agent="claude", debug=False, expected_yaml_paths=None, max_retries=3):
    """
    Execute a skill using the specified agent with retry support.

    Args:
        skill_name: Name of the skill (e.g., "find-CServerSideClient_IsHearingClient")
        agent: Agent type ("claude" or "codex")
        debug: Enable debug output
        expected_yaml_paths: List of paths to expected yaml output files. If provided,
                            the skill is considered failed if any file is not generated.
        max_retries: Maximum number of retry attempts (default: 3)

    Returns:
        True if successful, False otherwise
    """
    claude_session_id = str(uuid.uuid4())
    codex_developer_instructions = None

    if "codex" in agent.lower():
        system_prompt_path = Path(".claude/agents/sig-finder.md")
        try:
            system_prompt_raw = system_prompt_path.read_text(encoding="utf-8")
        except FileNotFoundError:
            print(f"    Error: Codex system prompt file not found: {system_prompt_path}")
            return False
        except OSError as e:
            print(f"    Error: Failed to read Codex system prompt from {system_prompt_path}: {e}")
            return False

        codex_system_prompt = system_prompt_raw.strip()

        # Remove optional YAML frontmatter so only the prompt instructions are passed.
        if codex_system_prompt.startswith("---"):
            lines = codex_system_prompt.splitlines()
            frontmatter_end = None
            for idx, line in enumerate(lines[1:], start=1):
                if line.strip() == "---":
                    frontmatter_end = idx
                    break
            if frontmatter_end is not None:
                codex_system_prompt = "\n".join(lines[frontmatter_end + 1:]).strip()

        if not codex_system_prompt:
            print(f"    Error: Codex system prompt is empty in {system_prompt_path}")
            return False

        codex_developer_instructions = f"developer_instructions={json.dumps(codex_system_prompt)}"

    # Verify SKILL.md exists before launching agent
    skill_md_path = os.path.join(".claude", "skills", skill_name, "SKILL.md")
    if not os.path.exists(skill_md_path):
        print(f"    Error: Skill file not found: {skill_md_path}")
        return False

    for attempt in range(max_retries):
        is_retry = attempt > 0
        agent_input = None

        # Determine agent type based on executable name
        is_claude_agent = "claude" in agent.lower()
        is_codex_agent = "codex" in agent.lower()

        if is_claude_agent:
            cmd = [agent,
                   "-p", f"/{skill_name}",
                   "--agent", "sig-finder",
                   "--allowedTools", "mcp__ida-pro-mcp__*",
                   "--settings", '{"alwaysThinkingEnabled": false}',
                   ]
            # Add session management flags
            if is_retry:
                cmd.extend(["--resume", claude_session_id])
            else:
                cmd.extend(["--session-id", claude_session_id])
            retry_target_desc = f"session {claude_session_id}"
        elif is_codex_agent:
            skill_path = f".claude/skills/{skill_name}/SKILL.md"
            skill_prompt = f"Run SKILL: {skill_path}"
            agent_input = skill_prompt
            if is_retry:
                cmd = [agent, "-c", codex_developer_instructions, "-c", "model_reasoning_effort=high", "-c", "model_reasoning_summary=none", "-c", "model_verbosity=low", "exec", "resume", "--last", "-"]
            else:
                cmd = [agent, "-c", codex_developer_instructions, "-c", "model_reasoning_effort=high", "-c", "model_reasoning_summary=none", "-c", "model_verbosity=low", "exec", "-"]
            retry_target_desc = "the latest codex session (--last)"
        else:
            print(f"    Error: Unknown agent type '{agent}'. Agent name must contain 'claude' or 'codex'.")
            return False

        attempt_str = f"(attempt {attempt + 1}/{max_retries})" if max_retries > 1 else ""
        retry_str = "[RETRY] " if is_retry else ""

        display_cmd = cmd
        if "--system" in cmd:
            system_arg_index = cmd.index("--system") + 1
            if system_arg_index < len(cmd):
                display_cmd = cmd.copy()
                display_cmd[system_arg_index] = "<sig-finder-system-prompt>"

        for idx, arg in enumerate(cmd[:-1]):
            if arg == "-c" and cmd[idx + 1].startswith("developer_instructions="):
                if display_cmd is cmd:
                    display_cmd = cmd.copy()
                display_cmd[idx + 1] = "developer_instructions=<sig-finder-system-prompt>"

        prompt_transport = " <prompt via stdin>" if agent_input is not None else ""
        print(f"    {retry_str}Running {attempt_str}: {' '.join(display_cmd)}{prompt_transport}")

        try:
            result = _run_process_with_stream_capture(
                cmd,
                agent_input=agent_input,
                debug=debug,
                timeout=SKILL_TIMEOUT,
            )

            if result.returncode != 0:
                print(f"    Skill failed with return code: {result.returncode}")
                if not debug and result.stderr:
                    print(f"    stderr: {result.stderr[:500]}")
                if attempt < max_retries - 1:
                    print(f"    Retrying with {retry_target_desc}...")
                continue

            if _output_contains_error_marker(result.stdout, result.stderr):
                print("    Error: Skill output contains error marker")
                if attempt < max_retries - 1:
                    print(f"    Retrying with {retry_target_desc}...")
                continue

            # Verify all yaml files were generated if expected_yaml_paths is provided
            if expected_yaml_paths is not None:
                missing_files = [p for p in expected_yaml_paths if not os.path.exists(p)]
                if missing_files:
                    print(f"    Error: Expected yaml files not generated: {missing_files}")
                    if attempt < max_retries - 1:
                        print(f"    Retrying with {retry_target_desc}...")
                    continue

            return True

        except subprocess.TimeoutExpired:
            print(f"    Error: Skill execution timeout ({SKILL_TIMEOUT} seconds)")
            if attempt < max_retries - 1:
                print(f"    Retrying with {retry_target_desc}...")
            continue
        except FileNotFoundError:
            print(f"    Error: Agent '{agent}' not found. Please ensure it is installed and in PATH.")
            return False
        except Exception as e:
            print(f"    Error executing skill: {e}")
            if attempt < max_retries - 1:
                print(f"    Retrying with {retry_target_desc}...")
            continue

    print(f"    Failed after {max_retries} attempts")
    return False


def process_binary(
    binary_path,
    skills,
    agent,
    host,
    port,
    ida_args,
    platform,
    debug=False,
    max_retries=3,
    old_binary_dir=None,
    gamever=None,
    module_name=None,
    vcall_targets=None,
    vcall_output_dir="vcall_finder",
    llm_model=DEFAULT_LLM_MODEL,
    llm_apikey=None,
    llm_baseurl=None,
    llm_temperature=None,
):
    """
    Process a single binary file.

    Args:
        binary_path: Path to binary file
        skills: List of skill dicts with 'name', 'expected_output', 'expected_input',
            optional legacy 'prerequisite', and optional 'max_retries' keys
        agent: Agent type ("claude" or "codex")
        host: MCP server host
        port: MCP server port
        ida_args: Additional arguments for idalib-mcp
        platform: Platform name (e.g., "windows", "linux")
        debug: Enable debug output
        max_retries: Default maximum number of retry attempts for skill execution
        old_binary_dir: Directory containing old version YAML files for signature reuse

    Returns:
        Tuple of (success_count, fail_count, skip_count)
    """
    success_count = 0
    fail_count = 0
    skip_count = 0

    # Get the directory containing the binary for yaml output check
    binary_dir = os.path.dirname(binary_path)

    # Build skill_map for lookup
    skill_map = {skill["name"]: skill for skill in skills}

    # Topological sort skills based on inferred dependency tree
    sorted_skill_names = topological_sort_skills(skills)

    # Filter skills that need processing (skip if all expected outputs already exist)
    skills_to_process = []
    for skill_name in sorted_skill_names:
        skill = skill_map[skill_name]
        # Skip skills restricted to a different platform
        skill_platform = skill.get("platform")
        if skill_platform and skill_platform != platform:
            print(f"  Skipping skill: {skill_name} (platform '{skill_platform}' != '{platform}')")
            skip_count += 1
            continue
        try:
            expected_outputs = expand_expected_paths(binary_dir, skill["expected_output"], platform)
        except ValueError as e:
            fail_count += 1
            print(f"  Failed: {skill_name} ({e})")
            continue
        # Check if all output files already exist
        if all_expected_outputs_exist(expected_outputs):
            print(f"  Skipping skill: {skill_name} (all outputs exist)")
            skip_count += 1
        else:
            # Use skill-specific max_retries if provided, otherwise use default
            skill_max_retries = skill.get("max_retries") or max_retries
            skills_to_process.append((skill_name, expected_outputs, skill_max_retries))

    vcall_targets = list(vcall_targets or [])

    if not should_start_binary_processing(skills_to_process, vcall_targets):
        print("  All skills already have yaml files and no vcall_finder targets remain, skipping IDA startup")
        return success_count, fail_count, skip_count

    # Start idalib-mcp
    process = start_idalib_mcp(binary_path, host, port, ida_args, debug)
    if process is None:
        return success_count, fail_count + len(skills_to_process) + len(vcall_targets), skip_count

    try:
        # Process each skill: try preprocess first, then run_skill if needed
        for skill_index, (skill_name, expected_outputs, skill_max_retries) in enumerate(skills_to_process):
            if all_expected_outputs_exist(expected_outputs):
                print(f"  Skipping skill: {skill_name} (all outputs exist)")
                skip_count += 1
                continue

            # Ensure MCP connection is alive before running the skill
            process, mcp_ok = ensure_mcp_available(
                process, binary_path, host, port, ida_args, debug
            )
            if not mcp_ok:
                remaining = len(skills_to_process) - skill_index
                fail_count += remaining
                print(f"  Failed to restore MCP connection, aborting remaining {remaining} skill(s)")
                break

            # Check if all expected_input files are available before running the skill
            skill = skill_map[skill_name]
            try:
                expected_inputs = expand_expected_paths(binary_dir, skill.get("expected_input", []), platform)
            except ValueError as e:
                fail_count += 1
                print(f"  Failed: {skill_name} ({e})")
                continue
            missing_inputs = [p for p in expected_inputs if not os.path.exists(p)]
            if missing_inputs:
                fail_count += 1
                missing_names = [os.path.basename(p) for p in missing_inputs]
                print(f"  Failed: {skill_name} (missing expected_input: {', '.join(missing_names)})")
                continue

            # Try preprocessing first. Some preprocessors can run without old YAMLs.
            old_yaml_map = None
            if old_binary_dir:
                old_yaml_map = {}
                for new_path in expected_outputs:
                    filename = os.path.basename(new_path)
                    old_path = os.path.join(old_binary_dir, filename)
                    old_yaml_map[new_path] = old_path

            try:
                preprocess_ok = _run_preprocess_single_skill_via_mcp(
                    host=host,
                    port=port,
                    skill_name=skill_name,
                    expected_outputs=expected_outputs,
                    old_yaml_map=old_yaml_map,
                    new_binary_dir=binary_dir,
                    platform=platform,
                    debug=debug,
                    llm_model=llm_model,
                    llm_apikey=llm_apikey,
                    llm_baseurl=llm_baseurl,
                    llm_temperature=llm_temperature,
                )
            except Exception as e:
                if debug:
                    print(f"  Pre-processing error for {skill_name}: {e}")
                preprocess_ok = False

            if preprocess_ok:
                missing_outputs = [p for p in expected_outputs if not os.path.exists(p)]
                if missing_outputs:
                    fail_count += 1
                    missing_names = [os.path.basename(p) for p in missing_outputs]
                    print(f"  Pre-processed but missing expected_output: {skill_name} ({', '.join(missing_names)})")
                else:
                    success_count += 1
                    if old_binary_dir:
                        print(f"  Pre-processed: {skill_name} (signature reuse)")
                    else:
                        print(f"  Pre-processed: {skill_name}")
                continue

            if all_expected_outputs_exist(expected_outputs):
                print(f"  Skipping skill: {skill_name} (all outputs exist)")
                skip_count += 1
                continue

            print(f"  Processing skill: {skill_name}")

            if run_skill(skill_name, agent, debug, expected_yaml_paths=expected_outputs, max_retries=skill_max_retries):
                success_count += 1
                print(f"    Success")
            else:
                fail_count += 1
                print(f"    Failed")

        for object_index, object_name in enumerate(vcall_targets):
            process, mcp_ok = ensure_mcp_available(
                process, binary_path, host, port, ida_args, debug
            )
            if not mcp_ok:
                remaining = len(vcall_targets) - object_index
                fail_count += remaining
                print(f"  Failed to restore MCP connection, aborting remaining {remaining} vcall_finder target(s)")
                break

            print(f"  Processing vcall_finder: {object_name}")
            try:
                export_stats = asyncio.run(
                    preprocess_single_vcall_object_via_mcp(
                        host=host,
                        port=port,
                        output_root=vcall_output_dir,
                        gamever=gamever,
                        module_name=module_name,
                        platform=platform,
                        object_name=object_name,
                        debug=debug,
                    )
                )
            except Exception as exc:
                fail_count += 1
                print(f"    Failed to export vcall_finder for {object_name}: {exc}")
                continue

            object_status = export_stats["status"]
            if object_status == "success":
                success_count += 1
            elif object_status == "failed":
                fail_count += 1
            else:
                skip_count += 1

            exported_functions = export_stats["exported_functions"]
            failed_functions = export_stats["failed_functions"]
            skipped_functions = export_stats["skipped_functions"]
            if debug or failed_functions:
                print(
                    "    vcall_finder summary: "
                    f"status={object_status}, "
                    f"exported_functions={exported_functions}, "
                    f"failed_functions={failed_functions}, "
                    f"skipped_functions={skipped_functions}"
                )

    finally:
        # Gracefully quit IDA via MCP to avoid breaking IDB
        quit_ida_gracefully(process, host, port, debug=debug)

    return success_count, fail_count, skip_count


def main():
    """Main entry point."""
    args = parse_args()

    config_path = args.configyaml
    bin_dir = args.bindir
    gamever = args.gamever
    oldgamever = args.oldgamever
    platforms = args.platforms
    module_filter = args.module_filter
    agent = args.agent
    ida_args = args.ida_args
    debug = args.debug

    # Validate config file exists
    if not os.path.exists(config_path):
        print(f"Error: Config file not found: {config_path}")
        sys.exit(1)

    # Print configuration
    print(f"Config file: {config_path}")
    print(f"Binary directory: {bin_dir}")
    print(f"Game version: {gamever}")
    print(f"Old game version: {oldgamever or '(disabled)'}")
    print(f"Platforms: {', '.join(platforms)}")
    print(f"Modules filter: {args.modules}")
    print(f"Agent: {agent}")
    if ida_args:
        print(f"IDA args: {ida_args}")
    if debug:
        print("Debug mode: enabled")

    # Parse config
    print("\nParsing config...")
    modules = parse_config(config_path)
    print(f"Found {len(modules)} modules")

    if not modules:
        print("No modules found in config.")
        sys.exit(0)

    # Process each module
    total_success = 0
    total_fail = 0
    total_skip = 0
    all_vcall_objects = set()

    for module in modules:
        module_name = module["name"]
        skills = module["skills"]
        vcall_targets = resolve_module_vcall_targets(module, args.vcall_finder_filter)

        # Filter modules if specified
        if module_filter is not None and module_name not in module_filter:
            print(f"\nModule '{module_name}': Not in filter list, skipping")
            continue

        if not skills and not vcall_targets:
            print(f"\nModule '{module_name}': No skills or vcall_finder targets defined, skipping")
            continue

        all_vcall_objects.update(vcall_targets)

        print(f"\n{'='*60}")
        print(f"Module: {module_name}")
        print(f"Skills: {len(skills)}")
        if vcall_targets:
            print(f"VCall targets: {len(vcall_targets)}")
        print(f"{'='*60}")

        for platform in platforms:
            path_key = f"path_{platform}"
            module_path = module.get(path_key)

            if not module_path:
                print(f"\n  Platform {platform}: No path defined, skipping")
                total_skip += len(skills) + len(vcall_targets)
                continue

            # Build binary path
            binary_path = get_binary_path(bin_dir, gamever, module_name, module_path)

            print(f"\n  Platform: {platform}")
            print(f"  Binary: {binary_path}")

            # Check if binary exists
            if not os.path.exists(binary_path):
                print(f"  Error: Binary file not found: {binary_path}")
                print(f"  Hint: Run download_bin.py first to download binaries")
                total_skip += len(skills) + len(vcall_targets)
                continue

            # Compute old binary dir for signature reuse
            old_binary_dir = None
            if oldgamever:
                old_binary_path = get_binary_path(bin_dir, oldgamever, module_name, module_path)
                candidate_dir = os.path.dirname(old_binary_path)
                if os.path.isdir(candidate_dir):
                    old_binary_dir = candidate_dir
                elif debug:
                    print(f"  Old version directory not found: {candidate_dir}")

            # Process binary
            success, fail, skip = process_binary(
                binary_path, skills, agent,
                DEFAULT_HOST, DEFAULT_PORT, ida_args, platform, debug,
                max_retries=args.maxretry,
                old_binary_dir=old_binary_dir,
                gamever=gamever,
                module_name=module_name,
                vcall_targets=vcall_targets,
                llm_model=args.llm_model,
                llm_apikey=args.llm_apikey,
                llm_baseurl=args.llm_baseurl,
                llm_temperature=args.llm_temperature,
            )
            total_success += success
            total_fail += fail
            total_skip += skip

    if args.vcall_finder_filter and all_vcall_objects:
        print("\nRunning vcall_finder OpenAI aggregation")
        for object_name in sorted(all_vcall_objects):
            print(f"  Aggregating vcall_finder: {object_name}")
            try:
                stats = aggregate_vcall_results_for_object(
                    base_dir="vcall_finder",
                    gamever=gamever,
                    object_name=object_name,
                    model=args.llm_model,
                    api_key=args.llm_apikey,
                    base_url=args.llm_baseurl,
                    temperature=args.llm_temperature,
                    debug=debug,
                )
                aggregation_status = stats["status"]
                if aggregation_status == "success":
                    total_success += 1
                elif aggregation_status == "failed":
                    total_fail += 1
                else:
                    total_skip += 1

                if debug or stats["failed"]:
                    print(
                        "    vcall_finder aggregation summary: "
                        f"status={aggregation_status}, "
                        f"processed={stats['processed']}, failed={stats['failed']}"
                    )
            except Exception as exc:
                total_fail += 1
                print(f"  Failed to aggregate {object_name}: {exc}")

    # Summary
    print(f"\n{'='*60}")
    print(f"Summary")
    print(f"{'='*60}")
    print(f"  Successful: {total_success}")
    print(f"  Failed: {total_fail}")
    print(f"  Skipped: {total_skip}")

    if total_fail > 0:
        sys.exit(1)


if __name__ == "__main__":
    main()
