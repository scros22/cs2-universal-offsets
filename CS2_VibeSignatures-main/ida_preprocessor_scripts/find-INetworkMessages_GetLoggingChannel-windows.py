#!/usr/bin/env python3
"""Preprocess script for find-INetworkMessages_GetLoggingChannel-windows skill."""

from ida_analyze_util import preprocess_common_skill

TARGET_FUNCTION_NAMES = [
    "INetworkMessages_GetLoggingChannel",
]

LLM_DECOMPILE = [
    # (symbol_name, path_to_prompt, path_to_reference)
    (
        "INetworkMessages_GetLoggingChannel",
        "prompt/call_llm_decompile.md",
        "references/server/CNetworkUtlVectorEmbedded_TryLateResolve_m_vecRenderAttributes.{platform}.yaml",
    ),
]

FUNC_VTABLE_RELATIONS = [
    # (func_name, vtable_class)
    ("INetworkMessages_GetLoggingChannel", "INetworkMessages"),
]

GENERATE_YAML_DESIRED_FIELDS = [
    # (symbol_name, generate_yaml_fields)
    (
        "INetworkMessages_GetLoggingChannel",
        [
            "func_name",
            "vfunc_sig",
            "vfunc_sig_max_match:10", # Allow maximum up to 10 matches instead of 1 unique match. This also stops extending signature bytes when match count reduced down to <= 10. Because we are 100% sure 10 matches points to exact the same template function (but instanced).
            "vfunc_offset",
            "vfunc_index",
            "vtable_name",
        ],
    ),
]

async def preprocess_skill(
    session, skill_name, expected_outputs, old_yaml_map,
    new_binary_dir, platform, image_base, llm_config=None, debug=False,
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
        func_vtable_relations=FUNC_VTABLE_RELATIONS,
        llm_decompile_specs=LLM_DECOMPILE,
        llm_config=llm_config,
        generate_yaml_desired_fields=GENERATE_YAML_DESIRED_FIELDS,
        debug=debug,
    )
