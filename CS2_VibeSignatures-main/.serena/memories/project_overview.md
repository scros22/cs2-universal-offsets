# Project overview

Purpose:
- Generate CS2 signatures/offsets through Agent SKILLs + MCP calls (IDA Pro based).
- Minimize or remove manual reverse-engineering work during game updates.
- Reuse old-version signatures whenever possible to reduce token cost and runtime.

Current coverage:
- Fully automated update path is explicitly provided for CounterStrikeSharp and CS2Fixes.
- Gamedata outputs are also maintained for swiftlys2, plugify, cs2kz-metamod, modsharp, and CS2Surf/Timer (with project-specific skipped symbols).

Core workflow:
1. Download binaries:
   - `python download_bin.py -gamever <ver>`
2. Analyze binaries and generate per-symbol YAML from `config.yaml`:
   - `python ida_analyze_bin.py -gamever=<ver> [-configyaml=...] [-modules=...] [-platform=...] [-agent=claude/codex] [-maxretry=3] [-debug]`
3. Convert generated YAML to target gamedata formats:
   - `python update_gamedata.py -gamever <ver> [-debug]`

Requirements:
- Python deps: `pyyaml`, `requests`, `asyncio`, `mcp`, `vdf`
- Agent runtime: Claude or Codex
- Reverse tools: IDA Pro 9.0+, `ida-pro-mcp`, `idalib` (mandatory for `ida_analyze_bin.py`)

Key repository components:
- `download_bin.py`: download CS2 binaries.
- `ida_analyze_bin.py`: orchestrate MCP + SKILL execution and YAML generation.
- `update_gamedata.py`: transform YAML results into each ecosystem's gamedata format.
- `config.yaml`: module/symbol/skill declarations.
- `bin/`: downloaded binaries and generated YAML by game version.
- `dist/`: generated gamedata outputs for supported ecosystems.
- `ida_preprocessor_scripts/`: deterministic preprocessors used by SKILL workflows.
- `patched-py/`: compatibility patches (notably for idalib/idapro environments).

Contribution direction:
- Project encourages new SKILL contributions via PR.
- Typical extension path: locate symbol in IDA -> create project SKILL -> create preprocessor script -> register skill and symbols in `config.yaml`.
- SKILL docs in README cover categories: vtable, regular function, virtual function, global variable, struct offset, and patch signatures.