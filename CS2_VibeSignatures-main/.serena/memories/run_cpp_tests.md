# run_cpp_tests

## Overview
`run_cpp_tests.py` is a C++ compile-test driver based on `config.yaml`: it probes clang target-triple support, runs runnable test items, compares compiler vtable output against YAML references when enabled, and can optionally invoke an LLM agent to fix C++ headers when vtable differences are detected (`-fixheader`).

## Responsibilities
- Parse CLI arguments (config path, bin dir, game version, clang path, C++ standard, debug flag).
- Parse and validate `cpp_tests` entries in `config.yaml`.
- Detect clang default target triple and probe support for configured target triples.
- Execute only tests whose target triples are supported; classify invalid and compile-failed items.
- Build clang compile commands per test item (includes, defines, additional compiler options).
- Run vtable comparison when `fdump-vtable-layouts` is present.
- Compare against **each** configured `reference_module` (e.g. server + client), not only the first one.
- If `-fixheader` is enabled and differences exist, invoke Claude/Codex agent with an English prompt containing:
  - explicit header paths from `cpp_tests[].headers`
  - vtable difference text (`Differences found: N` + `- ...` lines)
- Print final summary and return process exit code (`1` on compile/invalid/header-fix failures).

## Involved Files (no line numbers)
- run_cpp_tests.py
- cpp_tests_util.py
- .claude/agents/vtable-fixer.md
- config.yaml
- bin/<gamever>/*.yaml

## Architecture
Core flow:
- `main`
  - `parse_args`
  - `parse_config`
  - `get_default_target_triple`
  - `probe_target_support` (per target)
  - `run_one_test` (per runnable test)
    - `_to_list` / `_contains_fdump_vtable_layouts`
    - `build_compile_command`
    - vtable compare loop over `reference_modules`
  - `format_vtable_compare_report` (console output)
  - optional `-fixheader` branch
    - `_resolve_header_paths`
    - `_build_fix_prompt` (English prompt)
    - `run_fix_header_agent` (Claude/Codex with retry/resume)

Data-flow highlights:
- Input: `cpp_tests` from `config.yaml`, local clang environment, YAML references under `bin/<gamever>`.
- Intermediate: temporary per-test object files, collected compiler output, structured compare reports per module.
- Output: console logs, optional agent-driven header edits, exit code.

## Dependencies
- Python stdlib: `argparse`, `json`, `shlex`, `subprocess`, `tempfile`, `pathlib`, `typing`, `uuid`, `sys`.
- Third-party: `PyYAML`.
- External tools: `clang++`, and optional `claude` / `codex` for `-fixheader`.
- Internal module: `cpp_tests_util`.
- Agent prompt file: `.claude/agents/vtable-fixer.md` (for Codex `developer_instructions`).

## -fixheader Behavior
- Trigger condition: `-fixheader` is set **and** at least one compare report has differences.
- Header source: `cpp_tests[].headers` in `config.yaml`.
- Prompt content:
  - target interface/class symbol
  - explicit header file paths
  - vtable differences in console-like format
- Agent execution:
  - Supports Claude and Codex.
  - Retry + resume semantics are implemented.
  - Summary counters include `Header fix agent runs/failures`.

## Claude Permission / Tooling Controls
To reduce fallback to “manual patch only”, `run_cpp_tests.py` supports pass-through options for Claude fix runs:
- CLI-level:
  - `-claude_allowed_tools`
  - `-claude_permission_mode`
  - `-claude_extra_args`
- Per-test override in `config.yaml` (`cpp_tests[]`):
  - `claude_allowed_tools`
  - `claude_permission_mode`
  - `claude_extra_args`
- Override rule: per-test field > CLI option.

## Notes
- Vtable compare runs only when additional options contain `fdump-vtable-layouts`.
- `additional_compiler_options` and `additional_compile_options` are both accepted.
- Unsupported target triples are skipped (not failures).
- Missing required fields (`symbol/cpp/target`) or missing cpp file => `invalid` => failure exit code.
- If target->platform mapping fails, compile may pass but compare is skipped with notes.
- Header-fix failures also contribute to final non-zero exit.

## Callers (optional)
- CLI entrypoint: `python run_cpp_tests.py ...`