# copy_depot_bin

## Overview
Copies CS2 binaries from a local Steam depot into the repository's versioned `bin/` layout based on module entries in `config.yaml`, so later analysis steps can work with locally extracted binaries instead of downloaded ones. It also supports `-checkonly` mode, which performs a read-only readiness scan of expected target binaries and lets CI skip depot download when the cache is already complete.

## Responsibilities
- Parse CLI arguments for game version, output directory, platform filter, depot root, config path, and `-checkonly` mode.
- Read `config.yaml` and extract module metadata needed for either binary copying or readiness checking.
- Normalize expected source/target entries through `iter_module_entries`, including `windows` / `linux` expansion and `all-platform` handling.
- In copy mode, resolve source paths inside the local depot and copy binaries into `bin/<gamever>/<module>/<filename>` while creating parent directories as needed.
- In check-only mode, inspect only the expected target files, summarize ready/missing counts, and return mode-specific exit codes for ready, missing, and configuration-error outcomes.
- Skip existing targets, count success/failure totals, and return a non-zero exit code when any actual copy fails.

## Files Involved (no line numbers)
- `copy_depot_bin.py`
- `config.yaml`
- `bin/<gamever>/<module>/<binary>`
- `<depotdir>/<platform>/<configured module path>`
- `<depotdir>/<configured module path>` when `-platform all-platform`
- `.github/workflows/build-on-self-runner.yml`
- `tests/test_copy_depot_bin.py`
- `ida_analyze_bin.py`

## Architecture
The script now has two branches after shared argument/config handling:
```
parse_args
  -> validate config path
  -> parse_config (load modules with name/path_windows/path_linux)
  -> for each module
      -> iter_module_entries
          -> expand platforms and compute source_path + target_path
  -> if -checkonly
      -> check_module_targets
          -> test os.path.exists(target_path) only
      -> print CHECKONLY_RESULT=ready|missing
      -> return 0 / 1 / 2
  -> else
      -> validate depot directory and create bindir
      -> process_module
          -> skip existing target
          -> verify source exists
          -> copy_file(shutil.copy2)
      -> print summary
      -> return 0 / 1
```
`iter_module_entries` is the shared path-planning layer for both modes, so check-only readiness stays aligned with the target layout that copy mode will actually populate. `check_module_targets` is read-only and never touches depot files; `process_module` remains the state-changing branch that reads the depot and writes into `bin/`.

## Dependencies
- PyYAML (`yaml.safe_load`) for reading `config.yaml`
- Python standard library: `argparse`, `os`, `shutil`, `sys`, `pathlib`
- Local filesystem access for reading depot files and writing into `bin/`
- `config.yaml` module fields: `name`, `path_windows`, `path_linux`
- Local depot layout rooted at `<depotdir>/<platform>/...`, or flat `<depotdir>/...` when `-platform all-platform`
- CI workflow logic in `.github/workflows/build-on-self-runner.yml` that consumes the `-checkonly` exit-code contract

## Notes
- If `-platform` is omitted, both modes expand to `windows` and `linux`; `all-platform` treats the depot as a flat mixed layout but still writes targets under the same `bin/<gamever>/<module>/<filename>` convention.
- Existing target files are skipped and counted as successful work in copy mode.
- Missing source binaries in the depot are counted as failures only in copy mode and will cause exit code `1` after the summary.
- `-checkonly` does not require the depot directory to exist and does not create `bin/`; it only checks whether the expected target paths already exist.
- In `-checkonly` mode, `0` means all expected targets are ready, `1` means at least one target is missing, and `2` means configuration loading or validation failed inside `main()`.
- Modules without `name` are skipped with a warning during config parsing.
- Because both modes share `iter_module_entries`, CI readiness checking and actual copy mode stay aligned on the same expected target set.

## Callers (optional)
- Direct CLI invocation: `python copy_depot_bin.py -gamever=<version> [-bindir=bin] [-platform=windows|linux|all-platform] [-depotdir "path/to/cs2_depot"] [-checkonly]`
- `.github/workflows/build-on-self-runner.yml` runs `uv run copy_depot_bin.py -gamever "$env:GAMEVER" -platform all-platform -checkonly`; exit code `0` sets `bin_ready=true` and skips depot download, while exit code `1` sets `bin_ready=false` and continues to `download_depot.py`.