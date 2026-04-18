#!/usr/bin/env python3
"""
CS2Surf Gamedata Update Module

Updates gamedata for CS2Surf plugin.
"""

import os
import sys

# Add project root to path for imports
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(__file__))))

from gamedata_utils import (
    load_jsonc,
    save_jsonc,
    convert_sig_to_css,
    normalize_func_name_colons_to_underscore,
)

# Module metadata
MODULE_NAME = "CS2Surf"
MODULE_ENABLED = True

# Relative path to gamedata file within this dist directory
GAMEDATA_PATH = "gamedata/cs2surf-core.games.jsonc"


def update(yaml_data, func_lib_map, platforms, dist_dir, alias_to_name_map, debug=False):
    """
    Update CS2Surf gamedata file.

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
        print(f"  Warning: {gamedata_path} not found")
        return 0, 0, [], []

    gamedata = load_jsonc(gamedata_path)

    updated_count = 0
    skipped_count = 0
    updated_symbols = []
    skipped_symbols = []

    # Update Signature section (byte signatures)
    sig_updated, sig_skipped, sig_updated_syms, sig_skipped_syms = _update_signatures(
        gamedata, yaml_data, func_lib_map, platforms, alias_to_name_map, debug
    )
    updated_count += sig_updated
    skipped_count += sig_skipped
    updated_symbols.extend(sig_updated_syms)
    skipped_symbols.extend(sig_skipped_syms)

    # Update Offset section (virtual function indices)
    off_updated, off_skipped, off_updated_syms, off_skipped_syms = _update_offsets(
        gamedata, yaml_data, platforms, alias_to_name_map, debug
    )
    updated_count += off_updated
    skipped_count += off_skipped
    updated_symbols.extend(off_updated_syms)
    skipped_symbols.extend(off_skipped_syms)

    save_jsonc(gamedata_path, gamedata)

    return updated_count, skipped_count, updated_symbols, skipped_symbols


def _update_signatures(gamedata, yaml_data, _func_lib_map, platforms, alias_to_name_map, debug):
    """Update Signature section with byte signatures from YAML."""
    updated_count = 0
    skipped_count = 0
    updated_symbols = []
    skipped_symbols = []

    if "Signature" not in gamedata:
        return updated_count, skipped_count, updated_symbols, skipped_symbols

    signatures = gamedata["Signature"]

    for sig_name, sig_data in signatures.items():
        # Normalize the signature name for YAML lookup
        normalized_name = normalize_func_name_colons_to_underscore(sig_name)

        # Try to find in YAML data (check aliases first)
        yaml_name = alias_to_name_map.get(normalized_name, normalized_name)

        if yaml_name not in yaml_data:
            skipped_count += 1
            if debug:
                skipped_symbols.append({
                    "name": sig_name,
                    "reason": "no matching YAML data"
                })
            continue

        yaml_entry = yaml_data[yaml_name]

        for platform in platforms:
            if platform not in sig_data:
                continue

            yaml_platform = yaml_entry.get(platform, {})
            yaml_sig = yaml_platform.get("signature")

            if yaml_sig:
                # Convert signature format (replace ?? with ?)
                converted_sig = convert_sig_to_css(yaml_sig)
                if sig_data[platform] != converted_sig:
                    sig_data[platform] = converted_sig
                    updated_count += 1
                    if debug:
                        updated_symbols.append({
                            "name": sig_name,
                            "type": "signature",
                            "platform": platform
                        })

    return updated_count, skipped_count, updated_symbols, skipped_symbols


def _update_offsets(gamedata, yaml_data, platforms, alias_to_name_map, debug):
    """Update Offset section with virtual function indices from YAML."""
    updated_count = 0
    skipped_count = 0
    updated_symbols = []
    skipped_symbols = []

    if "Offset" not in gamedata:
        return updated_count, skipped_count, updated_symbols, skipped_symbols

    offsets = gamedata["Offset"]

    for offset_name, offset_data in offsets.items():
        # Normalize the offset name for YAML lookup
        normalized_name = normalize_func_name_colons_to_underscore(offset_name)

        # Try to find in YAML data (check aliases first)
        yaml_name = alias_to_name_map.get(normalized_name, normalized_name)

        if yaml_name not in yaml_data:
            skipped_count += 1
            if debug:
                skipped_symbols.append({
                    "name": offset_name,
                    "reason": "no matching YAML data"
                })
            continue

        yaml_entry = yaml_data[yaml_name]

        for platform in platforms:
            if platform not in offset_data:
                continue

            yaml_platform = yaml_entry.get(platform, {})

            # Check for vfunc_index (virtual function index)
            yaml_index = yaml_platform.get("vfunc_index")

            if yaml_index is not None:
                if offset_data[platform] != yaml_index:
                    offset_data[platform] = yaml_index
                    updated_count += 1
                    if debug:
                        updated_symbols.append({
                            "name": offset_name,
                            "type": "vfunc_index",
                            "platform": platform
                        })

    return updated_count, skipped_count, updated_symbols, skipped_symbols
