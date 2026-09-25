# API

Base URL: `https://cs2-sdk.com`. Every endpoint returns JSON, sets `Access-Control-Allow-Origin: *`, needs no authentication, and accepts `?session=<id>` (default `latest`). Addresses are hex strings (`"0x27130E8"`).

`GET /api` returns a machine-readable index of everything below, and [`/llms.txt`](https://cs2-sdk.com/llms.txt) is a compact description written for AI agents and IDE assistants.

## Batch query — `POST /api/query`

Fetch any number of signatures, offsets, schema fields and convars in one request.

```bash
curl -X POST https://cs2-sdk.com/api/query -H 'content-type: application/json' -d '{
  "signatures": { "CreateMove": true, "GetInaccuracy": true },
  "offsets":    [ "dwEntityList", "client.dll/dwLocalPlayerPawn" ],
  "schemas":    [ "C_CSPlayerPawn.m_iHealth", "CCSPlayerController" ],
  "convars":    [ "sv_cheats" ],
  "detail":     false
}'
```

```json
{
  "success": true,
  "build": 14184,
  "generated_at": "2026-09-25T15:06:58.227900400+01:00",
  "session": "latest",
  "detail": false,
  "found": {
    "signatures": { "CreateMove": "85 D2 0F 85 ? ? ? ? 48 8B C4 44 88 40 18",
                    "GetInaccuracy": "48 89 5C 24 10 55 56 57 48 81 EC ? ? ? ? 44 0F 29 84 24 80 00 00 00" },
    "offsets":    { "dwEntityList": "0x27130E8", "client.dll/dwLocalPlayerPawn": "0x255E658" },
    "schemas":    { "C_CSPlayerPawn.m_iHealth": "0x34C", "CCSPlayerController": { "m_hPlayerPawn": "0x92C", … } },
    "convars":    { "sv_cheats": { "kind": "convar", "value": "false", "type": "bool", "flags": "0x82100",
                                   "flag_names": ["FCVAR_NOTIFY", "FCVAR_REPLICATED", "FCVAR_RELEASE"],
                                   "description": "Allow cheats on server", "name": "sv_cheats" } }
  },
  "missing":  { "signatures": [], "offsets": [], "schemas": [], "convars": [] },
  "resolved": { "signatures": { "GetInaccuracy": { "name": "C_CSWeaponBaseGun_GetInaccuracy", "module": "client.dll", "match": "alias" } } },
  "counts":   { "requested": 7, "found": 7, "missing": 0 }
}
```

Each kind takes an array of names or an object `{ "Name": true }`; both spellings are accepted so a request can be written by hand or built from a map. The same request works on the query string — `GET /api/query?signatures=CreateMove,GetInaccuracy&offsets=dwEntityList&schemas=C_CSPlayerPawn.m_iHealth&convars=sv_cheats&detail=1` — with comma-separated names per kind (repeat a parameter to append).

### What each kind returns

| Kind | `detail: false` | `detail: true` |
|---|---|---|
| `signatures` | the pattern string | `{ name, pattern, module, rva, prototype, resolve }` |
| `offsets` | `"0x…"` | `{ name, module, value, value_dec, kind }` |
| `schemas` — `Class.field` | `"0x…"` | `{ class, field, module, offset, offset_dec, type, declared_in }` |
| `schemas` — `Class` | `{ field: "0x…", … }` | adds the class's size, parent and field types |
| `convars` | `{ kind, value, type, flags, flag_names, description, name }` | the same |

### Name matching

Names are tried in tiers — exact, case-insensitive, then the aliases described below — and the first tier with a hit wins. A hit that is not the exact published name is reported under `resolved` with the canonical `name`, its `module`, and `match` (`alias`, `case-insensitive`, `display-name`, `suffix`, …). If a name matches several entries, the first is returned (client.dll preferred) and the candidates are listed under `ambiguous`.

- **Signatures** match the published name, any folded alias, the display name (`CCSGOInput_CreateMove` → `CreateMove`), the `Class::Method` spelling, and as a last resort the method name alone (`SetVoiceData` → `HudVoiceStatus_SetVoiceData`).
- **Offsets** match with or without the `dw` prefix (`EntityList` = `dwEntityList`) and by their `offsets.hpp` identifier.
- **Schemas** — `Class.field` climbs the parent chain, so `C_CSPlayerPawn.m_iHealth` resolves to the field declared on `C_BaseEntity` (`declared_in` says where). `Class` alone returns every field of that class.
- Prefix any name with `module/` to pin a module: `client.dll/CreateMove`, `client/dwEntityList`, `server.dll/CCSPlayerPawn.m_iHealth`. `client` and `client.dll` both work.

Missing names are **not** errors: the response is still HTTP 200 with the name listed under `missing`.

### Errors and limits

| Status | When |
|---|---|
| 400 `{ "success": false, "error": "…" }` | Malformed JSON, wrong types, nothing to query |
| 404 | Unknown session |
| 413 | Body larger than 64 KB |

At most 1,000 names per request and 256 characters per name. Unknown keys in the body are ignored and listed under `warnings`.

## Single-item endpoints

| Endpoint | Returns |
|---|---|
| `GET /api/pattern/<name>` (alias `/api/signature/<name>`) | One signature as it appears in `patterns.json`. The name resolves exactly like the batch query (aliases, display names, `module/Name`); a non-exact hit adds `resolved: { requested, match }` |
| `GET /api/offset/<name>` | One global across modules, with `value_dec` |

## Dataset endpoints

| Endpoint | Filters | Returns |
|---|---|---|
| `GET /api/summary` | | Build, dump time and the size of every dataset |
| `GET /api/health` | | Liveness and session count |
| `GET /api/sessions` | | Available sessions |
| `GET /api/bundle` | | Everything in one JSON: manifest, signatures, vtables, protobufs, schema modules, features |
| `GET /api/patterns` (alias `/api/signatures`) | `module`, `name` (substring), `raw=1` | Signatures. `raw=1` returns the unfolded list with one entry per database name instead of folded aliases |
| `GET /api/offsets` | `module`, `detail=1`, `kind=global\|signature\|interface` | `{module: {name: "0x…"}}`; with `detail=1`, the tagged rows of `offsets_all.json` |
| `GET /api/schema` | `module=client_dll` | The per-module schema header as text |
| `GET /api/schemas` | `module=client_dll`, `class` (substring) | Structured classes and enums; without `module`, a per-module index |
| `GET /api/vtables` | `module` | Interfaces with RTTI class and every vtable slot |
| `GET /api/protobufs` | `module`, `message` | Protobuf message layouts |
| `GET /api/buttons` | | The kbutton table |
| `GET /api/convars` | `q` (substring), `kind=convars\|commands` | ConVars and ConCommands |
| `GET /api/netmessages` | `group`, `q` | Net-message id ↔ name ↔ protobuf, by group |
| `GET /api/weapons` | `q` | Per-weapon `CCSWeaponBaseVData` values |
| `GET /api/gameevents` | `q` | Game events with typed fields |
| `GET /api/entities` | `q` | The entity snapshot and per-class counts |
| `GET /api/status` | | Dump status: build + game client version, self-checks with details, unresolved and auto-healed signatures, known issues — [Status and Self-Healing](Status-and-Self-Healing.md) |
| `GET /api/engine` | | Engine struct layouts, with the header link for each |
| `GET /api/downloads` | | Exportable artifacts: label, href, format, size |
| `GET /api/export/patterns.txt` | | Every pattern as an aligned text file |
| `GET /raw/<path>` | | Any generated file verbatim: `/raw/cs2.hpp`, `/raw/offsets.json`, `/raw/patterns/patterns.json`, `/raw/schemas/client_dll.hpp`, … `GET /api` lists them under `raw_files` |

## Examples

**Python**

```python
import requests

r = requests.post("https://cs2-sdk.com/api/query", json={
    "signatures": ["CreateMove", "GetInaccuracy"],
    "offsets": ["dwEntityList", "dwLocalPlayerPawn"],
    "schemas": ["C_CSPlayerPawn.m_iHealth", "CCSPlayerController.m_hPlayerPawn"],
}).json()

assert r["success"]
print(r["build"], r["found"]["offsets"]["dwEntityList"])
for kind, names in r["missing"].items():
    for n in names:
        print("missing", kind, n)
```

**JavaScript**

```js
const r = await fetch("https://cs2-sdk.com/api/query", {
  method: "POST",
  headers: { "content-type": "application/json" },
  body: JSON.stringify({ offsets: ["dwEntityList"], schemas: ["C_BaseEntity.m_iHealth"], detail: true }),
}).then((x) => x.json());
console.log(r.found.offsets.dwEntityList.value_dec);
```

**PowerShell**

```powershell
$body = @{ signatures = @("CreateMove"); offsets = @("dwEntityList") } | ConvertTo-Json
Invoke-RestMethod -Method Post -Uri https://cs2-sdk.com/api/query -ContentType application/json -Body $body
```

**Build step** — pull the whole SDK instead of individual values:

```bash
curl -fsSL -o vendor/cs2/cs2.hpp https://cs2-sdk.com/raw/cs2.hpp
```

## Conventions

- Hex strings everywhere; `/api/offset/<name>` and `detail=1` add decimal twins (`value_dec`, `offset_dec`).
- The dump's build number is in every batch response (`build`) and in `/api/summary`; compare it with the game before trusting cached values.
- The server re-reads the files when a new dump is deployed. There is nothing to invalidate on your side.
