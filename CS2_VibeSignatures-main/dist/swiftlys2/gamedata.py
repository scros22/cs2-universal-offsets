#!/usr/bin/env python3
"""
SwiftlyS2 Gamedata Update Module

Updates signatures.jsonc and offsets.jsonc for SwiftlyS2 plugin framework.
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
MODULE_NAME = "SwiftlyS2"
MODULE_ENABLED = True

# Relative paths to gamedata files within this dist directory
SIGNATURES_PATH = "plugin_files/gamedata/cs2/core/signatures.jsonc"
OFFSETS_PATH = "plugin_files/gamedata/cs2/core/offsets.jsonc"


def update(yaml_data, func_lib_map, platforms, dist_dir, alias_to_name_map, debug=False):
    """
    Update SwiftlyS2 signatures.jsonc and offsets.jsonc files.

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
    total_updated = 0
    total_skipped = 0
    all_updated_symbols = []
    all_skipped_symbols = []

    # Update signatures.jsonc
    sig_path = os.path.join(dist_dir, SIGNATURES_PATH)
    if os.path.exists(sig_path):
        updated, skipped, updated_syms, skipped_syms = _update_signatures(
            yaml_data, func_lib_map, platforms, sig_path, alias_to_name_map, debug
        )
        total_updated += updated
        total_skipped += skipped
        all_updated_symbols.extend(updated_syms)
        all_skipped_symbols.extend(skipped_syms)
    else:
        print(f"  Warning: SwiftlyS2 signatures not found: {sig_path}")

    # Update offsets.jsonc
    off_path = os.path.join(dist_dir, OFFSETS_PATH)
    if os.path.exists(off_path):
        updated, skipped, updated_syms, skipped_syms = _update_offsets(
            yaml_data, func_lib_map, platforms, off_path, alias_to_name_map, debug
        )
        total_updated += updated
        total_skipped += skipped
        all_updated_symbols.extend(updated_syms)
        all_skipped_symbols.extend(skipped_syms)
    else:
        print(f"  Warning: SwiftlyS2 offsets not found: {off_path}")

    return total_updated, total_skipped, all_updated_symbols, all_skipped_symbols


def _update_signatures(yaml_data, func_lib_map, platforms, sig_path, alias_to_name_map, debug=False):
    """Update SwiftlyS2 signatures.jsonc file."""
    signatures = load_jsonc(sig_path)

    updated_count = 0
    skipped_count = 0
    updated_symbols = []
    skipped_symbols = []

    for func_name, entry in signatures.items():
        # Convert :: to _ for matching with YAML data
        yaml_func_name = normalize_func_name_colons_to_underscore(func_name, alias_to_name_map)

        # Determine library
        library = entry.get("lib")
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
                sig = convert_sig_to_swiftly(yaml_entry[platform]["func_sig"])
                entry[platform] = sig
                updated_count += 1
                if debug:
                    updated_symbols.append({
                        "name": func_name,
                        "type": "signature",
                        "platform": platform
                    })

    # Write back
    save_jsonc(sig_path, signatures)

    return updated_count, skipped_count, updated_symbols, skipped_symbols


def _update_offsets(yaml_data, func_lib_map, platforms, off_path, alias_to_name_map, debug=False):
    """Update SwiftlyS2 offsets.jsonc file."""
    offsets = load_jsonc(off_path)

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
                    "reason": "no matching YAML data"
                })
            continue

        # Update platform offsets
        for platform in platforms:
            if platform in yaml_entry:
                # Check for vfunc_index (virtual function offset)
                if "vfunc_index" in yaml_entry[platform]:
                    entry[platform] = yaml_entry[platform]["vfunc_index"]
                    updated_count += 1
                    if debug:
                        updated_symbols.append({
                            "name": func_name,
                            "type": "offset",
                            "platform": platform
                        })
                # Check for struct_member_offset (struct member offset)
                elif "struct_member_offset" in yaml_entry[platform]:
                    entry[platform] = yaml_entry[platform]["struct_member_offset"]
                    updated_count += 1
                    if debug:
                        updated_symbols.append({
                            "name": func_name,
                            "type": "struct_offset",
                            "platform": platform
                        })

    # Write back
    save_jsonc(off_path, offsets)

    return updated_count, skipped_count, updated_symbols, skipped_symbols
