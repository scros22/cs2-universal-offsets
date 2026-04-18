#!/usr/bin/env python3
"""
Depot Binary Copy Script for CS2_VibeSignatures

Copies CS2 binary files from a local Steam depot directory based on entries in config.yaml.

Usage:
    python copy_depot_bin.py -gamever=<version> [-bindir=bin] [-platform=windows|linux|all-platform] [-depotdir=cs2_depot] [-checkonly]

    -gamever: Game version subdirectory name (required)
    -bindir: Directory to save copied binaries (default: bin)
    -platform: Filter by platform (windows, linux, or all-platform). If not specified, copies both.
              all-platform: depot has mixed binaries without platform subdirectories.
    -depotdir: Local depot root directory (default: cs2_depot)

Requirements:
    uv sync
"""

import argparse
import os
import shutil
import sys
from pathlib import Path

try:
    import yaml
except ImportError as e:
    print(f"Error: Missing required dependency: {e.name}")
    print("Please install required dependencies with: uv sync")
    sys.exit(1)


DEFAULT_DEPOT_DIR = "cs2_depot"
DEFAULT_BIN_DIR = "bin"
DEFAULT_CONFIG_FILE = "config.yaml"
CHECKONLY_MISSING_EXIT = 1
CHECKONLY_ERROR_EXIT = 2


def parse_args():
    """Parse command line arguments."""
    parser = argparse.ArgumentParser(
        description="Copy CS2 binary files from a local Steam depot directory"
    )
    parser.add_argument(
        "-bindir",
        default=DEFAULT_BIN_DIR,
        help=f"Directory to save copied binaries (default: {DEFAULT_BIN_DIR})"
    )
    parser.add_argument(
        "-gamever",
        required=True,
        help="Game version subdirectory name (required)"
    )
    parser.add_argument(
        "-platform",
        choices=["windows", "linux", "all-platform"],
        default=None,
        help="Filter by platform (windows, linux, or all-platform). "
             "all-platform: depot has mixed binaries without platform subdirectories. "
             "If not specified, copies both with platform subdirectories."
    )
    parser.add_argument(
        "-depotdir",
        default=DEFAULT_DEPOT_DIR,
        help=f"Local depot root directory (default: {DEFAULT_DEPOT_DIR})"
    )
    parser.add_argument(
        "-config",
        default=DEFAULT_CONFIG_FILE,
        help=f"Path to config.yaml file (default: {DEFAULT_CONFIG_FILE})"
    )
    parser.add_argument(
        "-checkonly",
        action="store_true",
        help=(
            "Only check whether expected target binaries already exist. "
            "Return 0 when all expected targets exist, 1 when any target is missing, "
            "and 2 for configuration or argument errors."
        )
    )

    return parser.parse_args()


def parse_config(config_path):
    """
    Parse the config.yaml file and extract module entries.

    Args:
        config_path: Path to the config.yaml file

    Returns:
        List of dictionaries containing module data
    """
    with open(config_path, "r", encoding="utf-8") as f:
        config = yaml.safe_load(f)

    if config is None:
        return []
    if not isinstance(config, dict):
        raise ValueError("Config root must be a mapping")

    raw_modules = config.get("modules", [])
    if not isinstance(raw_modules, list):
        raise ValueError("'modules' must be a list")

    modules = []
    for module in raw_modules:
        name = module.get("name")
        path_windows = module.get("path_windows")
        path_linux = module.get("path_linux")

        if not name:
            print(f"  Warning: Skipping module without name")
            continue

        modules.append({
            "name": name,
            "path_windows": path_windows,
            "path_linux": path_linux
        })

    return modules


def build_source_path(depot_dir, platform, path, flat=False):
    """
    Build the source file path within the depot directory.

    Args:
        depot_dir: Root depot directory
        platform: Platform name (windows or linux)
        path: Relative file path within the platform depot
        flat: If True, skip the platform subdirectory (all-platform mode)

    Returns:
        Full source file path
    """
    if flat:
        return os.path.normpath(os.path.join(depot_dir, path))
    return os.path.normpath(os.path.join(depot_dir, platform, path))


def copy_file(source_path, target_path):
    """
    Copy a file from source path to target path.

    Args:
        source_path: Local source file path
        target_path: Local target file path

    Returns:
        True if successful, False otherwise
    """
    try:
        print(f"  Copying: {source_path}")

        # Ensure parent directory exists
        os.makedirs(os.path.dirname(target_path), exist_ok=True)

        shutil.copy2(source_path, target_path)

        print(f"  Saved to: {target_path}")
        return True

    except OSError as e:
        print(f"  Copy failed: {e}")
        return False


