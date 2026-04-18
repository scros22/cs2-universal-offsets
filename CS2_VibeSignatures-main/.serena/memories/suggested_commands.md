# Suggested commands (Windows)

Install deps:
- pip install pyyaml requests asyncio mcp vdf

Download binaries:
- python download_bin.py -gamever <version>
  Optional: -bindir=bin -platform=windows|linux -sourcebinsurl=<url> -config=<path>

Analyze binaries with IDA + agent:
- python ida_analyze_bin.py -gamever=<version>
  Optional: -configyaml=<path> -bindir=bin -platform=windows,linux -modules=server,engine -agent=claude|codex -debug

Update gamedata outputs from YAML:
- python update_gamedata.py -gamever <version>
  Optional: -configyaml=<path> -bindir=bin -platform=windows,linux

Agent skill usage examples:
- claude -p "/find-CCSPlayerController_ChangeTeam" --agent sig-finder
- codex exec "Run SKILL: .claude/skills/find-CCSPlayerController_ChangeTeam/SKILL.md"

Manual IDA step (when needed):
- Open the target binary in IDA GUI and start MCP server (Ctrl+Alt+M) before running skills.
