#!/usr/bin/env python3
"""Preprocess script for find-CLoopModeGame_OnEventMapCallbacks-client skill."""

from ida_preprocessor_scripts._register_event_listener_abstract import (
    preprocess_register_event_listener_abstract_skill,
)


SOURCE_YAML_STEM = "CLoopModeGame_RegisterEventMapInternal"
REGISTER_FUNC_TARGET_NAME = "RegisterEventListener_Abstract"
ANCHOR_EVENT_NAME = "CLoopModeGame::OnClientPollNetworking"
SEARCH_WINDOW_AFTER_ANCHOR = 64
SEARCH_WINDOW_BEFORE_CALL = 64

TARGET_SPECS = [
    {
        "target_name": "CLoopModeGame_OnClientPollNetworking",
        "event_name": "CLoopModeGame::OnClientPollNetworking",
        "rename_to": "CLoopModeGame_OnClientPollNetworking",
    },
    {
        "target_name": "CLoopModeGame_OnClientAdvanceTick",
        "event_name": "CLoopModeGame::OnClientAdvanceTick",
        "rename_to": "CLoopModeGame_OnClientAdvanceTick",
    },
    {
        "target_name": "CLoopModeGame_OnClientPostAdvanceTick",
        "event_name": "CLoopModeGame::OnClientPostAdvanceTick",
        "rename_to": "CLoopModeGame_OnClientPostAdvanceTick",
    },
    {
        "target_name": "CLoopModeGame_OnClientPreSimulate",
        "event_name": "CLoopModeGame::OnClientPreSimulate",
        "rename_to": "CLoopModeGame_OnClientPreSimulate",
    },
    {
        "target_name": "CLoopModeGame_OnClientPreOutput",
        "event_name": "CLoopModeGame::OnClientPreOutput",
        "rename_to": "CLoopModeGame_OnClientPreOutput",
    },
    {
        "target_name": "CLoopModeGame_OnClientPreOutputParallelWithServer",
        "event_name": "CLoopModeGame::OnClientPreOutputParallelWithServer",
        "rename_to": "CLoopModeGame_OnClientPreOutputParallelWithServer",
    },
    {
        "target_name": "CLoopModeGame_OnClientPostOutput",
        "event_name": "CLoopModeGame::OnClientPostOutput",
        "rename_to": "CLoopModeGame_OnClientPostOutput",
    },
    {
        "target_name": "CLoopModeGame_OnClientFrameSimulate",
        "event_name": "CLoopModeGame::OnClientFrameSimulate",
        "rename_to": "CLoopModeGame_OnClientFrameSimulate",
    },
    {
        "target_name": "CLoopModeGame_OnClientAdvanceNonRenderedFrame",
        "event_name": "CLoopModeGame::OnClientAdvanceNonRenderedFrame",
        "rename_to": "CLoopModeGame_OnClientAdvanceNonRenderedFrame",
    },
    {
        "target_name": "CLoopModeGame_OnClientPostSimulate",
        "event_name": "CLoopModeGame::OnClientPostSimulate",
        "rename_to": "CLoopModeGame_OnClientPostSimulate",
    },
    {
        "target_name": "CLoopModeGame_OnClientPauseSimulate",
        "event_name": "CLoopModeGame::OnClientPauseSimulate",
        "rename_to": "CLoopModeGame_OnClientPauseSimulate",
    },
    {
        "target_name": "CLoopModeGame_OnClientSimulate",
        "event_name": "CLoopModeGame::OnClientSimulate",
        "rename_to": "CLoopModeGame_OnClientSimulate",
    },
    {
        "target_name": "CLoopModeGame_OnPostDataUpdate",
        "event_name": "CLoopModeGame::OnPostDataUpdate",
        "rename_to": "CLoopModeGame_OnPostDataUpdate",
    },
    {
        "target_name": "CLoopModeGame_OnPreDataUpdate",
        "event_name": "CLoopModeGame::OnPreDataUpdate",
        "rename_to": "CLoopModeGame_OnPreDataUpdate",
    },
    {
        "target_name": "CLoopModeGame_OnFrameBoundary",
        "event_name": "CLoopModeGame::OnFrameBoundary",
        "rename_to": "CLoopModeGame_OnFrameBoundary",
    },
]

_COMMON_GENERATE_FIELDS = [
    "func_name",
    "func_sig",
    "func_va",
    "func_rva",
    "func_size",
]

GENERATE_YAML_DESIRED_FIELDS = [
    (REGISTER_FUNC_TARGET_NAME, _COMMON_GENERATE_FIELDS),
    *[(target_spec["target_name"], _COMMON_GENERATE_FIELDS) for target_spec in TARGET_SPECS],
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
    """Resolve RegisterEventListener_Abstract callbacks and write YAML outputs."""
    _ = skill_name, old_yaml_map
    return await preprocess_register_event_listener_abstract_skill(
        session=session,
        expected_outputs=expected_outputs,
        new_binary_dir=new_binary_dir,
        platform=platform,
        image_base=image_base,
        source_yaml_stem=SOURCE_YAML_STEM,
        register_func_target_name=REGISTER_FUNC_TARGET_NAME,
        anchor_event_name=ANCHOR_EVENT_NAME,
        target_specs=TARGET_SPECS,
        generate_yaml_desired_fields=GENERATE_YAML_DESIRED_FIELDS,
        search_window_after_anchor=SEARCH_WINDOW_AFTER_ANCHOR,
        search_window_before_call=SEARCH_WINDOW_BEFORE_CALL,
        debug=debug,
    )
