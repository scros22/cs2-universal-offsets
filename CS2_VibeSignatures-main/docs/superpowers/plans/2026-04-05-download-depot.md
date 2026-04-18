# Download Depot Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace tag-derived manifest download logic with a `download.yaml`-driven helper script, and make the self-runner workflow consume exact tag matches without parsing or rewriting the tag name.

**Architecture:** Add a small Python CLI at the repo root that loads `download.yaml`, resolves one exact `downloads[].tag` match, and invokes `DepotDownloader` once per declared manifest. Keep `.github/workflows/build-on-self-runner.yml` thin by exporting `TAG` and `GAMEVER` directly from `github.ref_name`, delegating depot download selection to `download_depot.py`, and leaving the rest of the pipeline unchanged.

**Tech Stack:** Python 3.10, `argparse`, `PyYAML`, `subprocess`, `unittest`, GitHub Actions YAML, PowerShell, Windows `cmd`, `uv`

---

## File Map

- Create: `download_depot.py`
  Responsibility: parse CLI args, load `download.yaml`, validate the download list, select the unique exact tag match, run `DepotDownloader` once per declared manifest, and return a non-zero exit code on any configuration or command failure.
- Create: `tests/test_download_depot.py`
  Responsibility: cover the helper script’s exact-match lookup, duplicate/missing tag failure modes, and per-manifest command generation without touching the network.
- Modify: `.github/workflows/build-on-self-runner.yml`
  Responsibility: stop parsing `MANIFESTID`, export `TAG`/`GAMEVER` unchanged from `github.ref_name`, invoke `download_depot.py`, and keep the rest of the pipeline intact.
- Modify: `download.yaml`
  Responsibility: keep `downloads[].tag` aligned with the real Git tag names that trigger the workflow.
- Reference: `docs/superpowers/specs/2026-04-05-download-depot-design.md`
  Responsibility: accepted design baseline for exact tag matching, helper-script responsibilities, and failure semantics.

## Validation Notes

- This repository does not include a local harness for end-to-end GitHub Actions execution on the self-hosted Windows runner.
- Local validation for this change is therefore limited to:
  - `unittest` coverage for `download_depot.py`
  - YAML syntax checks for `.github/workflows/build-on-self-runner.yml` and `download.yaml`
  - text-level confirmation that obsolete `MANIFESTID` parsing is gone
- Runtime validation of the full workflow still requires a real tag push on the approved self-hosted runner.

### Task 1: Cover the helper script with failing tests, then implement it

**Files:**
- Create: `tests/test_download_depot.py`
- Create: `download_depot.py`
- Reference: `docs/superpowers/specs/2026-04-05-download-depot-design.md`

- [ ] **Step 1: Write the failing unit tests for exact tag matching and manifest download dispatch**

