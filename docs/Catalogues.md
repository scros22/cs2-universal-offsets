# Catalogues

Besides the SDK proper, a run records several tables that are read from the live game and are useful on their own.

## ConVars and ConCommands — `convars/convars.json`, `convars.hpp`

Every entry of the tier0 `CCvar` registry: 4,110 convars and 1,185 commands on build 14184.

```json
{
  "build_number": 14184,
  "convar_count": 4110,
  "convars": [
    { "name": "CS_WarnFriendlyDamageInterval", "type": "int32", "type_id": 3, "value": "3",
      "flags": "0x4004", "flag_names": ["FCVAR_GAMEDLL", "FCVAR_CHEAT"],
      "description": "Defines how frequently the server notifies clients that a player damaged a friend",
      "address": "0x341E0348690" }
  ],
  "command_count": 1185,
  "commands": [
    { "name": "+bugvoice", "flags": "0x20002", "flag_names": ["FCVAR_DEVELOPMENTONLY", "FCVAR_DONTRECORD"],
      "description": "Start recording bug voice attachment.", "address": "0x34114223480" }
  ]
}
```

`value` is the value at dump time, as a string. `flags` is the raw `FCVAR_*` mask and `flag_names` its decoding. `address` is the object's address in the dumped process (not an RVA; it is a heap object) and is only meaningful for that session. `convars.hpp` is the same list as a commented, column-aligned reference table for reading, not code.

## Game events — `gameevents/gameevents.json`

Every event registered with the game event manager (273), with its numeric id and typed keys:

```json
{
  "build_number": 14184,
  "event_count": 273,
  "events": [
    { "name": "achievement_earned", "id": 68, "local": false,
      "fields": [ { "name": "player", "type": "player_controller", "description": "" },
                  { "name": "achievement", "type": "int16", "description": "" } ] }
  ]
}
```

Field types are the ones the event definition declares: `bool`, `byte`, `int16`, `int32`, `uint64`, `float`, `string`, `player_controller`, `player_pawn`, and `local` for client-only keys. The event-level `local` flag marks events that are never networked.

## Weapons — `weapons/weapons.json`

`CCSWeaponBaseVData` of every weapon entity present at dump time, read through the schema of the same run — 44 weapons when the dump is taken in a match with every weapon spawned:

```json
{ "name": "weapon_ak47", "damage": 36, "armor_ratio": 1.55, "penetration": 2.0, "range": 8192.0, "range_modifier": 0.98,
  "cycle_time": 0.1, "spread": 0.0006, "inaccuracy_stand": 0.00641, "inaccuracy_move": 0.17506,
  "recoil_magnitude": 30.0, "headshot_multiplier": 4.0, "max_speed": 215.0, "num_bullets": 1, "price": 2700,
  "address": "0x433CB013800" }
```

## Entities — `entities/entities.json`

A snapshot of the entity list at dump time: `index`, `classname`, `health`, `max_health`, `team` and `origin` for every entity (177 in the current dump), plus per-classname counts (`by_class`). It exists to show what a live entity list looks like on the current build and to give the walkers something to verify against; it is not a reference table.

## Buttons — `buttons.json`, `buttons.hpp`

The RVAs of the client's `kbutton_t` objects (`attack`, `attack2`, `jump`, `duck`, `forward`, `back`, `left`, `right`, `use`, `reload`, `zoom`, `sprint`, `showscores`, `lookatweapon`, `turnleft`, `turnright`). `buttons.json` stores them as decimal integers, `buttons.hpp` as `button::<name>` hex constants, and `client_dll.hpp` repeats them as the `client::InputButton` enum.

## Feature recipes — `verified_features.json`

How the features on the site's **Features** tab are built, each in two flavours:

- **Internal** — code running inside `cs2.exe`: hooks, direct reads, calls into game functions.
- **External** — a separate process using `ReadProcessMemory` / `WriteProcessMemory`: no hooks, no calls. Features that cannot work that way (the skin and knife changers need game code to rebuild materials and models) say why instead.

| Feature | Category | External |
|---|---|---|
| Entity list | Core | yes — walk the chunked identity list |
| ESP | Visuals | yes |
| FOV changer | Visuals | yes — the controller's `m_iDesiredFOV` |
| Aimbot | Aim | yes — write `dwViewAngles` or move the mouse |
| Skin changer | Skins | no |
| Knife changer | Skins | no |

Every recipe was checked against a working implementation and against the current build — IDA on the binaries, plus read-only checks of the live process (the entity-list walk, bone indices, view angles, sensitivity). Nothing numeric is typed into the recipes: each offset is resolved from this dump's schema (`"source": "schema"`), from the hand-verified [engine structs](Engine-Structs) (`"engine"`), or as a schema field plus a fixed delta (`"schema+manual"`, e.g. the bone array at `m_modelState + 0x80`). The handful of values that exist nowhere else are `"manual"` and carry `verified_build`; `stale: true` means they were verified on an older build. Globals come from `offsets.json`, and every function carries this build's RVA and pattern.

```json
{
  "name": "FOV changer",
  "category": "Visuals",
  "summary": "Change your first-person field of view without touching the scope zoom.",
  "internal": {
    "summary": "Override the camera the game is about to render with: hook OverrideView and write the view setup's FOV.",
    "steps": [ "Hook ClientMode::OverrideView(this, CViewSetup*). Call the original first.", "…" ],
    "fields": [ { "class": "CViewSetup", "field": "m_flFov", "offset": "0x498", "source": "engine", "type": "float", "note": "…" } ],
    "globals": [ { "module": "client.dll", "name": "dwLocalPlayerPawn", "value": "0x255E658" } ],
    "functions": [ { "name": "OverrideView", "module": "client.dll", "role": "hook", "rva": "0xD02650", "pattern": "…" } ],
    "convars": [ { "name": "zoom_sensitivity_ratio", "use": "…" } ],
    "notes": [ "…" ]
  },
  "external": { "…": "same shape" },
  "external_unavailable": null
}
```

The dumper's `verified_fields`, `verified_hooks` and `verified_manual` self-checks fail (or warn, for manual values) when any reference stops resolving — see [Status and Self-Healing](Status-and-Self-Healing). On the site, **Copy as C++** turns a recipe into a header of `constexpr` offsets, globals and RVAs for the current build.

## API

`GET /api/convars?q=&kind=convars|commands`, `GET /api/gameevents?q=`, `GET /api/weapons?q=`, `GET /api/entities?q=`, `GET /api/buttons`, and `features` inside `GET /api/bundle`. `POST /api/query` accepts `"convars": ["sv_cheats"]`.
