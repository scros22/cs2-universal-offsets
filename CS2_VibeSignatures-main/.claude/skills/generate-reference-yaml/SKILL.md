---
name: generate-reference-yaml
description: Generate reference YAML via project CLI into ida_preprocessor_scripts/references/<module>/<func_name>.<platform>.yaml
disable-model-invocation: true
---

# Generate Reference YAML

Use this skill as the unified backend entrypoint through project CLI.

Do not call IDA API directly in this skill. Always run `generate_reference_yaml.py`.

## Required parameters

- `func_name`
- `gamever` (Obtain from dll path via ida-pro-mcp if not specified)
- `module` (Obtain from dll path via ida-pro-mcp if not specified)
- `platform` (Obtain from dll path via ida-pro-mcp if not specified)

## Command examples

### 1) Attach to existing MCP

```bash
uv run generate_reference_yaml.py -gamever 14141 -module engine -platform windows -func_name CNetworkGameClient_RecordEntityBandwidth -mcp_host 127.0.0.1 -mcp_port 13337
```

### 2) Auto-start `idalib-mcp` with binary

```bash
uv run generate_reference_yaml.py -gamever 14141 -module engine -platform windows -func_name CNetworkGameClient_RecordEntityBandwidth -auto_start_mcp -binary bin/14141/engine/engine2.dll
```

## Output path

- `ida_preprocessor_scripts/references/<module>/<func_name>.<platform>.yaml`

## Manual checks after generation

1. `func_va` is credible for current binary/version.
2. `disasm_code` is non-empty and matches target function semantics.
3. `procedure` matches expected semantics when available; it can be an empty string when Hex-Rays is unavailable.
4. `func_name` only confirms the output file targets your requested canonical name; it does not prove address resolution correctness.

## `LLM_DECOMPILE` path wiring

- Generated file path in repository:
  - `ida_preprocessor_scripts/references/<module>/<func_name>.<platform>.yaml`
- In target `find-*.py`, when `LLM_DECOMPILE` uses relative paths, write:
  - `references/<module>/<func_name>.<platform>.yaml`
- Example tuple:
  - `("CNetworkMessages_FindNetworkGroup", "prompt/call_llm_decompile.md", "references/engine/CNetworkGameClient_RecordEntityBandwidth.windows.yaml")`