```python
import subprocess
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import yaml

import download_depot


class DownloadDepotTests(unittest.TestCase):
    def write_config(self, payload):
        handle = tempfile.NamedTemporaryFile(
            "w",
            encoding="utf-8",
            suffix=".yaml",
            delete=False,
        )
        with handle:
            yaml.safe_dump(payload, handle, sort_keys=False)
        config_path = Path(handle.name)
        self.addCleanup(config_path.unlink, missing_ok=True)
        return str(config_path)

    def test_load_downloads_returns_yaml_list(self):
        config_path = self.write_config(
            {
                "downloads": [
                    {"tag": "14141", "manifests": {"2347771": "111"}},
                ]
            }
        )

        downloads = download_depot.load_downloads(config_path)

        self.assertEqual(downloads[0]["tag"], "14141")
        self.assertEqual(downloads[0]["manifests"], {"2347771": "111"})

    def test_find_download_entry_requires_exact_match(self):
        downloads = [
            {"tag": "14141", "manifests": {"2347771": "111"}},
            {"tag": "release_14141b", "manifests": {"2347773": "222"}},
        ]

        entry = download_depot.find_download_entry(downloads, "release_14141b")

        self.assertEqual(entry["tag"], "release_14141b")
        with self.assertRaises(download_depot.ConfigError):
            download_depot.find_download_entry(downloads, "missing")

    def test_find_download_entry_rejects_duplicate_tag(self):
        downloads = [
            {"tag": "14141", "manifests": {"2347771": "111"}},
            {"tag": "14141", "manifests": {"2347773": "222"}},
        ]

        with self.assertRaises(download_depot.ConfigError) as context:
            download_depot.find_download_entry(downloads, "14141")

        self.assertIn(
            "Duplicate download entries matched tag",
            str(context.exception),
        )

    @patch("download_depot.subprocess.run")
    def test_download_manifests_only_runs_declared_depots(self, mock_run):
        mock_run.side_effect = [
            subprocess.CompletedProcess(args=[], returncode=0),
            subprocess.CompletedProcess(args=[], returncode=0),
        ]
        entry = {
            "tag": "release_14141b",
            "name": "1.41.4.1",
            "manifests": {
                "2347771": "111",
                "2347773": "222",
            },
        }

        result = download_depot.download_manifests(
            entry,
            depot_dir="cs2_depot",
            app_id="730",
            os_name="all-platform",
        )

        self.assertEqual(result, 0)
        self.assertEqual(
            [call.args[0] for call in mock_run.call_args_list],
            [
                [
                    "DepotDownloader",
                    "-app",
                    "730",
                    "-depot",
                    "2347771",
                    "-os",
                    "all-platform",
                    "-dir",
                    "cs2_depot",
                    "-manifest",
                    "111",
                ],
                [
                    "DepotDownloader",
                    "-app",
                    "730",
                    "-depot",
                    "2347773",
                    "-os",
                    "all-platform",
                    "-dir",
                    "cs2_depot",
                    "-manifest",
                    "222",
                ],
            ],
        )
```

- [ ] **Step 2: Run the tests to verify they fail before the helper exists**

Run:

```bash
uv run python -m unittest discover -s tests -p 'test_download_depot.py' -v
```

Expected:

```text
FAIL: test_download_depot (unittest.loader._FailedTest.test_download_depot)
ModuleNotFoundError: No module named 'download_depot'
```

- [ ] **Step 3: Implement the minimal helper script that satisfies the tests and the accepted spec**

```python
#!/usr/bin/env python3
"""
Download depot manifests defined in download.yaml.

Usage:
    python download_depot.py -tag=<tag> [-config=download.yaml] [-depotdir=cs2_depot] [-app=730] [-os=all-platform]
"""

import argparse
import subprocess
import sys
from pathlib import Path

try:
    import yaml
except ImportError as e:
    print(f"Error: Missing required dependency: {e.name}")
    print("Please install required dependencies with: uv sync")
    sys.exit(1)


DEFAULT_CONFIG_FILE = "download.yaml"
DEFAULT_DEPOT_DIR = "cs2_depot"
DEFAULT_APP_ID = "730"
DEFAULT_OS_NAME = "all-platform"


class ConfigError(Exception):
    """Raised when the download.yaml configuration is invalid."""


def parse_args(argv=None):
    parser = argparse.ArgumentParser(
        description="Download depot manifests for an exact tag from download.yaml"
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
        help=f"Directory to download depot files into (default: {DEFAULT_DEPOT_DIR})",
    )
    parser.add_argument(
        "-app",
        default=DEFAULT_APP_ID,
        help=f"Steam app id to pass to DepotDownloader (default: {DEFAULT_APP_ID})",
    )
    parser.add_argument(
        "-os",
        default=DEFAULT_OS_NAME,
        help=f"OS selector to pass to DepotDownloader (default: {DEFAULT_OS_NAME})",
    )
    return parser.parse_args(argv)


def load_downloads(config_path):
    path = Path(config_path)
    if not path.is_file():
        raise ConfigError(f"Config file not found: {config_path}")

    with path.open("r", encoding="utf-8") as file:
        config = yaml.safe_load(file) or {}

    downloads = config.get("downloads")
    if not isinstance(downloads, list):
        raise ConfigError("Config must define a 'downloads' list.")

    return downloads


def find_download_entry(downloads, tag):
    matches = [entry for entry in downloads if isinstance(entry, dict) and entry.get("tag") == tag]
    if not matches:
        raise ConfigError(f"No download entry matched tag: {tag}")
    if len(matches) > 1:
        raise ConfigError(f"Duplicate download entries matched tag: {tag}")

    entry = matches[0]
    manifests = entry.get("manifests")
    if not isinstance(manifests, dict):
        raise ConfigError(f"Download entry for tag '{tag}' must define a manifests mapping.")

    return entry


def build_download_command(depot, manifest, depot_dir, app_id, os_name):
    return [
        "DepotDownloader",
        "-app",
        str(app_id),
        "-depot",
        str(depot),
        "-os",
        str(os_name),
        "-dir",
        str(depot_dir),
        "-manifest",
        str(manifest),
    ]


def download_manifests(entry, depot_dir, app_id, os_name):
    manifests = entry["manifests"]
    print(f"Matched tag: {entry['tag']}")
    if entry.get("name"):
        print(f"Name: {entry['name']}")
    print(f"Manifest count: {len(manifests)}")

    for depot, manifest in manifests.items():
        command = build_download_command(depot, manifest, depot_dir, app_id, os_name)
        print(f"Running: {' '.join(command)}")
        result = subprocess.run(command, check=False)
        if result.returncode != 0:
            print(
                f"DepotDownloader failed for depot {depot} with exit code {result.returncode}"
            )
            return result.returncode

    return 0


def main(argv=None):
    args = parse_args(argv)

    try:
        downloads = load_downloads(args.config)
        entry = find_download_entry(downloads, args.tag)
    except ConfigError as exc:
        print(f"Error: {exc}")
        return 1

    return download_manifests(
        entry,
        depot_dir=args.depotdir,
        app_id=args.app,
        os_name=args.os,
    )


if __name__ == "__main__":
    sys.exit(main())
```

