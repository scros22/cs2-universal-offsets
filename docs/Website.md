# cs2-sdk.com

[cs2-sdk.com](https://cs2-sdk.com) serves the current dump. It is a front-end over the same JSON files that are committed to this repository, so what you see there is exactly what is in `include/`. The site is updated from `main` after every dump.

## Tabs

| Tab | Shows |
|---|---|
| Overview | Build, dump time, counts, and links into every dataset |
| Status | The dumper's self-checks for this dump, unresolved and auto-healed signatures, known issues; a banner appears on every page when a check failed |
| Patterns | Every signature with pattern, RVA, prototype, prologue bytes and aliases; a detail drawer per function |
| Offsets | Every resolved global, with a kind filter (global / signature / interface) and a module picker |
| Interfaces | Registered interfaces, RTTI class, method count, and each vtable slot |
| Classes | Schema classes and enums per module, with field offsets, types and annotations |
| ConVars | ConVars and ConCommands with value, type, decoded flags and description |
| Net Messages | Message id ↔ name ↔ protobuf, by group |
| Weapons | `CCSWeaponBaseVData` values of every weapon in the dump |
| Game Events | Every registered event with its typed keys |
| Entities | The entity snapshot taken at dump time, with per-class counts |
| Engine | The hand-verified engine structs, with a link to each drop-in header |
| Features | How each feature is built — Internal or External (switch at the top), numbered steps, the exact offsets, globals and functions for this build, and Copy as C++ |
| Collections | Your bookmarks, grouped |
| API | The API reference with live examples |
| Downloads | Every exportable artifact with its size |

`Ctrl+K` opens a search across all datasets. Aliases and class-stripped names are indexed, so `GetInaccuracy`, `C_CSWeaponBaseGun_GetInaccuracy` and `CCSGOInput::CreateMove` all find their function.

## Bookmarks and collections

Star any signature, offset or class to bookmark it, and group bookmarks into collections. A collection can be

- exported as `.hpp`, `.json` or `.txt`,
- shared by link,
- turned into an API request with **Copy as API query** — the `POST /api/query` body that fetches exactly those items, so a project can pull its own subset after every update.

Collections live in your browser; there are no accounts. Each item shows its current value, so after a CS2 update you see the new numbers without redoing anything.

## Downloads

The **Downloads** tab and `GET /api/downloads` list every artifact. `GET /api/export/patterns.txt` produces a plain-text pattern list, and every generated file is served verbatim under `/raw/<path>` (`/raw/cs2.hpp`, `/raw/offsets.json`, `/raw/schemas/client_dll.hpp`, …).

## Sessions

Every endpoint accepts `?session=<id>`; `latest` (the default) is the current dump and the only session the public site keeps. `GET /api/sessions` lists what is available.

## Availability

The site is behind Cloudflare. The front-end retries transient errors (a 52x, a brief 502 during a deploy) a few times before showing anything, and shows a Retry button if they persist. If the site is ever down, the same data is in this repository under `include/` and on the [releases](https://github.com/scros22/cs2-universal-offsets/releases) page.
