#!/usr/bin/env python3
"""Preprocess script for find-CCSPlayer_MovementServices_FullWalkMove-AND-CCSPlayer_MovementServices_CheckVelocity-AND-CCSPlayer_MovementServices_WaterMove skill."""

from ida_analyze_util import preprocess_common_skill

TARGET_FUNCTION_NAMES = [
    "CCSPlayer_MovementServices_FullWalkMove",
    "CCSPlayer_MovementServices_CheckVelocity",
    "CCSPlayer_MovementServices_WaterMove",
]


GENERATE_YAML_DESIRED_FIELDS = [
    # (symbol_name, generate_yaml_fields)
    (
        "CCSPlayer_MovementServices_FullWalkMove",
        [
            "func_name",
            "func_va",
            "func_rva",
            "func_size",
            "func_sig",
        ],
    ),
    (
        "CCSPlayer_MovementServices_CheckVelocity",
        [
            "func_name",
            "func_va",
            "func_rva",
            "func_size",
            "func_sig",
        ],
    ),
    (
        "CCSPlayer_MovementServices_WaterMove",
        [
            "func_name",
            "func_va",
            "func_rva",
            "func_size",
            "func_sig",
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
        generate_yaml_desired_fields=GENERATE_YAML_DESIRED_FIELDS,
        debug=debug,
    )