- [ ] **Step 4: Re-run the unit tests to verify the helper behavior passes**

Run:

```bash
uv run python -m unittest discover -s tests -p 'test_download_depot.py' -v
```

Expected:

```text
Four named tests finish with status ok, followed by a final OK summary.
```

- [ ] **Step 5: Commit the helper implementation slice**

Run:

```bash
git add download_depot.py tests/test_download_depot.py
git commit -m "feat(download): 新增下载清单脚本"
```

Expected:

```text
One new commit is created with the exact subject: feat(download): 新增下载清单脚本
```

### Task 2: Rewire the GitHub Actions workflow to use exact tag matching

**Files:**
- Modify: `.github/workflows/build-on-self-runner.yml`
- Reference: `download_depot.py`
- Reference: `docs/superpowers/specs/2026-04-05-download-depot-design.md`

- [ ] **Step 1: Replace the trigger and preflight step so the workflow keeps the tag unchanged**

```yaml
name: Build On Self Runner

on:
  push:
    tags:
      - "*"

permissions:
  contents: write

jobs:
  build:
    if: github.repository == 'HLND2T/CS2_VibeSignatures' || github.repository == 'hzqst/CS2_VibeSignatures'
    environment: win64
    runs-on: [self-hosted, windows, x64]
    env:
      RUNNER_AGENT: ${{ vars.RUNNER_AGENT }}
      PERSISTED_WORKSPACE: ${{ secrets.PERSISTED_WORKSPACE }}

    steps:
      - name: Parse tag and validate environment configuration
        shell: pwsh
        run: |
          $tag = "${{ github.ref_name }}"

          if ([string]::IsNullOrWhiteSpace($tag)) {
            throw "github.ref_name is empty."
          }

          if ([string]::IsNullOrWhiteSpace($env:PERSISTED_WORKSPACE)) {
            throw "PERSISTED_WORKSPACE secret is not configured for the win64 environment."
          }

          if ([string]::IsNullOrWhiteSpace($env:RUNNER_AGENT)) {
            throw "RUNNER_AGENT variable is not configured for the win64 environment."
          }

          "TAG=$tag" | Out-File -FilePath $env:GITHUB_ENV -Encoding utf8 -Append
          "GAMEVER=$tag" | Out-File -FilePath $env:GITHUB_ENV -Encoding utf8 -Append
          "WORKSPACE=${{ github.workspace }}" | Out-File -FilePath $env:GITHUB_ENV -Encoding utf8 -Append

      - name: Checkout repository
        uses: actions/checkout@v4
```

- [ ] **Step 2: Replace the depot download step so the workflow delegates manifest resolution to `download_depot.py`**

