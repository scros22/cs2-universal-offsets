#!/usr/bin/env python3
"""
Gamedata Utilities for CS2_VibeSignatures

Shared utility functions for gamedata update modules.
"""

import json


# =============================================================================
# Signature Format Converters
# =============================================================================

def convert_sig_to_css(sig):
    """
    Convert YAML signature to CounterStrikeSharp format.

    YAML: "48 89 5C 24 ?? 48 8B D9"
    CSS:  "48 89 5C 24 ? 48 8B D9"

    Args:
        sig: Signature string from YAML

    Returns:
        Converted signature string
    """
    return sig.replace("??", "?")


def convert_sig_to_cs2fixes(sig):
    """
    Convert YAML signature to CS2Fixes VDF format.

    YAML: "48 89 5C 24 ?? 48 8B D9"
    VDF:  "\\x48\\x89\\x5C\\x24\\x2A\\x48\\x8B\\xD9"

    Args:
        sig: Signature string from YAML

    Returns:
        Converted signature string with \\xHH format
    """
    parts = sig.split()
    result = []
    for part in parts:
        if part == "??":
            result.append("\\x2A")
        else:
            result.append(f"\\x{part}")
    return "".join(result)


def convert_sig_to_swiftly(sig):
    """
    Convert YAML signature to Swiftly format.

    YAML: "48 89 5C 24 ?? 48 8B D9"
    Swiftly: "48 89 5C 24 ? 48 8B D9"

    Args:
        sig: Signature string from YAML

    Returns:
        Converted signature string
    """
    return sig.replace("??", "?")


# =============================================================================
# Name Normalization
# =============================================================================

def normalize_func_name_colons_to_underscore(name, alias_to_name_map=None):
    """
    Convert function name from double-colon format to underscore format.

    First looks up the name in the alias_to_name_map from config.yaml.
    If not found, falls back to simple :: to _ replacement.

    Example: CCSPlayerController::ChangeTeam -> CCSPlayerController_ChangeTeam

    Args:
        name: Function name with double colons
        alias_to_name_map: Optional mapping from aliases to names (from config.yaml)

    Returns:
        Function name with underscores
    """
    # First try to find in config.yaml aliases
    if alias_to_name_map and name in alias_to_name_map:
        return alias_to_name_map[name]

    # Fallback: simple replacement
    return name.replace("::", "_")


# =============================================================================
# JSONC File Handling
# =============================================================================

def strip_jsonc_comments(content):
    """
    Strip comments from JSONC content.

    Removes both single-line (//) and multi-line (/* */) comments,
    while preserving strings that might contain comment-like patterns.

    Args:
        content: JSONC content string

    Returns:
        JSON content string without comments
    """
    result = []
    i = 0
    in_string = False
    escape_next = False

    while i < len(content):
        char = content[i]

        if escape_next:
            result.append(char)
            escape_next = False
            i += 1
            continue

        if char == '\\' and in_string:
            result.append(char)
            escape_next = True
            i += 1
            continue

        if char == '"' and not escape_next:
            in_string = not in_string
            result.append(char)
            i += 1
            continue

        if not in_string:
            # Check for single-line comment
            if char == '/' and i + 1 < len(content) and content[i + 1] == '/':
                # Skip until end of line
                while i < len(content) and content[i] != '\n':
                    i += 1
                continue

            # Check for multi-line comment
            if char == '/' and i + 1 < len(content) and content[i + 1] == '*':
                i += 2
                # Skip until */
                while i + 1 < len(content):
                    if content[i] == '*' and content[i + 1] == '/':
                        i += 2
                        break
                    i += 1
                continue

        result.append(char)
        i += 1

    return ''.join(result)


def load_jsonc(file_path):
    """
    Load and parse a JSONC file (JSON with comments).

    Args:
        file_path: Path to the JSONC file

    Returns:
        Parsed JSON data as dictionary
    """
    with open(file_path, "r", encoding="utf-8") as f:
        content = f.read()
    content = strip_jsonc_comments(content)
    return json.loads(content)


def save_jsonc(file_path, data, original_content=None):
    """
    Save data to a JSONC file, preserving comments if original content provided.

    Since preserving comments is complex, we just write clean JSON for now.

    Args:
        file_path: Path to the JSONC file
        data: Data to save
        original_content: Original file content (unused for now)
    """
    with open(file_path, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=4)
        f.write("\n")
