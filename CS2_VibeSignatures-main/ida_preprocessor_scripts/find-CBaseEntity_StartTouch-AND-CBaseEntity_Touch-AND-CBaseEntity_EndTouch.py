#!/usr/bin/env python3
"""Preprocess script for find-CBaseEntity_StartTouch-AND-CBaseEntity_Touch-AND-CBaseEntity_EndTouch skill."""

from ida_analyze_util import preprocess_common_skill

TARGET_FUNCTION_NAMES = [
    "CBaseEntity_StartTouch",
    "CBaseEntity_Touch",
    "CBaseEntity_EndTouch",
]

LLM_DECOMPILE = [
    # (symbol_name, path_to_prompt, path_to_reference)
    # StartTouch and Touch are dispatched by CBaseEntity_PhysicsDispatchStartTouch
    (
        "CBaseEntity_StartTouch",
        "prompt/call_llm_decompile.md",
        "references/server/CBaseEntity_PhysicsDispatchStartTouch.{platform}.yaml",
    ),
    (
        "CBaseEntity_Touch",
        "prompt/call_llm_decompile.md",
        "references/server/CBaseEntity_PhysicsDispatchStartTouch.{platform}.yaml",
    ),
    # EndTouch is dispatched by CBaseEntity_PhysicsNotifyOtherOfEndTouch
    (
        "CBaseEntity_EndTouch",
        "prompt/call_llm_decompile.md",
        "references/server/CBaseEntity_PhysicsNotifyOtherOfEndTouch.{platform}.yaml",
    ),
]

FUNC_VTABLE_RELATIONS = [
    # (func_name, vtable_class)
    ("CBaseEntity_StartTouch", "CBaseEntity"),
    ("CBaseEntity_Touch", "CBaseEntity"),
    ("CBaseEntity_EndTouch", "CBaseEntity"),
]

GENERATE_YAML_DESIRED_FIELDS = [
    # (symbol_name, generate_yaml_fields)
    (
        "CBaseEntity_StartTouch",
        [
            "func_name",
            "vfunc_sig",
            "vfunc_offset",
            "vfunc_index",
            "vtable_name",
        ],
    ),
    (
        "CBaseEntity_Touch",
        [
            "func_name",
            "vfunc_sig",
            "vfunc_offset",
            "vfunc_index",
            "vtable_name",
        ],
    ),
    (
        "CBaseEntity_EndTouch",
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

    """Reuse previous gamever func_sig to locate target function(s) and write YAML."""
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
