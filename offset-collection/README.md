# offset-collection/

Curated dump of every signature / offset file we have lying around.
Source of truth for any `Signatures::*` or `Offsets::*` constants used by the build.

## Layout

| Folder              | Contents                                                                 |
|---------------------|--------------------------------------------------------------------------|
| `cs2/`              | CS2 (`client.dll`, `engine2.dll`, ...) signatures and offset headers.    |
| `research-notes/`   | Forum dumps, raw quotes, weapon icon tables, paint kit research, etc.    |
| `legacy-unreal/`    | Unrelated Fortnite / Unreal Engine offset dumps. Kept for reference only.|

## Files in `cs2/`

- `nuvora_client_sigs_apr2026.txt` – Nuvora's verified `client.dll` signatures (April 2026).
  Referenced from code/comments as `br5rhvh.txt` (legacy filename).
- `cs2_sigs.txt` – Older `client.dll` signature dump.
- `enhanced_signatures.h` – Header with research-derived patterns (Raphilaa / koz11).

## Conventions

1. New offsets land in `cs2/`. Name files by *source* + *date* (e.g. `dumper_2026-04.txt`).
2. Anything not directly used by `core/signatures.h` lives here (not at repo root).
3. Don't delete old dumps – move them to `legacy-*/` instead. Diffs across versions are useful.
