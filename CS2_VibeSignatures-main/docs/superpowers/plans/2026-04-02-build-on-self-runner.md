# Build On Self Runner Workflow Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a GitHub Actions workflow at `.github/workflows/build-on-self-runner.yml` that runs on approved self-hosted Windows runners for valid version tags, persists `cs2_depot` and `bin`, executes the repository build pipeline, archives outputs, and publishes a release asset.

**Architecture:** Implement a single GitHub Actions job on `self-hosted` Windows x64 with `environment: win64`. Job-level `env` maps `${{ vars.RUNNER_AGENT }}` and `${{ secrets.PERSISTED_WORKSPACE }}` into step environments, a `pwsh` preflight step validates the tag plus required environment-scoped config, exports normalized tag variables into `GITHUB_ENV`, and all remaining `cmd` steps reuse those variables to create persistent directory junctions, run the existing Python scripts, create a `7z` archive, and publish a GitHub Release.

**Tech Stack:** GitHub Actions YAML, PowerShell, Windows `cmd`, `uv`, repository Python scripts, 7-Zip, `softprops/action-gh-release@v1`

---

## File Map

- Create: `.github/workflows/build-on-self-runner.yml`
  Responsibility: define the entire self-hosted tag-driven release workflow, including trigger rules, tag parsing, `win64` environment config injection, persistent workspace junctions, build steps, archiving, and release publishing.
- Reference: `docs/superpowers/specs/2026-04-02-build-on-self-runner-design.md`
  Responsibility: accepted design baseline for tag parsing rules, failure conditions, and packaging scope.

## Validation Notes

- This repository does not include a local runner harness for end-to-end GitHub Actions execution.
- Implementation validation for this task is therefore static:
  - YAML syntax parse
  - required command/guard presence checks
  - manual sanity check that sample tags map to the intended regex groups
- Runtime verification requires a real tag push on an approved self-hosted runner and is out of scope for local execution.

### Task 1: Scaffold Workflow Trigger, Permissions, and Preflight Parsing

**Files:**
- Create: `.github/workflows/build-on-self-runner.yml`
- Reference: `docs/superpowers/specs/2026-04-02-build-on-self-runner-design.md`

- [ ] **Step 1: Create the workflow skeleton with trigger, permissions, repository guard, and `win64` environment binding**

```yaml
name: Build On Self Runner

on:
  push:
    tags:
      - "v*"

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
      - name: Checkout repository
        uses: actions/checkout@v4
```

- [ ] **Step 2: Add a `pwsh` preflight step that validates the tag format and required environment-scoped configuration**

```yaml
      - name: Parse tag and validate environment configuration
        shell: pwsh
        run: |
          $tag = "${{ github.ref_name }}"
          if ($tag -notmatch '^v(?<gamever>\d+[a-z]?)(?:-(?<manifest>\d+))?$') {
            throw "Unsupported tag format: $tag"
          }

          if ([string]::IsNullOrWhiteSpace($env:PERSISTED_WORKSPACE)) {
            throw "PERSISTED_WORKSPACE secret is not configured for the win64 environment."
          }

          if ([string]::IsNullOrWhiteSpace($env:RUNNER_AGENT)) {
            throw "RUNNER_AGENT variable is not configured for the win64 environment."
          }

          "GAMEVER=$($Matches.gamever)" | Out-File -FilePath $env:GITHUB_ENV -Encoding utf8 -Append
          "MANIFESTID=$($Matches.manifest)" | Out-File -FilePath $env:GITHUB_ENV -Encoding utf8 -Append
          "WORKSPACE=${{ github.workspace }}" | Out-File -FilePath $env:GITHUB_ENV -Encoding utf8 -Append
```

- [ ] **Step 3: Run a static YAML syntax parse after writing the skeleton**

Run:

```bash
uv run python -c "from pathlib import Path; import yaml; yaml.safe_load(Path('.github/workflows/build-on-self-runner.yml').read_text(encoding='utf-8')); print('YAML_OK')"
```

Expected:

