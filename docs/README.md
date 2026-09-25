# Documentation

This folder mirrors the [project wiki](https://github.com/scros22/cs2-universal-offsets/wiki); edit the wiki, not these files.

An SDK generator for Counter-Strike 2. It attaches to a running `cs2.exe` (read-only, through [memflow](https://github.com/memflow/memflow)), walks the engine's own data structures and writes an `include/` tree you can drop straight into a project: schema classes, IDA-style signatures, resolved globals, interfaces with their vtables, protobuf layouts, hand-verified engine structs and runtime catalogues (convars, game events, weapons, entities).

The `include/` tree committed to this repository is always the latest dump. The same data is browsable and queryable at **[cs2-sdk.com](https://cs2-sdk.com)**, and a [Discord bot](Discord-Bot.md) answers from it.

## Current dump

| | |
|---|---|
| CS2 build | **14184** (Steam client build 2000917), dumped 2026-09-25 |
| Dumper | **v2.1.7** — [releases](https://github.com/scros22/cs2-universal-offsets/releases) |
| Signatures | 578 entries, 577 resolve, 527 unique functions, 0 ambiguous matches; every one verified against fresh IDA analysis, self-checked on every dump |
| Globals | 205 — the 32 a2x-compatible `dwXxx` (values identical to a2x), 62 resolved by signature, 111 interface instances |
| Schema | 3,301 classes and 570 enums across 18 modules |
| Interfaces | 110 registered interfaces, primary vtable walked, every one RTTI-named |
| Protobufs | 2,003 message layouts |
| Catalogues | 4,110 convars, 1,185 commands, 196 net messages, 273 game events, 44 weapons, 12 engine structs |

## Thirty-second start

```bash
# one value
curl https://cs2-sdk.com/api/offset/dwLocalPlayerPawn
curl https://cs2-sdk.com/api/pattern/CreateMove

# many values in one request
curl -X POST https://cs2-sdk.com/api/query -H 'content-type: application/json' \
  -d '{"signatures":{"CreateMove":true},"offsets":["dwEntityList"],"schemas":["C_CSPlayerPawn.m_iHealth"]}'

# the whole SDK as one header
curl -O https://cs2-sdk.com/raw/cs2.hpp
```

Or add the repository as a submodule and `#include <cs2.hpp>` — see [Getting Started](Getting-Started.md).

## Pages

| Page | What it covers |
|---|---|
| [Getting Started](Getting-Started) | Using the committed `include/`, the site, or running the dumper yourself |
| [Output Layout](Output-Layout) | Every file the dumper writes and which one to depend on |
| [Signatures](Signatures) | `patterns.json`: fields, resolve kinds, naming and aliases, scanning at runtime |
| [Offsets](Offsets) | The three kinds of resolved globals and how to read them |
| [Schemas](Schemas) | Schema class headers, `SCHEMA_FIELD`, `CHandle<T>::Get()`, entity helpers |
| [Interfaces and Vtables](Interfaces-and-Vtables) | Typed interface structs and `vtables.json` |
| [Protobufs](Protobufs) | Packed protobuf message structs and the net-message table |
| [Engine Structs](Engine-Structs) | Hand-verified non-schema layouts: `CUserCmd`, `CCSGOInput`, `CSwapChainDx11`, … |
| [Catalogues](Catalogues) | ConVars, game events, weapons, entities, buttons, verified features |
| [Website](Website) | cs2-sdk.com: tabs, search, bookmarks and collections, downloads |
| [Status and Self-Healing](Status-and-Self-Healing) | The self-checks behind `status.json` and how stale signatures re-anchor themselves |
| [API](API) | Full reference for the JSON API, including the batch query |
| [Discord Bot](Discord-Bot) | Slash commands, status channel, self-hosting |
| [Contributing](Contributing) | Adding or fixing a signature, the verification standard, code layout |
| [Changelog](Changelog) | Release history |
| [FAQ](FAQ) | Short answers to the questions that come up most |
| [Source 2 VFX/VCS notes](research/SOURCE2_VFX_VCS_INTEL.md) | Archived reverse-engineering note on the material / shader-compile path (April 2026) |
