# update_gamedata

## Overview
Generate and update gamedata (JSON/VDF/JSONC, etc.) for multiple plugins/frameworks from versioned YAML signature data, enabling unified cross-platform synchronization of signatures and offsets.

## Responsibilities
- Parse command-line arguments and read `config.yaml`.
- Load YAML signature files for each module under `bin_dir/gamever`.
- Build mappings from function names to library/category/alias.
- Convert YAML signatures into target formats and write back to corresponding gamedata files.
- Output update/skip statistics.

## Files Involved (no line numbers)
- update_gamedata.py
- config.yaml
- bin/<gamever>/<module>/<func>.<platform>.yaml
- CounterStrikeSharp/gamedata/gamedata.json
- cs2fixes/gamedata/cs2fixes.games.txt
- cs2kz/gamedata/cs2kz-core.games.txt
- SwiftlyS2/gamedata/signatures.jsonc
- SwiftlyS2/gamedata/offsets.jsonc
- plugify/gamedata/gamedata.jsonc

## Architecture
Core flow is a serial pipeline of "load config -> aggregate YAML -> update by format":
```
parse_args
  -> load_config
  -> build_function_library_map / build_alias_to_name_map
  -> load_all_yaml_data (load module signature YAML under bin_dir/gamever)
  -> update_counterstrikesharp (JSON)
  -> update_cs2fixes (VDF)
  -> update_cs2kz (VDF)
  -> update_swiftlys2 (JSONC: signatures/offsets)
  -> update_plugify (JSONC)
```
Format conversion is handled by `convert_sig_to_css` / `convert_sig_to_cs2fixes` / `convert_sig_to_swiftly`; names containing `::` are mapped through `normalize_func_name_colons_to_underscore` and `alias_to_name_map`. VDF output handles backslash escaping to satisfy target plugin format requirements.

## Dependencies
- PyYAML (read `config.yaml` and YAML signatures)
- requests (unused)
- vdf (parse/generate VDF)
- JSON/JSONC read-write (builtin `json` + JSONC comment stripping)
- Directory layout: `bin/<gamever>/<module>/` and target gamedata paths for each plugin

## Notes
- JSONC write-back does not preserve comments (`save_jsonc` writes plain JSON directly).
- Missing YAML files trigger a warning and are skipped.
- Incomplete `::` name or alias mapping causes skips.
- VDF output must replace `\\x` with `\x`; otherwise CS2Fixes/CS2KZ parsing will not match.

## Callers (optional)
- Direct CLI invocation: `python update_gamedata.py -gamever 14135 [-debug]`