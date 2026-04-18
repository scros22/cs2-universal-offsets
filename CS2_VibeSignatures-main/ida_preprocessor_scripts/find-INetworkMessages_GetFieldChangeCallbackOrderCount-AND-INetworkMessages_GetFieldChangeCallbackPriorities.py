#!/usr/bin/env python3
"""Preprocess script for find-INetworkMessages_GetFieldChangeCallbackOrderCount-AND-INetworkMessages_GetFieldChangeCallbackPriorities skill."""

from ida_analyze_util import preprocess_common_skill

TARGET_FUNCTION_NAMES = [
    "INetworkMessages_GetFieldChangeCallbackOrderCount",
    "INetworkMessages_GetFieldChangeCallbackPriorities",
]

LLM_DECOMPILE = [
    # (symbol_name, path_to_prompt, path_to_reference)
    (
        "INetworkMessages_GetFieldChangeCallbackOrderCount",
        "prompt/call_llm_decompile.md",
        "references/networksystem/CFlattenedSerializers_CreateFieldChangedEventQueue.{platform}.yaml",
    ),
    (
        "INetworkMessages_GetFieldChangeCallbackPriorities",
        "prompt/call_llm_decompile.md",
        "references/networksystem/CFlattenedSerializers_CreateFieldChangedEventQueue.{platform}.yaml",
    ),
]

FUNC_VTABLE_RELATIONS = [
    # (func_name, vtable_class)
    ("INetworkMessages_GetFieldChangeCallbackOrderCount", "INetworkMessages"),
    ("INetworkMessages_GetFieldChangeCallbackPriorities", "INetworkMessages"),
]

GENERATE_YAML_DESIRED_FIELDS = [
    # (symbol_name, generate_yaml_fields)
    (
        "INetworkMessages_GetFieldChangeCallbackOrderCount",
        [
            "func_name",
            "vfunc_sig",
            "vfunc_offset",
            "vfunc_index",
            "vtable_name",
        ],
    ),
    (
        "INetworkMessages_GetFieldChangeCallbackPriorities",
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
