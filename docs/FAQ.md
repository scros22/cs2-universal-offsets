# FAQ

**Which build is this for?**
The one in `manifest.json` and `GET /api/summary` — CS2 build 14183 at the time of writing. CS2 has two numbers: the game's build number (14183, what the console shows) and Steam's client build id (2000915). The site and the dump use the game's build number; release notes mention both.

**The game updated. When is the dump refreshed?**
Usually the same day. Signatures are re-verified after every update, and anything that broke is fixed rather than dropped. Watch the #announcements channel of the Discord, the [releases](https://github.com/scros22/cs2-universal-offsets/releases) page, or poll `GET /api/summary` and compare `build_number`.

**Why does the dumper need administrator rights?**
It opens `cs2.exe` for reading through memflow, which needs `PROCESS_VM_READ` on a process it does not own. It never writes to the process.

**Why dump in a match?**
`weapons.json` and `entities.json` are read from the entities that exist at dump time. In the main menu there are no weapons and few entities; in a match with every weapon spawned you get all 44. Everything else (schemas, signatures, offsets, interfaces, protobufs, convars, events) is the same anywhere.

**Is the dumper detectable?**
It is an external, read-only tool: it injects nothing and hooks nothing. What you build with the output is a different question, and entirely your responsibility.

**A name I know is missing. Where is it?**
Probably folded. When several names resolve to the same function, one is published and the rest become aliases; every alias still resolves in the site search, `/api/pattern/<name>` and `/api/query`, and the response says which entry you got. `GET /api/patterns?raw=1` shows the unfolded list. If it really is missing, open an issue with the IDA address and what the function does — see [Contributing](Contributing.md).

**Why is the published name different from the name I know?**
The published name is the most descriptive one for the function that was actually verified. Sixteen widely copied names were removed in v2.1.2 because the function they pointed at does something else (`SendChatMessage` printed to the local HUD; `GetLocalControllerById` returned the local pawn; …). The [Changelog](Changelog.md) lists them.

**Are the `dwXxx` names the same as the a2x dumper's?**
Yes. `offsets.json` uses the same names and semantics for the classic globals, so code written against a2x output works unchanged. The other 173 globals only exist here (`offsets_all.json`, `offsets.hpp`).

**`patterns.json` fails to parse in my language.**
Since v2.1.4 it is strict JSON (`"rva": "0xB64A10"`). Older dumps wrote bare hex literals; if you are pinned to one of those, quote them (`s/: (0x[0-9A-F]+)/: "\1"/`) or use `GET /api/patterns`.

**How do I get a value from another language?**
`GET /api/query`, or the JSON files: `schemas.json`, `offsets_all.json`, `patterns.json`, `protobufs.json`, `vtables.json`, the catalogues. Nothing depends on the C++ headers.

**Can I use this in my project?**
Yes — MIT licence. A link back is appreciated.

**Where do I ask?**
GitHub issues for wrong or missing data; the Discord for everything else.
