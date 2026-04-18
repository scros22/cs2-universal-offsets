#!/usr/bin/env python3
"""
Download depot manifests by exact tag matching from download.yaml.
"""

import argparse
import subprocess
import sys
from pathlib import Path

try:
    import yaml
except ImportError as exc:
    print(f"Error: Missing required dependency: {exc.name}")
    print("Please install required dependencies with: uv sync")
    sys.exit(1)


DEFAULT_CONFIG_FILE = "download.yaml"
DEFAULT_DEPOT_DIR = "cs2_depot"
DEFAULT_APP_ID = "730"
DEFAULT_OS = "all-platform"


class ConfigError(Exception):
    """Configuration validation or lookup error."""


def parse_args() -> argparse.Namespace:
    """Parse command line arguments."""
    parser = argparse.ArgumentParser(
        description="Download declared depot manifests for a specific tag."
    )
    parser.add_argument("-tag", required=True, help="Exact tag to match in download.yaml")
    parser.add_argument(
        "-config",
        default=DEFAULT_CONFIG_FILE,
        help=f"Path to download config file (default: {DEFAULT_CONFIG_FILE})",
    )
    parser.add_argument(
        "-depotdir",
        default=DEFAULT_DEPOT_DIR,
        help=f"Output depot directory (default: {DEFAULT_DEPOT_DIR})",
    )
    parser.add_argument(
        "-app",
        default=DEFAULT_APP_ID,
        help=f"Steam app id (default: {DEFAULT_APP_ID})",
    )
    parser.add_argument(
        "-os",
        default=DEFAULT_OS,
        help=f"DepotDownloader -os value (default: {DEFAULT_OS})",
    )
    return parser.parse_args()


def load_downloads(config_path: str) -> list[dict]:
    """Load and validate downloads list from YAML config."""
    path = Path(config_path)
    if not path.is_file():
        raise ConfigError(f"Config file not found: {config_path}")

    try:
        with path.open("r", encoding="utf-8") as handle:
            config = yaml.safe_load(handle) or {}
    except yaml.YAMLError as exc:
        raise ConfigError(f"Invalid YAML in config file: {config_path}") from exc
    except OSError as exc:
        raise ConfigError(f"Failed to read config file: {config_path}") from exc

    if not isinstance(config, dict):
        raise ConfigError("Config root must be a mapping/object")

    downloads = config.get("downloads")
    if not isinstance(downloads, list):
        raise ConfigError("Config field 'downloads' must be a list")
    for index, entry in enumerate(downloads):
        if not isinstance(entry, dict):
            raise ConfigError(f"Config field 'downloads[{index}]' must be a mapping/object")
    return downloads


def find_download_entry(downloads: list[dict], tag: str) -> dict:
    """Find exactly one download entry by exact tag match."""
    matches = [entry for entry in downloads if isinstance(entry, dict) and entry.get("tag") == tag]
    if not matches:
        raise ConfigError(f"Tag not found in downloads config: {tag}")
    if len(matches) > 1:
        raise ConfigError(f"Duplicate tag entries found in downloads config: {tag}")

    entry = matches[0]
    manifests = entry.get("manifests")
    if not isinstance(manifests, dict):
        raise ConfigError(f"Download entry for tag '{tag}' must contain mapping field 'manifests'")
    return entry


def download_manifests(
    manifests: dict,
    app: str,
    os_name: str,
    depot_dir: str,
    branch: str | None = None,
) -> None:
    """Invoke DepotDownloader once per declared depot manifest."""
    if not isinstance(manifests, dict):
        raise ConfigError("Field 'manifests' must be a mapping of depot to manifest")

    for depot, manifest in manifests.items():
        command = [
            "DepotDownloader",
            "-app",
            str(app),
            "-depot",
            str(depot),
            "-os",
            str(os_name),
            "-dir",
            str(depot_dir),
        ]
        if branch:
            command.extend(["-branch", branch])
        command.extend(["-manifest", str(manifest)])
        print(f"Running: {' '.join(command)}")
        subprocess.run(command, check=True)


def main() -> int:
    """Main entry point."""
    args = parse_args()

    try:
        downloads = load_downloads(args.config)
        entry = find_download_entry(downloads, args.tag)
        manifests = entry["manifests"]
        branch = entry.get("branch")
        if branch is not None and not isinstance(branch, str):
            raise ConfigError(f"Download entry for tag '{args.tag}' field 'branch' must be a string")

        name = entry.get("name")
        if name:
            print(f"Matched tag '{args.tag}' ({name})")
        else:
            print(f"Matched tag '{args.tag}'")
        print(f"Manifest count: {len(manifests)}")

        download_manifests(
            manifests=manifests,
            app=args.app,
            os_name=args.os,
            depot_dir=args.depotdir,
            branch=branch,
        )
    except ConfigError as exc:
        print(f"Error: {exc}")
        return 1
    except FileNotFoundError:
        print("Error: DepotDownloader executable not found in PATH")
        return 1
    except subprocess.CalledProcessError as exc:
        print(f"Error: DepotDownloader failed with exit code {exc.returncode}")
        return exc.returncode or 1

    print("All manifest downloads completed successfully.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
