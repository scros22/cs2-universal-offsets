# Output Layout

After a successful run you'll have:

```text
dumps/
├── 21-04-26-CS2-SDK/                    # one folder per dated session
│   ├── manifest.json                    # session metadata
│   ├── logs/
│   │   └── cs2-sdk.log                  # full file log (always written)
│   ├── offsets/
│   │   ├── buttons.{cs,hpp,json,rs,zig}        # input/button hash table
│   │   ├── interfaces.{cs,hpp,json,rs,zig}     # raw interface RVAs
│   │   ├── offsets.{cs,hpp,json,rs,zig}        # raw module-level offsets
│   │   ├── <module>.{cs,hpp,json,rs,zig}       # raw schema constants
│   │   ├── info.json                           # build_number + timestamp
│   │   │
│   │   ├── cs2sdk.hpp                          # SDK: single-include header
│   │   ├── cs2sdk.rs                           # SDK: Rust amalgamation
│   │   ├── netvars.{json,hpp,cs}               # SDK: split netvars
│   │   ├── interfaces_sdk.{hpp,cs}             # SDK: typed accessors
│   │   └── sdk/
│   │       ├── cs2sdk_macros.hpp               # SCHEMA_FIELD macro family
│   │       └── <module>.hpp                    # typed schema classes
│   │
│   └── signatures/
│       ├── signatures.json              # all hits, one per line
│       ├── signatures.hpp               # cs2::signatures::<module>
│       ├── signatures.cs                # CS2Sdk.Signatures
│       ├── signatures.rs                # pub mod cs2_signatures
│       ├── SIGNATURES.md                # human-readable table
│       └── diff.json                    # delta vs previous session
│
└── latest/                              # mirror of the most recent
                                         # successful session
```

## Which file should *I* depend on?

| Use case                                      | File(s)                                      |
| --------------------------------------------- | -------------------------------------------- |
| Internal cheat in C++                         | `offsets/cs2sdk.hpp`                         |
| Internal cheat in C#                          | `offsets/netvars.cs` + `interfaces_sdk.cs`   |
| Anti-cheat / research feed                    | `latest/offsets/*.json` + `signatures.json`  |
| Patch-day comparison                          | `signatures/diff.json`                       |
| Tooling that already targets a2x/cs2-dumper   | `offsets/<module>.{cs,hpp,json,rs,zig}` (unchanged shape) |

## Why both old and new files?

The raw constant emitters (`<module>.hpp` with
`constexpr std::ptrdiff_t m_iHealth = 0x344;`) preserve compatibility
with every existing consumer of the original a2x/cs2-dumper output.
The new `sdk/` and `cs2sdk.hpp` files are *additive* — they layer typed
classes on top of the same data and are intended for cheat developers
who want ergonomic field access.
