#!/usr/bin/env python3
"""
IDA Skill Preprocessor for CS2_VibeSignatures.

This module is a lightweight MCP entrypoint. It opens an MCP session,
resolves the skill-specific preprocessor script under ida_preprocessor_scripts,
and delegates preprocessing to the script's exported method.
"""

import importlib.util
import inspect
import re
from pathlib import Path

from ida_analyze_util import parse_mcp_result

try:
    import httpx
    from mcp import ClientSession
    from mcp.client.streamable_http import streamable_http_client
except ImportError:
    pass


_SCRIPT_DIR = Path(__file__).resolve().parent / "ida_preprocessor_scripts"
_PREPROCESS_EXPORT_NAME = "preprocess_skill"
_SCRIPT_ENTRY_CACHE = {}


def _get_preprocess_entry(skill_name, debug=False):
    """Load and cache `preprocess_skill` from ida_preprocessor_scripts/{skill_name}.py."""
    if skill_name in _SCRIPT_ENTRY_CACHE:
        return _SCRIPT_ENTRY_CACHE[skill_name]

    script_path = _SCRIPT_DIR / f"{skill_name}.py"
    if not script_path.exists():
        if debug:
            print(f"    Preprocess: no script for skill {skill_name}: {script_path}")
        _SCRIPT_ENTRY_CACHE[skill_name] = None
        return None

    module_name = "ida_preprocessor_script_" + re.sub(r"[^0-9a-zA-Z_]", "_", skill_name)
    spec = importlib.util.spec_from_file_location(module_name, script_path)
    if spec is None or spec.loader is None:
        if debug:
            print(f"    Preprocess: failed to load module spec for {script_path}")
        _SCRIPT_ENTRY_CACHE[skill_name] = None
        return None

    module = importlib.util.module_from_spec(spec)
    try:
        spec.loader.exec_module(module)
    except Exception as e:
        if debug:
            print(f"    Preprocess: failed to import script {script_path}: {e}")
        _SCRIPT_ENTRY_CACHE[skill_name] = None
        return None

    preprocess_func = getattr(module, _PREPROCESS_EXPORT_NAME, None)
    if not callable(preprocess_func):
        if debug:
            print(
                f"    Preprocess: script {script_path} does not export callable "
                f"{_PREPROCESS_EXPORT_NAME}"
            )
        _SCRIPT_ENTRY_CACHE[skill_name] = None
        return None

    _SCRIPT_ENTRY_CACHE[skill_name] = preprocess_func
    return preprocess_func


async def preprocess_single_skill_via_mcp(
    host, port, skill_name, expected_outputs, old_yaml_map,
    new_binary_dir, platform,
    llm_model=None, llm_apikey=None, llm_baseurl=None, llm_temperature=None,
    debug=False,
):
    """
    Attempt to pre-process a single skill via IDA MCP and skill script dispatch.

    Dispatch flow:
      ida_analyze_bin.py -> preprocess_single_skill_via_mcp ->
      ida_preprocessor_scripts/{skill_name}.py::preprocess_skill

    Args:
        host: MCP server host
        port: MCP server port
        skill_name: name of the skill to preprocess
        expected_outputs: list of expected output YAML paths
        old_yaml_map: dict mapping new_yaml_path -> old_yaml_path
        new_binary_dir: directory for new version YAML outputs
        platform: "windows" or "linux"
        llm_model: optional OpenAI-compatible model name for scripts that support LLM
        llm_apikey: optional OpenAI-compatible API key for scripts that support LLM
        llm_baseurl: optional OpenAI-compatible base URL for scripts that support LLM
        llm_temperature: optional OpenAI-compatible temperature for scripts that support LLM
        debug: enable debug output

    Returns:
        True if skill script preprocessing succeeded, False otherwise
    """
    preprocess_func = _get_preprocess_entry(skill_name, debug=debug)
    if preprocess_func is None:
        return False

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

                    # Get image_base once for the script.
                    ib_result = await session.call_tool(
                        name="py_eval",
                        arguments={"code": "hex(idaapi.get_imagebase())"}
                    )
                    ib_data = parse_mcp_result(ib_result)
                    if isinstance(ib_data, dict):
                        image_base = int(ib_data.get("result", "0x0"), 16)
                    else:
                        image_base = int(str(ib_data), 16) if ib_data else 0

                    try:
                        llm_config = {
                            "model": llm_model,
                            "api_key": llm_apikey,
                            "base_url": llm_baseurl,
                            "temperature": llm_temperature,
                        }
                        preprocess_kwargs = {
                            "session": session,
                            "skill_name": skill_name,
                            "expected_outputs": expected_outputs,
                            "old_yaml_map": old_yaml_map,
                            "new_binary_dir": new_binary_dir,
                            "platform": platform,
                            "image_base": image_base,
                            "debug": debug,
                        }
                        if "llm_config" in inspect.signature(preprocess_func).parameters:
                            preprocess_kwargs["llm_config"] = llm_config

                        result = preprocess_func(**preprocess_kwargs)
                        if inspect.isawaitable(result):
                            result = await result
                        return bool(result)
                    except Exception as e:
                        if debug:
                            print(f"    Preprocess: script execution failed for {skill_name}: {e}")
                        return False

    except Exception as e:
        if debug:
            print(f"  Preprocess MCP connection error for {skill_name}: {e}")

    return False

