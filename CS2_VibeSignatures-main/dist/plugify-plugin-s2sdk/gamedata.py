#!/usr/bin/env python3
"""
Plugify Gamedata Update Module

Updates gamedata.jsonc for Plugify S2SDK plugin.
"""

import os
import sys

# Add project root to path for imports
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(__file__))))
from gamedata_utils import (
    convert_sig_to_swiftly,
    normalize_func_name_colons_to_underscore,
    load_jsonc,
    save_jsonc
)

# Module metadata
MODULE_NAME = "Plugify"
MODULE_ENABLED = True

# Relative path to gamedata file within this dist directory
GAMEDATA_PATH = "assets/gamedata.jsonc"

# Platform key mapping: windows -> win64, linux -> linuxsteamrt64
PLATFORM_MAP = {
    "windows": "win64",
    "linux": "linuxsteamrt64"
}


def update(yaml_data, func_lib_map, platforms, dist_dir, alias_to_name_map, debug=False):
    """
    Update Plugify gamedata.jsonc file.

    Args:
        yaml_data: Loaded YAML data
        func_lib_map: Function name to library mapping
        platforms: List of platforms to update
        dist_dir: Path to this module's dist directory
        alias_to_name_map: Mapping from aliases to function names
        debug: If True, collect updated and skipped symbols info

    Returns:
        Tuple of (updated_count, skipped_count, updated_symbols, skipped_symbols)
    """
    gamedata_path = os.path.join(dist_dir, GAMEDATA_PATH)

    if not os.path.exists(gamedata_path):
        print(f"  Warning: Plugify gamedata not found: {gamedata_path}")
        return 0, 0, [], []

    gamedata = load_jsonc(gamedata_path)

    updated_count = 0
    skipped_count = 0
    updated_symbols = []
    skipped_symbols = []

    # Process csgo section (main game)
    csgo = gamedata.get("csgo", {})

    # Update Signatures
    signatures = csgo.get("Signatures", {})
    for func_name, entry in signatures.items():
        # Convert :: to _ for matching with YAML data
        yaml_func_name = normalize_func_name_colons_to_underscore(func_name, alias_to_name_map)

        # Determine library
        library = entry.get("library")
        if not library and yaml_func_name in func_lib_map:
            library = func_lib_map[yaml_func_name]

        if not library:
            skipped_count += 1
            if debug:
                skipped_symbols.append({
                    "name": func_name,
                    "reason": "unknown library"
                })
            continue

        # Find matching YAML data
        yaml_entry = yaml_data.get(yaml_func_name)
        if not yaml_entry or yaml_entry.get("library") != library:
            skipped_count += 1
            if debug:
                skipped_symbols.append({
                    "name": func_name,
                    "reason": "no matching YAML data"
                })
            continue

        # Update platform signatures
        for platform in platforms:
            plugify_platform = PLATFORM_MAP.get(platform)
            if not plugify_platform:
                continue
            if platform in yaml_entry and "func_sig" in yaml_entry[platform]:
                sig = convert_sig_to_swiftly(yaml_entry[platform]["func_sig"])
                entry[plugify_platform] = sig
                updated_count += 1
                if debug:
                    updated_symbols.append({
                        "name": func_name,
                        "type": "signature",
                        "platform": platform
                    })

    # Update Offsets
    offsets = csgo.get("Offsets", {})
    for func_name, entry in offsets.items():
        # Convert :: to _ for matching with YAML data
        yaml_func_name = normalize_func_name_colons_to_underscore(func_name, alias_to_name_map)

        # Find matching YAML data
        yaml_entry = yaml_data.get(yaml_func_name)
        if not yaml_entry:
            skipped_count += 1
            if debug:
                skipped_symbols.append({
                    "name": func_name,
                    "reason": "no matching YAML data (offset)"
                })
            continue

        # Update platform offsets
        for platform in platforms:
            plugify_platform = PLATFORM_MAP.get(platform)
            if not plugify_platform:
                continue
            if platform in yaml_entry:
                # Check for vfunc_index (virtual function offset)
                if "vfunc_index" in yaml_entry[platform]:
                    entry[plugify_platform] = yaml_entry[platform]["vfunc_index"]
                    updated_count += 1
                    if debug:
                        updated_symbols.append({
                            "name": func_name,
                            "type": "offset",
                            "platform": platform
                        })
                # Check for struct_member_offset (struct member offset)
                elif "struct_member_offset" in yaml_entry[platform]:
                    entry[plugify_platform] = yaml_entry[platform]["struct_member_offset"]
                    updated_count += 1
                    if debug:
                        updated_symbols.append({
                            "name": func_name,
                            "type": "struct_offset",
                            "platform": platform
                        })

    # Write back
    save_jsonc(gamedata_path, gamedata)

    return updated_count, skipped_count, updated_symbols, skipped_symbols