```text
YAML_OK
```

- [ ] **Step 4: Confirm the trigger and guard rails are present**

Run:

```bash
rg -n "v\\*|contents: write|environment: win64|runs-on: \\[self-hosted, windows, x64\\]|github.repository|RUNNER_AGENT: \\$\\{\\{ vars\\.RUNNER_AGENT \\}\\}|PERSISTED_WORKSPACE: \\$\\{\\{ secrets\\.PERSISTED_WORKSPACE \\}\\}|Parse tag and validate environment configuration" .github/workflows/build-on-self-runner.yml
```

Expected:

```text
Matches for the trigger, permissions, runner labels, repository guard, and preflight step.
```

### Task 2: Add Persistent Workspace Links and Build Pipeline Steps

**Files:**
- Modify: `.github/workflows/build-on-self-runner.yml`
- Reference: `README.md`
- Reference: `docs/superpowers/specs/2026-04-02-build-on-self-runner-design.md`

- [ ] **Step 1: Add a `cmd` step that creates persistent target directories and safely creates the workspace junctions**

```yaml
      - name: Prepare persisted workspace links
        shell: cmd
        run: |
          @echo off
          if "%PERSISTED_WORKSPACE%"=="" (
            echo PERSISTED_WORKSPACE is not configured.
            exit /b 1
          )

          if not exist "%PERSISTED_WORKSPACE%\\cs2_depot" mkdir "%PERSISTED_WORKSPACE%\\cs2_depot"
          if not exist "%PERSISTED_WORKSPACE%\\bin" mkdir "%PERSISTED_WORKSPACE%\\bin"

          if exist "%WORKSPACE%cs2_depot" (
            pwsh -NoProfile -Command "$item = Get-Item -LiteralPath '%WORKSPACE%\\cs2_depot' -Force; if (-not ($item.Attributes -band [IO.FileAttributes]::ReparsePoint)) { Write-Error 'cs2_depot exists but is not a directory junction or symlink.'; exit 1 }; $target = [string]$item.Target; if ($target -ne '%PERSISTED_WORKSPACE%\\cs2_depot') { Write-Error ('cs2_depot link target mismatch: ' + $target); exit 1 }"
            if errorlevel 1 exit /b 1
          ) else (
            mklink /j "%WORKSPACE%\\cs2_depot" "%PERSISTED_WORKSPACE%\\cs2_depot"
          )

          if exist "%WORKSPACE%bin" (
            pwsh -NoProfile -Command "$item = Get-Item -LiteralPath '%WORKSPACE%\\bin' -Force; if (-not ($item.Attributes -band [IO.FileAttributes]::ReparsePoint)) { Write-Error 'bin exists but is not a directory junction or symlink.'; exit 1 }; $target = [string]$item.Target; if ($target -ne '%PERSISTED_WORKSPACE%\\bin') { Write-Error ('bin link target mismatch: ' + $target); exit 1 }"
            if errorlevel 1 exit /b 1
          ) else (
            mklink /j "%WORKSPACE%\\bin" "%PERSISTED_WORKSPACE%\\bin"
          )
```

- [ ] **Step 2: Add depot download and binary copy steps, including optional manifest handling**

```yaml
      - name: Update CS2 depot
        shell: cmd
        run: |
          @echo off
          set CMD=DepotDownloader -app 730 -os all-platform -dir "%GITHUB_WORKSPACE%\\cs2_depot"
          if not "%MANIFESTID%"=="" set CMD=%CMD% -manifest %MANIFESTID%
          call %CMD%

      - name: Copy depot binaries
        shell: cmd
        run: |
          uv run copy_depot_bin.py -gamever %GAMEVER% -platform all-platform
```

- [ ] **Step 3: Add the analysis, gamedata generation, and C++ test steps**

