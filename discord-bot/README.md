# cs2-sdk Discord bot

Companion bot for **cs2-sdk.com**. It talks to the site's local API
(`http://127.0.0.1:7777/api`) — the same data the website serves — so
everything it shows is exactly what is on the site.

## What it does

- **#status** — one embed, edited in place every few minutes: build, dump
  time, latest dumper release, and the size of every dataset.
- **#announcements** — one post per new dump (build number or dump time
  changed), with counts and a link to the release notes.
- **Slash commands** (autocomplete on every name):

  | command | what you get |
  |---|---|
  | `/build` | current build and dump status |
  | `/sig name` | pattern, RVA, module, prototype, aliases — any alias works |
  | `/offset name` | RVA of a global (dwXxx, signature global, interface) |
  | `/schema class [field]` | class layout, or one field's offset and type |
  | `/convar name` | value, type, flags, description (ConCommands too) |
  | `/weapon name` | live `CCSWeaponBaseVData` stats |
  | `/event name` | game event id + typed fields |
  | `/struct name` | engine struct layout (`CUserCmd`, `CBaseUserCmdPB`, …) |
  | `/search text` | signatures, offsets, classes, convars and structs at once |
  | `/query …` | batch lookup, identical to `POST /api/query`, with the curl to reproduce it |
  | `/api` | API cheat sheet |
  | `/refresh` | (Manage Server) re-read the dump now |

## Running

```bash
cd discord-bot
npm install
npm run selftest            # renders every embed against the live API, checks Discord limits
pm2 start ecosystem.config.cjs && pm2 save
```

`config.json` holds the bot token and client id (never commit it), plus:

```json
{
  "apiBase": "http://127.0.0.1:7777/api",
  "session": "latest",
  "statusIntervalMinutes": 5,
  "channels": { "status": "status", "announcements": "announcements" },
  "githubRepo": "scros22/cs2-universal-offsets",
  "brand": { "name": "CS2 SDK", "url": "https://cs2-sdk.com" }
}
```

Channels are found by name (or by id if you put an id there). State — the
status message id and the last announced dump — is in `data/state.json`.
