#!/usr/bin/env python3
"""Preprocess script for find-IGameSystem_ClientPreEntityThink skill."""

from ida_preprocessor_scripts._igamesystem_dispatch_common import (
    preprocess_igamesystem_dispatch_skill,
)

SOURCE_YAML_STEM = "CLoopModeGame_OnClientPreOutputParallelWithServer"
TARGET_SPECS = [
    {
        "target_name": "IGameSystem_ClientPreEntityThink",
        "rename_to": "GameSystem_OnClientPreEntityThink",
        "dispatch_rank": 0,
    },
]
VIA_INTERNAL_WRAPPER = True
INTERNAL_RENAME_TO = "CLoopModeGame_OnClientPreOutputParallelWithServerInternal"
MULTI_ORDER = "scan"
EXPECTED_DISPATCH_COUNT = 3


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
    """Resolve target function(s) via IGameSystem dispatch and write YAML."""
    _ = skill_name, old_yaml_map
    return await preprocess_igamesystem_dispatch_skill(
        session=session,
        expected_outputs=expected_outputs,
        new_binary_dir=new_binary_dir,
        platform=platform,
        image_base=image_base,
        source_yaml_stem=SOURCE_YAML_STEM,
        target_specs=TARGET_SPECS,
        via_internal_wrapper=VIA_INTERNAL_WRAPPER,
        internal_rename_to=INTERNAL_RENAME_TO,
        multi_order=MULTI_ORDER,
        expected_dispatch_count=EXPECTED_DISPATCH_COUNT,
        debug=debug,
    )
