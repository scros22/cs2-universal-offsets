#!/usr/bin/env python3
"""Preprocess script for find-CCSPlayerPawn_OnTakeDamage skill."""

from ida_analyze_util import preprocess_common_skill

INHERIT_VFUNCS=[
    # (target_func_name, inherit_vtable_class, base_vfunc_name, generate_func_sig)
    ("CCSPlayerPawn_OnTakeDamage", "CCSPlayerPawn", "CBaseEntity_OnTakeDamage", False),
    ("CCSPlayerPawn_OnTakeDamage_Alive", "CCSPlayerPawn", "CBaseEntity_OnTakeDamage_Alive", False),
    ("CCSPlayerPawn_OnTakeDamage_Dying", "CCSPlayerPawn", "CBaseEntity_OnTakeDamage_Dying", False),
    ("CCSPlayerPawn_OnTakeDamage_Dead", "CCSPlayerPawn", "CBaseEntity_OnTakeDamage_Dead", False),
]

GENERATE_YAML_DESIRED_FIELDS = [
    # (symbol_name, generate_yaml_fields)
    (
        "CCSPlayerPawn_OnTakeDamage",
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
    (
        "CCSPlayerPawn_OnTakeDamage_Alive",
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
    (
        "CCSPlayerPawn_OnTakeDamage_Dying",
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
    (
        "CCSPlayerPawn_OnTakeDamage_Dead",
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
    """Reuse old func_sig first; fallback to vtable index + generated signature when needed."""
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
