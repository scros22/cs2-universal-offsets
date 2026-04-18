#!/usr/bin/env python3
"""Preprocess script for find-UTIL_CreateEntityByName-AND-CBaseEntity_DispatchSpawn skill."""

from ida_analyze_util import preprocess_common_skill

TARGET_FUNCTION_NAMES = [
    "UTIL_CreateEntityByName",
    "CBaseEntity_DispatchSpawn",
]

LLM_DECOMPILE = [
    # (symbol_name, path_to_prompt, path_to_reference)
    (
        "UTIL_CreateEntityByName",
        "prompt/call_llm_decompile.md",
        "references/server/GiveNamedItem.{platform}.yaml",
    ),
    (
        "CBaseEntity_DispatchSpawn",
        "prompt/call_llm_decompile.md",
        "references/server/GiveNamedItem.{platform}.yaml",
    ),
]

GENERATE_YAML_DESIRED_FIELDS = [
    # (symbol_name, generate_yaml_fields)
    (
        "UTIL_CreateEntityByName",
        [
            "func_name",
            "func_sig",
            "func_va",
            "func_rva",
            "func_size",
        ],
    ),
    (
        "CBaseEntity_DispatchSpawn",
        [
            "func_name",
            "func_sig",
            "func_va",
            "func_rva",
            "func_size",
        ],
    ),
]

async def preprocess_skill(
    session, skill_name, expected_outputs, old_yaml_map,
    new_binary_dir, platform, image_base, llm_config=None, debug=False,
):
    """Reuse previous gamever func_sig to locate target function(s) and write YAML."""
    return await preprocess_common_skill(
        session=session,
        expected_outputs=expected_outputs,
        old_yaml_map=old_yaml_map,
        new_binary_dir=new_binary_dir,
        platform=platform,
        image_base=image_base,
        func_names=TARGET_FUNCTION_NAMES,
        llm_decompile_specs=LLM_DECOMPILE,
        llm_config=llm_config,
        generate_yaml_desired_fields=GENERATE_YAML_DESIRED_FIELDS,
        debug=debug,
    )
