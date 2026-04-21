# Consuming the Generated SDK

This guide shows how to drop the dumper output into a real internal
cheat in C++, C#, and Rust. We use a tiny worked example throughout —
reading and writing a player's health.

## 1. The File You Want

After a successful run, the most useful single file is:

```text
dumps/<session>/offsets/cs2sdk.hpp
```

It is a *single-include amalgamation*: include it once and you
transitively get the SCHEMA_FIELD macros, every per-module schema
class header, the resolved interface accessors, the netvar offsets,
and the signature constants.

For builds-as-source consumers (e.g. anti-cheat researchers) you can
also point your URL at the `latest/` mirror:

```text
https://raw.githubusercontent.com/scros22/cs2-universal-offsets/main/dumps/latest/offsets/cs2sdk.hpp
```

`latest/` is rewritten on every successful dump so this URL always
serves the newest build.

## 2. C++

### 2.1 Layout

Copy the entire `dumps/<session>/offsets/` folder into your project
and add it to your include path:

```text
my-cheat/
├── src/
└── vendor/cs2sdk/
    ├── cs2sdk.hpp
    ├── interfaces.hpp
    ├── netvars.hpp
    ├── signatures.hpp
    └── sdk/
        ├── cs2sdk_macros.hpp
        ├── client_dll.hpp
        ├── engine2_dll.hpp
        └── ...
```

### 2.2 Pinning the build number

Every generated header defines `CS2_BUILD`. Refuse to load against the
wrong build:

```cpp
#include "cs2sdk/cs2sdk.hpp"

bool sanity_check(std::uint32_t running_build) {
    if (cs2::CS2_BUILD != running_build) {
        // Wrong patch — bail before you crash on a moved offset.
        return false;
    }
    return true;
}
```

### 2.3 Worked example: read + write `m_iHealth`

```cpp
#include "cs2sdk/cs2sdk.hpp"

void heal(void* pawn_ptr) {
    auto* pawn = reinterpret_cast<cs2::sdk::client_dll::C_CSPlayerPawn*>(pawn_ptr);
    if (pawn->m_iHealth() < 100) {
        pawn->m_iHealth() = 100;
    }
}
```

`SCHEMA_FIELD(int32_t, m_iHealth, 0x344)` expands to two inline
accessors that produce a typed lvalue at `this + 0x344`. Zero overhead
in release.

### 2.4 Worked example: resolve an interface

```cpp
#include "cs2sdk/interfaces_sdk.hpp"

void* engine_client(std::uintptr_t engine2_base) {
    return cs2::ifaces::engine2_dll::Source2EngineToClient001(engine2_base);
}
```

`engine2_base` is whatever your loader has on hand —
`GetModuleHandleW(L"engine2.dll")`, your own enumerator, etc.

## 3. C\#

```csharp
using CS2Sdk.NetVars;
using CS2Sdk.Interfaces;

// Pinning the build number
if (NetVars.CS2_BUILD != currentBuild) return;

// Read m_iHealth via the netvar split file
nint healthOff = NetVars.C_CSPlayerPawn.m_iHealth;
int hp = ReadInt32(pawnPtr + healthOff);

// Resolve an interface
nint engineClient = Interfaces.engine2_dll.Source2EngineToClient001(engine2Base);
```

## 4. Rust

```rust
mod cs2sdk;            // dumped cs2sdk.rs
use cs2sdk::signatures::client_dll::*;

fn cl_recv_user_cmds(client_base: usize) -> usize {
    client_base + CL_RecvUserCmds as usize
}
```

(For a full typed Rust SDK that mirrors the C++ class headers, treat
the `signatures.rs` + per-module offsets as the foundation and add a
thin newtype wrapper layer in your own crate.)

## 5. Workflow Tips

- **Diff first.** On patch day, compare `diff.json` against your last
  pin before you ship — it'll tell you which fields shifted.
- **Pin builds.** `static_assert(cs2::CS2_BUILD == ...);` is cheap
  insurance.
- **Only ship what you use.** The amalgamation is convenient for dev;
  for final builds you can include only the `sdk/<module>.hpp` files
  your cheat actually touches.
- **Don't edit the generated files.** They get overwritten on every
  run. If you need a custom type, add it in your own header and have
  the SDK header forward to it.
