#!/usr/bin/env python3
"""Preprocess script for find-NET_ReloadGameEvents_CommandHandler skill."""

from ida_preprocessor_scripts._registerconcommand import (
    preprocess_registerconcommand_skill,
)


TARGET_FUNCTION_NAMES = [
    "NET_ReloadGameEvents_CommandHandler",
]

COMMAND_NAME = "net_reloadgameevents"
HELP_STRING = "Reload the game events"
SEARCH_WINDOW_BEFORE_CALL = 96
SEARCH_WINDOW_AFTER_XREF = 96

GENERATE_YAML_DESIRED_FIELDS = [
    (
        "NET_ReloadGameEvents_CommandHandler",
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
    session,
    skill_name,
    expected_outputs,
    old_yaml_map,
    new_binary_dir,
    platform,
    image_base,
    debug=False,
):
    _ = skill_name, old_yaml_map
    return await preprocess_registerconcommand_skill(
        session=session,
        expected_outputs=expected_outputs,
        new_binary_dir=new_binary_dir,
        platform=platform,
        image_base=image_base,
        target_name=TARGET_FUNCTION_NAMES[0],
        generate_yaml_desired_fields=GENERATE_YAML_DESIRED_FIELDS,
        command_name=COMMAND_NAME,
        help_string=HELP_STRING,
        search_window_before_call=SEARCH_WINDOW_BEFORE_CALL,
        search_window_after_xref=SEARCH_WINDOW_AFTER_XREF,
        debug=debug,
    )
