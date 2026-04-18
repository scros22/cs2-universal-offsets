#!/usr/bin/env python3
"""Preprocess script for find-CNetworkMessages_FindNetworkMessage skill."""

from ida_analyze_util import preprocess_common_skill

TARGET_FUNCTION_NAMES = [
    "CNetworkMessages_FindNetworkMessage",
]

FUNC_XREFS_WINDOWS = [
    # (func_name, xref_strings_list, xref_signatures_list, xref_funcs_list, exclude_funcs_list, exclude_strings_list)
    (
        "CNetworkMessages_FindNetworkMessage",
        [
            "unknown",
        ],
        [
            "41 ?? FF FF 00 00",
            "41 ?? FF 7F 00 00"
        ],
        [],
        ["CNetworkMessages_FindNetworkMessagePartial", "CNetworkMessages_ConfirmAllMessageHandlersInstalled"],
        [],
    ),
]

FUNC_XREFS_LINUX = [
    # (func_name, xref_strings_list, xref_signatures_list, xref_funcs_list, exclude_funcs_list, exclude_strings_list)
    (
        "CNetworkMessages_FindNetworkMessage",
        [
            "unknown",
        ],
        [
            "81 FB FF FF 00 00",
            "66 81 E2 FF 7F"
        ],
        [],
        ["CNetworkMessages_FindNetworkMessagePartial", "CNetworkMessages_ConfirmAllMessageHandlersInstalled"],
        [],
    ),
]

GENERATE_YAML_DESIRED_FIELDS = [
    # (symbol_name, generate_yaml_fields)
    (
        "CNetworkMessages_FindNetworkMessage",
        [
            "func_name",
            "func_va",
            "func_rva",
            "func_size",
            "func_sig",
            "vtable_name",
            "vfunc_offset",
            "vfunc_index",
        ],
    ),
]

FUNC_VTABLE_RELATIONS = [
    # (func_name, vtable_class)
    ("CNetworkMessages_FindNetworkMessage", "CNetworkMessages"),
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
        func_xrefs=FUNC_XREFS_WINDOWS if platform == "windows" else FUNC_XREFS_LINUX,
        func_vtable_relations=FUNC_VTABLE_RELATIONS,
        generate_yaml_desired_fields=GENERATE_YAML_DESIRED_FIELDS,
        debug=debug,
    )