```yaml
      - name: Update CS2 depot
        shell: cmd
        run: |
          @echo off
          uv run download_depot.py -tag %TAG% -depotdir "%GITHUB_WORKSPACE%\cs2_depot" -config download.yaml

      - name: Copy depot binaries
        shell: cmd
        run: |
          uv run copy_depot_bin.py -gamever %GAMEVER% -platform all-platform
```

- [ ] **Step 3: Run static validation to confirm the workflow keeps tags unchanged and no longer mentions `MANIFESTID`**

Run:

```bash
uv run python -c "from pathlib import Path; import yaml; yaml.safe_load(Path('.github/workflows/build-on-self-runner.yml').read_text(encoding='utf-8')); print('WORKFLOW_YAML_OK')"
```

Expected:

```text
WORKFLOW_YAML_OK
```

Run:

```bash
rg -n 'tags:|TAG=\\$tag|GAMEVER=\\$tag|download_depot.py -tag %TAG%' .github/workflows/build-on-self-runner.yml
```

Expected:

```text
Matches for the wildcard tag trigger, TAG/GAMEVER export, and helper invocation.
```

Run:

```bash
if rg -n 'MANIFESTID|manifestId|\\-notmatch' .github/workflows/build-on-self-runner.yml; then exit 1; else echo 'NO_MANIFEST_TAG_PARSING'; fi
```

Expected:

```text
NO_MANIFEST_TAG_PARSING
```

- [ ] **Step 4: Commit the workflow rewrite**

Run:

```bash
git add .github/workflows/build-on-self-runner.yml
git commit -m "fix(workflow): 改用下载清单驱动下载"
```

Expected:

```text
One new commit is created with the exact subject: fix(workflow): 改用下载清单驱动下载
```

### Task 3: Align `download.yaml` with real tag names and run final targeted verification

**Files:**
- Modify: `download.yaml`
- Reference: `download_depot.py`
- Reference: `.github/workflows/build-on-self-runner.yml`

- [ ] **Step 1: Rewrite `download.yaml` so every `downloads[].tag` exactly matches the real Git tag names**

```yaml
downloads:
  - tag: "14141"
    name: 1.41.4.1
    manifests:
      "2347771": "8158810264338894897"
      "2347773": "4721707405076024766"

  - tag: "14141b"
    name: 1.41.4.1
    manifests:
      "2347771": "2367650111076067440"
      "2347773": "5170166536177825328"

  - tag: "14141c"
    name: 1.41.4.1
    manifests:
      "2347771": "5302572154886330081"
      "2347773": "5549971910709061943"
```

- [ ] **Step 2: Validate that the config is syntactically correct, has unique tags, and every entry still has a manifest mapping**

Run:

```bash
uv run python -c "from pathlib import Path; import yaml; data = yaml.safe_load(Path('download.yaml').read_text(encoding='utf-8')); downloads = data['downloads']; tags = [item['tag'] for item in downloads]; assert len(tags) == len(set(tags)); assert all(isinstance(item.get('manifests'), dict) for item in downloads); print('DOWNLOAD_CONFIG_OK', ','.join(tags))"
```

Expected:

```text
DOWNLOAD_CONFIG_OK 14141,14141b,14141c
```

- [ ] **Step 3: Run the complete targeted verification set for the helper, workflow YAML, and config**

Run:

```bash
uv run python -m unittest discover -s tests -p 'test_download_depot.py' -v
uv run python -c "from pathlib import Path; import yaml; yaml.safe_load(Path('.github/workflows/build-on-self-runner.yml').read_text(encoding='utf-8')); yaml.safe_load(Path('download.yaml').read_text(encoding='utf-8')); print('WORKFLOW_AND_CONFIG_OK')"
```

Expected:

```text
The unittest command ends with OK, and the YAML command prints WORKFLOW_AND_CONFIG_OK
WORKFLOW_AND_CONFIG_OK
```

Run:

```bash
rg -n 'MANIFESTID|manifestId|\\-notmatch' download_depot.py .github/workflows/build-on-self-runner.yml download.yaml
```

Expected:

```text
No matches.
```

- [ ] **Step 4: Commit the config alignment and verified final state**

Run:

```bash
git add download.yaml
git commit -m "chore(download): 同步下载标签配置"
```

Expected:

```text
One new commit is created with the exact subject: chore(download): 同步下载标签配置
```