def iter_module_entries(module, bin_dir, gamever, platform_filter, depot_dir):
    """
    Build expected source/target entries for a module.

    Args:
        module: Dictionary with module info
        bin_dir: Base directory to save binaries
        gamever: Game version subdirectory name
        platform_filter: Optional platform filter
        depot_dir: Root depot directory

    Returns:
        List of dictionaries containing source/target info
    """
    name = module["name"]
    flat = platform_filter == "all-platform"

    if platform_filter and not flat:
        platforms = [platform_filter]
    else:
        platforms = ["windows", "linux"]

    entries = []
    for platform in platforms:
        path = module.get(f"path_{platform}")
        if not path:
            print(f"  Skipping {name} ({platform}): no path defined")
            continue

        filename = Path(path).name
        entries.append({
            "name": name,
            "platform": platform,
            "source_path": build_source_path(depot_dir, platform, path, flat=flat),
            "target_path": os.path.join(bin_dir, gamever, name, filename),
        })

    return entries


def check_module_targets(module, bin_dir, gamever, platform_filter, depot_dir):
    """
    Check whether expected target files already exist for a module.

    Returns:
        Tuple of (ready_count, missing_count)
    """
    ready_count = 0
    missing_count = 0

    for entry in iter_module_entries(module, bin_dir, gamever, platform_filter, depot_dir):
        print(f"\nChecking: {entry['name']} ({entry['platform']})")
        if os.path.exists(entry["target_path"]):
            print(f"  [READY] Target already exists: {entry['target_path']}")
            ready_count += 1
        else:
            print(f"  [MISSING] Target not found: {entry['target_path']}")
            missing_count += 1

    return ready_count, missing_count


def process_module(module, bin_dir, gamever, platform_filter, depot_dir):
    """
    Process a single module: copy binary files for specified platforms.

    Args:
        module: Dictionary with module info (name, path_windows, path_linux)
        bin_dir: Base directory to save binaries
        gamever: Game version subdirectory name
        platform_filter: Optional platform filter (windows, linux, or None for both)
        depot_dir: Root depot directory

    Returns:
        Tuple of (success_count, fail_count)
    """
    success_count = 0
    fail_count = 0
    for entry in iter_module_entries(module, bin_dir, gamever, platform_filter, depot_dir):
        print(f"\nProcessing: {entry['name']} ({entry['platform']})")

        # Skip if already exists
        if os.path.exists(entry["target_path"]):
            print(f"  [SKIP] File already exists, skipping copy: {entry['target_path']}")
            success_count += 1
            continue

        # Build source path and verify it exists
        if not os.path.exists(entry["source_path"]):
            print(f"  [ERROR] Source file not found in depot: {entry['source_path']}")
            fail_count += 1
            continue

        # Copy file
        if copy_file(entry["source_path"], entry["target_path"]):
            success_count += 1
        else:
            fail_count += 1

    return success_count, fail_count


def main():
    """Main entry point."""
    args = parse_args()

    config_path = args.config
    bin_dir = args.bindir
    gamever = args.gamever
    platform_filter = args.platform
    depot_dir = args.depotdir
    error_exit = CHECKONLY_ERROR_EXIT if args.checkonly else 1

    # Validate config file exists
    if not os.path.exists(config_path):
        print(f"Error: Config file not found: {config_path}")
        return error_exit

    # Validate depot directory exists
    if not args.checkonly and not os.path.isdir(depot_dir):
        print(f"Error: Depot directory not found: {depot_dir}")
        return 1

    # Create bin directory if needed
    if not args.checkonly:
        os.makedirs(bin_dir, exist_ok=True)

    print(f"Config file: {config_path}")
    print(f"Binary directory: {bin_dir}")
    print(f"Game version: {gamever}")
    print(f"Depot directory: {depot_dir}")
    if platform_filter:
        print(f"Platform filter: {platform_filter}")
    if args.checkonly:
        print("Check-only mode: enabled")

    # Parse config
    print("\nParsing config...")
    try:
        modules = parse_config(config_path)
    except (ValueError, yaml.YAMLError) as e:
        print(f"Error: Failed to parse config file: {e}")
        return error_exit

    print(f"Found {len(modules)} modules to process")

    if not modules:
        print("No modules found in config.")
        return 0

    if args.checkonly:
        total_ready = 0
        total_missing = 0

        for module in modules:
            ready, missing = check_module_targets(
                module, bin_dir, gamever, platform_filter, depot_dir
            )
            total_ready += ready
            total_missing += missing

        print(f"\n{'=' * 50}")
        print(f"Check-only summary: {total_ready} ready, {total_missing} missing")

        if total_missing > 0:
            print("CHECKONLY_RESULT=missing")
            return CHECKONLY_MISSING_EXIT

        print("CHECKONLY_RESULT=ready")
        return 0

    # Process each module
    total_success = 0
    total_fail = 0

    for module in modules:
        success, fail = process_module(module, bin_dir, gamever, platform_filter, depot_dir)
        total_success += success
        total_fail += fail

    # Summary
    print(f"\n{'='*50}")
    print(f"Completed: {total_success} successful, {total_fail} failed")

    if total_fail > 0:
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
