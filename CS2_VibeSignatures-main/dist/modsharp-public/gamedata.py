#!/usr/bin/env python3
"""
ModSharp Gamedata Update Module

Updates gamedata for ModSharp plugin framework.
Handles JSONC files in .asset/gamedata/ directory.
"""

import os
import sys

# Add project root to path for imports
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(__file__))))
from gamedata_utils import (
    convert_sig_to_css,
    normalize_func_name_colons_to_underscore,
    load_jsonc,
    save_jsonc
)

# Module metadata
MODULE_NAME = "ModSharp"
MODULE_ENABLED = True

# Relative paths to gamedata files within this dist directory
GAMEDATA_DIR = ".asset/gamedata"


def update(yaml_data, func_lib_map, platforms, dist_dir, alias_to_name_map, debug=False):
    """
    Update ModSharp gamedata files.

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
    gamedata_dir = os.path.join(dist_dir, GAMEDATA_DIR)

    if not os.path.isdir(gamedata_dir):
        print(f"  Warning: ModSharp gamedata directory not found: {gamedata_dir}")
        return 0, 0, [], []

    updated_count = 0
    skipped_count = 0
    updated_symbols = []
    skipped_symbols = []

    # Process all .games.jsonc files in the gamedata directory
    for filename in os.listdir(gamedata_dir):
        if not filename.endswith(".games.jsonc"):
            continue

        file_path = os.path.join(gamedata_dir, filename)
        print(f"  Processing {filename}...")

        try:
            gamedata = load_jsonc(file_path)
        except Exception as e:
            print(f"    Error loading {filename}: {e}")
            continue

        file_updated = 0
        file_skipped = 0

        # Update Addresses section (signatures)
        if "Addresses" in gamedata:
            u, s, usyms, ssyms = _update_addresses(
                gamedata["Addresses"],
                yaml_data,
                func_lib_map,
                platforms,
                alias_to_name_map,
                debug
            )
            file_updated += u
            file_skipped += s
            updated_symbols.extend(usyms)
            skipped_symbols.extend(ssyms)

        # Update VFuncs section (virtual function indices)
        if "VFuncs" in gamedata:
            u, s, usyms, ssyms = _update_vfuncs(
                gamedata["VFuncs"],
                yaml_data,
                func_lib_map,
                platforms,
                alias_to_name_map,
                debug
            )
            file_updated += u
            file_skipped += s
            updated_symbols.extend(usyms)
            skipped_symbols.extend(ssyms)

        # Update Offsets section (struct member offsets)
        if "Offsets" in gamedata:
            u, s, usyms, ssyms = _update_offsets(
                gamedata["Offsets"],
                yaml_data,
                func_lib_map,
                platforms,
                alias_to_name_map,
                debug
            )
            file_updated += u
            file_skipped += s
            updated_symbols.extend(usyms)
            skipped_symbols.extend(ssyms)

        # Save the updated gamedata
        if file_updated > 0:
            save_jsonc(file_path, gamedata)
            print(f"    Updated: {file_updated}, Skipped: {file_skipped}")

        updated_count += file_updated
        skipped_count += file_skipped

    return updated_count, skipped_count, updated_symbols, skipped_symbols


def _update_addresses(addresses, yaml_data, func_lib_map, platforms, alias_to_name_map, debug):
    """
    Update Addresses section with signatures from YAML data.

    ModSharp Addresses format:
    - Simple: {"library": "server", "linux": "sig", "windows": "sig"}
    - With factory: {"library": "server", "linux": {"factory": "...", "signature": "..."}}
    """
    updated_count = 0
    skipped_count = 0
    updated_symbols = []
    skipped_symbols = []

    for func_name, entry in addresses.items():
        # Convert :: to _ for matching with YAML data
        yaml_func_name = normalize_func_name_colons_to_underscore(func_name, alias_to_name_map)

        # Determine library for this function
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
            if platform not in yaml_entry:
                continue

            platform_yaml = yaml_entry[platform]
            if "func_sig" not in platform_yaml:
                continue

            sig = convert_sig_to_css(platform_yaml["func_sig"])

            # Check if entry has nested structure (with factory)
            if isinstance(entry.get(platform), dict):
                # Update signature in nested structure
                entry[platform]["signature"] = sig
            else:
                # Update simple signature
                entry[platform] = sig

            updated_count += 1
            if debug:
                updated_symbols.append({
                    "name": func_name,
                    "type": "signature",
                    "platform": platform
                })

    return updated_count, skipped_count, updated_symbols, skipped_symbols


def _update_vfuncs(vfuncs, yaml_data, _func_lib_map, platforms, alias_to_name_map, debug):
    """
    Update VFuncs section with virtual function indices from YAML data.

    ModSharp VFuncs format:
    {"linux": 123, "windows": 456}
    """
    updated_count = 0
    skipped_count = 0
    updated_symbols = []
    skipped_symbols = []

    for func_name, entry in vfuncs.items():
        # Convert :: to _ for matching with YAML data
        yaml_func_name = normalize_func_name_colons_to_underscore(func_name, alias_to_name_map)

        # Find matching YAML data
        yaml_entry = yaml_data.get(yaml_func_name)
        if not yaml_entry:
            skipped_count += 1
            if debug:
                skipped_symbols.append({
                    "name": func_name,
                    "reason": "no matching YAML data (vfunc)"
                })
            continue

        # Update platform vfunc indices
        for platform in platforms:
            if platform not in yaml_entry:
                continue

            platform_yaml = yaml_entry[platform]
            if "vfunc_index" not in platform_yaml:
                continue

            entry[platform] = platform_yaml["vfunc_index"]
            updated_count += 1
            if debug:
                updated_symbols.append({
                    "name": func_name,
                    "type": "vfunc",
                    "platform": platform
                })

    return updated_count, skipped_count, updated_symbols, skipped_symbols


def _update_offsets(offsets, yaml_data, _func_lib_map, platforms, alias_to_name_map, debug):
    """
    Update Offsets section with struct member offsets from YAML data.

    ModSharp Offsets format:
    {"linux": 123, "windows": 456}
    """
    updated_count = 0
    skipped_count = 0
    updated_symbols = []
    skipped_symbols = []

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
            if platform not in yaml_entry:
                continue

            platform_yaml = yaml_entry[platform]

            # Check for vfunc_index (virtual function offset)
            if "vfunc_index" in platform_yaml:
                entry[platform] = platform_yaml["vfunc_index"]
                updated_count += 1
                if debug:
                    updated_symbols.append({
                        "name": func_name,
                        "type": "offset",
                        "platform": platform
                    })
            # Check for struct_member_offset (struct member offset)
            elif "struct_member_offset" in platform_yaml:
                entry[platform] = platform_yaml["struct_member_offset"]
                updated_count += 1
                if debug:
                    updated_symbols.append({
                        "name": func_name,
                        "type": "struct_offset",
                        "platform": platform
                    })

    return updated_count, skipped_count, updated_symbols, skipped_symbols
