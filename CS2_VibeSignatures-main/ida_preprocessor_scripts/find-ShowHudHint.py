#!/usr/bin/env python3
"""Preprocess script for find-ShowHudHint skill."""

from ida_preprocessor_scripts._define_inputfunc import (
    preprocess_define_inputfunc_skill,
)

TARGET_NAME = "ShowHudHint"
INPUT_NAME = "ShowHudHint"
HANDLER_PTR_OFFSET = 0x10
ALLOWED_SEGMENT_NAMES = (".data",)
RENAME_TO = "ShowHudHint"

GENERATE_YAML_DESIRED_FIELDS = [
    # (symbol_name, generate_yaml_fields)
    (
        "ShowHudHint",
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
    """Locate the ShowHudHint input handler from its DEFINE_INPUTFUNC descriptor."""
    return await preprocess_define_inputfunc_skill(
        session=session,
        expected_outputs=expected_outputs,
        platform=platform,
        image_base=image_base,
        target_name=TARGET_NAME,
        input_name=INPUT_NAME,
        generate_yaml_desired_fields=GENERATE_YAML_DESIRED_FIELDS,
        handler_ptr_offset=HANDLER_PTR_OFFSET,
        allowed_segment_names=ALLOWED_SEGMENT_NAMES,
        rename_to=RENAME_TO,
        debug=debug,
    )
