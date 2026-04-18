#!/usr/bin/env python3
"""Preprocess script for find-CCSBotManager_AddBot_BotNavIgnore skill."""

from ida_analyze_util import preprocess_common_skill

TARGET_PATCH_NAMES = [
    "CCSBotManager_AddBot_BotNavIgnore",
]


GENERATE_YAML_DESIRED_FIELDS = [
    # (symbol_name, generate_yaml_fields)
    (
        "CCSBotManager_AddBot_BotNavIgnore",
        [
            "patch_name",
            "patch_sig",
            "patch_bytes",
        ],
    ),
]

async def preprocess_skill(
    session, skill_name, expected_outputs, old_yaml_map,
    new_binary_dir, platform, image_base, debug=False,
):
    """Reuse previous gamever patch_sig to validate and write patch YAML."""
    return await preprocess_common_skill(
        session=session,
        expected_outputs=expected_outputs,
        old_yaml_map=old_yaml_map,
        new_binary_dir=new_binary_dir,
        platform=platform,
        image_base=image_base,
        patch_names=TARGET_PATCH_NAMES,
        generate_yaml_desired_fields=GENERATE_YAML_DESIRED_FIELDS,
        debug=debug,
    )
