#!/usr/bin/env python3
"""Preprocess script for find-CNetworkMessages_SetIsForServer skill."""

from ida_analyze_util import preprocess_common_skill

INHERIT_VFUNCS = [
    (
        "CNetworkMessages_SetIsForServer",
        "CNetworkMessages",
        "../engine/INetworkMessages_SetIsForServer",
        True,
    ),
]


GENERATE_YAML_DESIRED_FIELDS = [
    # (symbol_name, generate_yaml_fields)
    (
        "CNetworkMessages_SetIsForServer",
        [
            "func_name",
            "func_va",
            "func_rva",
            "func_size",
            "vtable_name",
            "vfunc_offset",
            "vfunc_index",
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
    _ = skill_name

    return await preprocess_common_skill(
        session=session,
        expected_outputs=expected_outputs,
        old_yaml_map=old_yaml_map,
        new_binary_dir=new_binary_dir,
        platform=platform,
        image_base=image_base,
        inherit_vfuncs=INHERIT_VFUNCS,
        generate_yaml_desired_fields=GENERATE_YAML_DESIRED_FIELDS,
        debug=debug,
    )
