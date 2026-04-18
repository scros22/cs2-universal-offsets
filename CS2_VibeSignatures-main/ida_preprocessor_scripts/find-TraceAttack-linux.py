#!/usr/bin/env python3
"""Preprocess script for find-TraceAttack-linux skill.

On Linux, FireBulletImpactEvent is a separate function called by TraceAttack,
so TraceAttack is found via xref_funcs (callers of both FireBulletImpactEvent
and CBaseEntity_TakeDamageOld).
"""

from ida_analyze_util import preprocess_common_skill

TARGET_FUNCTION_NAMES = [
    "TraceAttack",
]

FUNC_XREFS = [
    # (func_name, xref_strings_list, xref_signatures_list, xref_funcs_list, exclude_funcs_list, exclude_strings_list)
    (
        "TraceAttack",
        [],
        [],
        ["FireBulletImpactEvent", "CBaseEntity_TakeDamageOld"],
        [],
        [],
    ),
]

GENERATE_YAML_DESIRED_FIELDS = [
    # (symbol_name, generate_yaml_fields)
    (
        "TraceAttack",
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
        generate_yaml_desired_fields=GENERATE_YAML_DESIRED_FIELDS,
        debug=debug,
    )