```yaml
      - name: Analyze binaries
        shell: cmd
        run: |
          uv run ida_analyze_bin.py -gamever %GAMEVER% -agent=%RUNNER_AGENT% -debug

      - name: Update gamedata
        shell: cmd
        run: |
          uv run update_gamedata.py -gamever %GAMEVER% -debug

      - name: Run C++ tests
        shell: cmd
        run: |
          uv run run_cpp_tests.py -gamever %GAMEVER% -fixheader -agent=%RUNNER_AGENT% -debug
```

- [ ] **Step 4: Re-run YAML parsing and verify the pipeline commands are present**

Run:

```bash
uv run python -c "from pathlib import Path; import yaml; yaml.safe_load(Path('.github/workflows/build-on-self-runner.yml').read_text(encoding='utf-8')); print('YAML_OK')"
```

Expected:

```text
YAML_OK
```

Run:

```bash
rg -n "mklink /j|DepotDownloader -app 730 -os all-platform|copy_depot_bin.py|ida_analyze_bin.py|update_gamedata.py|run_cpp_tests.py" .github/workflows/build-on-self-runner.yml
```

Expected:

```text
Matches for the persistent-junction logic and every required repository command.
```

### Task 3: Add Archiving, Release Publishing, and Final Static Verification

**Files:**
- Modify: `.github/workflows/build-on-self-runner.yml`
- Reference: `docs/superpowers/specs/2026-04-02-build-on-self-runner-design.md`

- [ ] **Step 1: Add the 7-Zip archive step with the required include roots and exclude patterns**

```yaml
      - name: Archive release payload
        shell: cmd
        working-directory: ${{ github.workspace }}
        run: |
          7z a gamedata-%GAMEVER%.7z "bin\\%GAMEVER%\\*" "dist\\*" "hl2sdk_cs2\\*" -r ^
          -x!*.dll ^
          -x!*.so ^
          -x!*.i64 ^
          -x!*.id0 ^
          -x!*.id1 ^
          -x!*.id2 ^
          -x!*.nam ^
          -x!*.til ^
          -x!.git ^
          -x!.git-blame-ignore-revs ^
          -x!.gitmodules
```

- [ ] **Step 2: Add the GitHub Release publishing step**

```yaml
      - name: Create release
        uses: softprops/action-gh-release@v1
        if: startsWith(github.ref, 'refs/tags/')
        with:
          name: gamedata-${{ github.ref_name }}
          files: gamedata-${{ env.GAMEVER }}.7z
```

- [ ] **Step 3: Run final static validation on the completed workflow**

Run:

```bash
uv run python -c "from pathlib import Path; import yaml; yaml.safe_load(Path('.github/workflows/build-on-self-runner.yml').read_text(encoding='utf-8')); print('YAML_OK')"
```

Expected:

```text
YAML_OK
```

Run:

```bash
rg -n "gamedata-%GAMEVER%.7z|action-gh-release|refs/tags/|environment: win64|PERSISTED_WORKSPACE|MANIFESTID|RUNNER_AGENT" .github/workflows/build-on-self-runner.yml
```

Expected:

```text
Matches for the archive file name, release action, tag guard, required environment variables, and parsed workflow variables.
```

- [ ] **Step 4: Do a manual spec-to-workflow checklist before handing off**

Checklist:

```text
- Valid tags accepted: v14141, v14141a, v14141a-7617088375292372759
- Invalid tags fail in preflight
- Only HLND2T/CS2_VibeSignatures and hzqst/CS2_VibeSignatures are allowed
- Runner labels are [self-hosted, windows, x64]
- environment `win64` provides `vars.RUNNER_AGENT` and `secrets.PERSISTED_WORKSPACE`
- cs2_depot and bin are mapped into persisted storage via junctions
- DepotDownloader optionally appends -manifest
- copy_depot_bin.py uses -platform all-platform
- ida_analyze_bin.py, update_gamedata.py, run_cpp_tests.py run in order
- Archive includes bin/{GAMEVER}, dist, hl2sdk_cs2
- Archive excludes *.dll, *.so, *.i64, *.id0, *.id1, *.id2, *.nam, *.til, .git, .git-blame-ignore-revs, .gitmodules
- Release uploads gamedata-{GAMEVER}.7z
```
