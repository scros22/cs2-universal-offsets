#!/usr/bin/env python3
"""Preprocess script for find-CLoopModeGame_LoopShutdown-windows skill."""

from ida_analyze_util import preprocess_common_skill

# CLoopModeGame_Shutdown has been inlined into CLoopModeGame_LoopShutdown on Windows
TARGET_FUNCTION_NAMES = [
    "CLoopModeGame_LoopShutdown",
]

FUNC_XREFS = [
    # (func_name, xref_strings_list, xref_signatures_list, xref_funcs_list, exclude_funcs_list, exclude_strings_list)
    (
        "CLoopModeGame_LoopShutdown",
        ["--CLoopModeGame::SetWorldSession"],
        [],
        ["CLoopModeGame_SetGameSystemState", "IGameSystem_DestroyAllGameSystems"],
        ["CLoopModeGame_ReceivedServerInfo", "CLoopModeGame_SetWorldSession"],
        [],
    ),
]

FUNC_VTABLE_RELATIONS = [
    # (func_name, vtable_class)
    ("CLoopModeGame_LoopShutdown", "CLoopModeGame"),
]


GENERATE_YAML_DESIRED_FIELDS = [
    # (symbol_name, generate_yaml_fields)
    (
        "CLoopModeGame_LoopShutdown",
        [
            "func_name",
            "func_va",
            "func_rva",
            "func_size",
            "func_sig",
            "vtable_name",
            "vfunc_offset",
            "vfunc_index",
        ],
    ),
]

async def preprocess_skill(
    session, skill_name, expected_outputs, old_yaml_map,
    new_binary_dir, platform, image_base, debug=False,
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
        func_xrefs=FUNC_XREFS,
        func_vtable_relations=FUNC_VTABLE_RELATIONS,
        generate_yaml_desired_fields=GENERATE_YAML_DESIRED_FIELDS,
        debug=debug,
    )
