#!/usr/bin/env python3
"""
CS2KZ Gamedata Update Module

Updates cs2kz-core.games.txt for CS2KZ plugin (VDF format).
"""

import os
import sys

# Add project root to path for imports
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(__file__))))
from gamedata_utils import (
    convert_sig_to_cs2fixes,
    normalize_func_name_colons_to_underscore
)

try:
    import vdf
except ImportError:
    print("Error: Missing required dependency: vdf")
    print("Please install required dependencies with: uv sync")
    vdf = None

# Module metadata
MODULE_NAME = "CS2KZ"
MODULE_ENABLED = True

# Relative path to gamedata file within this dist directory
GAMEDATA_PATH = "gamedata/cs2kz-core.games.txt"


def update(yaml_data, func_lib_map, platforms, dist_dir, alias_to_name_map, debug=False):
    """
    Update CS2KZ cs2kz-core.games.txt file (VDF format).

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
    if vdf is None:
        print("  Error: vdf module not available")
        return 0, 0, [], []

    gamedata_path = os.path.join(dist_dir, GAMEDATA_PATH)

    if not os.path.exists(gamedata_path):
        print(f"  Warning: CS2KZ gamedata not found: {gamedata_path}")
        return 0, 0, [], []

    # Load existing gamedata (handle BOM)
    with open(gamedata_path, "r", encoding="utf-8-sig") as f:
        content = f.read()

    # Parse VDF
    gamedata = vdf.loads(content)

    updated_count = 0
    skipped_count = 0
    updated_symbols = []
    skipped_symbols = []

    # Navigate to csgo section
    csgo = gamedata.get("Games", {}).get("csgo", {})

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
            if platform in yaml_entry and "func_sig" in yaml_entry[platform]:
                sig = convert_sig_to_cs2fixes(yaml_entry[platform]["func_sig"])
                entry[platform] = sig
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
            if platform in yaml_entry:
                # Check for vfunc_index (virtual function offset)
                if "vfunc_index" in yaml_entry[platform]:
                    # CS2KZ uses string values for offsets (same as CS2Fixes)
                    entry[platform] = str(yaml_entry[platform]["vfunc_index"])
                    updated_count += 1
                    if debug:
                        updated_symbols.append({
                            "name": func_name,
                            "type": "offset",
                            "platform": platform
                        })
                # Check for struct_member_offset (struct member offset)
                elif "struct_member_offset" in yaml_entry[platform]:
                    entry[platform] = str(yaml_entry[platform]["struct_member_offset"])
                    updated_count += 1
                    if debug:
                        updated_symbols.append({
                            "name": func_name,
                            "type": "struct_offset",
                            "platform": platform
                        })

    # Write back with BOM
    # Use vdf.dumps() to get string, then manually fix escaped backslashes
    vdf_content = vdf.dumps(gamedata, pretty=True)
    # VDF library escapes backslashes, but CS2KZ expects single backslash
    # Replace \\x with \x in signature strings
    vdf_content = vdf_content.replace("\\\\x", "\\x")

    with open(gamedata_path, "w", encoding="utf-8-sig") as f:
        f.write(vdf_content)

    return updated_count, skipped_count, updated_symbols, skipped_symbols
