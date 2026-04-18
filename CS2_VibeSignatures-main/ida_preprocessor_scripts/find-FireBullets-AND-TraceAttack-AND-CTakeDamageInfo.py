#!/usr/bin/env python3
"""Preprocess script for find-FireBullets-AND-TraceAttack-AND-CTakeDamageInfo skill."""

from ida_analyze_util import preprocess_common_skill

TARGET_FUNCTION_NAMES = [
    "FireBullets",
    "TraceAttack",
]

TARGET_STRUCT_MEMBER_NAMES = [
    "CTakeDamageInfo_HitGroupInfo",
]


GENERATE_YAML_DESIRED_FIELDS = [
    # (symbol_name, generate_yaml_fields)
    (
        "FireBullets",
        [
            "func_name",
            "func_va",
            "func_rva",
            "func_size",
            "func_sig",
        ],
    ),
    (
        "TraceAttack",
        [
            "func_name",
            "func_va",
            "func_rva",
            "func_size",
            "func_sig",
        ],
    ),
    (
        "CTakeDamageInfo_HitGroupInfo",
        [
            "struct_name",
            "member_name",
            "offset",
            "size",
            "offset_sig",
            "offset_sig_disp",
        ],
    ),
]

async def preprocess_skill(
    session, skill_name, expected_outputs, old_yaml_map,
    new_binary_dir, platform, image_base, debug=False,
):
    """Reuse previous gamever signatures to locate target output(s) and write YAML."""
    return await preprocess_common_skill(
        session=session,
        expected_outputs=expected_outputs,
        old_yaml_map=old_yaml_map,
        new_binary_dir=new_binary_dir,
        platform=platform,
        image_base=image_base,
        func_names=TARGET_FUNCTION_NAMES,
        struct_member_names=TARGET_STRUCT_MEMBER_NAMES,
        generate_yaml_desired_fields=GENERATE_YAML_DESIRED_FIELDS,
        debug=debug,
    )
