#!/usr/bin/env python3
"""Preprocess script for find-INetworkMessages_GetNetworkGroupCount-AND-INetworkMessages_GetNetworkGroupName-AND-INetworkMessages_GetNetworkGroupColor skill."""

from ida_analyze_util import preprocess_common_skill

TARGET_FUNCTION_NAMES = [
    "INetworkMessages_GetNetworkGroupCount",
    "INetworkMessages_GetNetworkGroupName",
    "INetworkMessages_GetNetworkGroupColor",
]

LLM_DECOMPILE = [
    # (symbol_name, path_to_prompt, path_to_reference)
    (
        "INetworkMessages_GetNetworkGroupCount",
        "prompt/call_llm_decompile.md",
        "references/networksystem/CNetworkSystem_SendNetworkStats.{platform}.yaml",
    ),
    (
        "INetworkMessages_GetNetworkGroupName",
        "prompt/call_llm_decompile.md",
        "references/networksystem/CNetworkSystem_SendNetworkStats.{platform}.yaml",
    ),
    (
        "INetworkMessages_GetNetworkGroupColor",
        "prompt/call_llm_decompile.md",
        "references/networksystem/CNetworkSystem_SendNetworkStats.{platform}.yaml",
    ),
]

FUNC_VTABLE_RELATIONS = [
    # (func_name, vtable_class)
    ("INetworkMessages_GetNetworkGroupCount", "INetworkMessages"),
    ("INetworkMessages_GetNetworkGroupName", "INetworkMessages"),
    ("INetworkMessages_GetNetworkGroupColor", "INetworkMessages"),
]


GENERATE_YAML_DESIRED_FIELDS = [
    # (symbol_name, generate_yaml_fields)
    (
        "INetworkMessages_GetNetworkGroupCount",
        [
            "func_name",
            "vfunc_sig",
            "vfunc_offset",
            "vfunc_index",
            "vtable_name",
        ],
    ),
    (
        "INetworkMessages_GetNetworkGroupName",
        [
            "func_name",
            "vfunc_sig",
            "vfunc_offset",
            "vfunc_index",
            "vtable_name",
        ],
    ),
    (
        "INetworkMessages_GetNetworkGroupColor",
        [
            "func_name",
            "vfunc_sig",
            "vfunc_offset",
            "vfunc_index",
            "vtable_name",
        ],
    ),
]

async def preprocess_skill(
    session, skill_name, expected_outputs, old_yaml_map,
    new_binary_dir, platform, image_base, llm_config=None, debug=False,
):
    """Locate target vfunc(s) via preprocessing and LLM decompile fallback."""
    return await preprocess_common_skill(
        session=session,
        expected_outputs=expected_outputs,
        old_yaml_map=old_yaml_map,
        new_binary_dir=new_binary_dir,
        platform=platform,
        image_base=image_base,
        func_names=TARGET_FUNCTION_NAMES,
        func_vtable_relations=FUNC_VTABLE_RELATIONS,
        llm_decompile_specs=LLM_DECOMPILE,
        llm_config=llm_config,
        generate_yaml_desired_fields=GENERATE_YAML_DESIRED_FIELDS,
        debug=debug,
    )
