#!/usr/bin/env python3
"""
CounterStrikeSharp Gamedata Update Module

Updates gamedata.json for CounterStrikeSharp plugin framework.
"""

import json
import os
import sys

# Add project root to path for imports
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(__file__))))
from gamedata_utils import (
    convert_sig_to_css,
    normalize_func_name_colons_to_underscore
)

# Module metadata
MODULE_NAME = "CounterStrikeSharp"
MODULE_ENABLED = True

# Relative path to gamedata file within this dist directory
GAMEDATA_PATH = "config/addons/counterstrikesharp/gamedata/gamedata.json"


def update(yaml_data, func_lib_map, platforms, dist_dir, alias_to_name_map, debug=False):
    """
    Update CounterStrikeSharp gamedata.json file.

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
        print(f"  Warning: CounterStrikeSharp gamedata not found: {gamedata_path}")
        return 0, 0, [], []

    # Load existing gamedata
    with open(gamedata_path, "r", encoding="utf-8") as f:
        gamedata = json.load(f)

    updated_count = 0
    skipped_count = 0
    updated_symbols = []
    skipped_symbols = []

    for func_name, entry in gamedata.items():
        # Convert :: to _ for matching with YAML data
        yaml_func_name = normalize_func_name_colons_to_underscore(func_name, alias_to_name_map)

        # Determine library for this function
        library = None
        if "signatures" in entry and "library" in entry["signatures"]:
            library = entry["signatures"]["library"]
        elif yaml_func_name in func_lib_map:
            library = func_lib_map[yaml_func_name]

        if not library:
            print(f"  Warning: Unknown library for {func_name}, skipping")
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

        # Update signatures
        if "signatures" in entry:
            for platform in platforms:
                if platform in yaml_entry and "func_sig" in yaml_entry[platform]:
                    sig = convert_sig_to_css(yaml_entry[platform]["func_sig"])
                    entry["signatures"][platform] = sig
                    updated_count += 1
                    if debug:
                        updated_symbols.append({
                            "name": func_name,
                            "type": "signature",
                            "platform": platform
                        })

        # Update offsets (vfunc_index or struct_member_offset)
        if "offsets" in entry:
            for platform in platforms:
                if platform in yaml_entry:
                    # Check for vfunc_index (virtual function offset)
                    if "vfunc_index" in yaml_entry[platform]:
                        entry["offsets"][platform] = yaml_entry[platform]["vfunc_index"]
                        updated_count += 1
                        if debug:
                            updated_symbols.append({
                                "name": func_name,
                                "type": "offset",
                                "platform": platform
                            })
                    # Check for struct_member_offset (struct member offset)
                    elif "struct_member_offset" in yaml_entry[platform]:
                        entry["offsets"][platform] = yaml_entry[platform]["struct_member_offset"]
                        updated_count += 1
                        if debug:
                            updated_symbols.append({
                                "name": func_name,
                                "type": "struct_offset",
                                "platform": platform
                            })

    # Write back
    with open(gamedata_path, "w", encoding="utf-8") as f:
        json.dump(gamedata, f, indent=2)
        f.write("\n")

    return updated_count, skipped_count, updated_symbols, skipped_symbols
