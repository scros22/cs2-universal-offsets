# Discord Bot

The cs2-sdk Discord bot answers from the same API the site uses, so every value it shows is the one on cs2-sdk.com. Its source is in [`discord-bot/`](https://github.com/scros22/cs2-universal-offsets/tree/main/discord-bot).

## Commands

Every name argument has autocomplete backed by the live index: signatures with their aliases, offsets, classes and fields, convars, weapons, events and structs.

| Command | What you get |
|---|---|
| `/build` | Current CS2 build, dump time and dataset counts |
| `/status` | Self-check results for the current dump, unresolved and auto-healed signatures, known issues |
| `/sig <name>` | Pattern, RVA, module, prototype and aliases — any alias or display name works |
| `/offset <name>` | RVA of a global: a `dwXxx`, a signature global or an interface instance |
| `/schema <class> [field]` | The class layout (parent, size, fields), or one field's offset and type |
| `/convar <name>` | Value, type, decoded flags and description; ConCommands too |
| `/weapon <name>` | Live `CCSWeaponBaseVData` stats (`weapon_ak47` or just `ak47`) |
| `/event <name>` | Game event id and its typed fields |
| `/struct <name>` | An engine struct layout (`CUserCmd`, `CCSGOInput`, `CBaseUserCmdPB`, …) |
| `/search <text>` | Signatures, offsets, classes, convars and structs matching the text, at once |
| `/query …` | A batch lookup, identical to `POST /api/query`, with the `curl` command that reproduces it |
| `/api` | The API cheat sheet |
| `/refresh` | Re-read the dump now (requires **Manage Server**) |

## Channels

- **#status** — one embed, edited in place every 5 minutes: build, dump time, latest dumper release, and the size of every dataset. Pin it.
- **#announcements** — one post per new dump (whenever the build number or dump time changes), with the counts and a link to the release notes.

Both channels are found by name (or id) from the config. The bot needs *View Channel*, *Send Messages* and *Embed Links* in them, plus *Manage Messages* in #status if it should pin its own message.

## Self-hosting

```bash
cd discord-bot
npm install
cp config.example.json config.json     # token, application id, API base
npm run selftest                        # renders every embed against the API and checks Discord's limits
pm2 start ecosystem.config.cjs && pm2 save
```

`config.json`:

```json
{
  "token": "…",
  "clientId": "…",
  "apiBase": "https://cs2-sdk.com/api",
  "session": "latest",
  "statusIntervalMinutes": 5,
  "channels": { "status": "status", "announcements": "announcements" },
  "githubRepo": "scros22/cs2-universal-offsets",
  "brand": { "name": "CS2 SDK", "url": "https://cs2-sdk.com", "iconUrl": "" }
}
```

Never commit `config.json`. Slash commands are registered per guild when the bot starts, so they appear immediately. Runtime state (the status message id and the last announced dump) is kept in `data/state.json`. Node 20 or newer and discord.js 14 are required; the bot needs only the `Guilds` intent.
