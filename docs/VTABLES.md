# VTables

The dumper walks every resolved interface's virtual function table and
emits per-slot indices to `offsets/vtables.{json,hpp,cs}`. Internals
use these to hook by index without hand-RE-ing every call site.

## What ends up in the output

For each entry in [interfaces.json](LAYOUT.md), we follow the instance
pointer and read the first qword to find its `vftable`. From there we
walk the vftable as a contiguous array of function pointers, stopping
at the first slot that doesn't fall inside any loaded module's image
range.

```cpp
namespace cs2::vtables::client_dll {
    // vtable @ client.dll+0x1A2B3C0 (218 methods)
    namespace Source2Client002 {
        inline constexpr std::ptrdiff_t update_global_vars  =  11; // client.dll + 0xADEAC0
        inline constexpr std::ptrdiff_t method_12           =  12; // client.dll + 0xADEC10
        // ...
    }
}
```

## Method-name recovery

We don't have PDBs. Most slots are emitted as `method_<N>` and you use
the index directly. As a bonus pass, every slot's RVA is
cross-referenced against the resolved signature database — if a hit's
RVA matches a method RVA, the signature's name is used instead of
`method_<N>`. That's how slots like `update_global_vars` get a real
name without any extra work on the consumer side.

## Class-name recovery (RTTI)

Every polymorphic class compiled by MSVC stores a back-pointer to a
`RTTICompleteObjectLocator` at slot `-1` of its vtable. Following that
chain (`COL → TypeDescriptor → mangled name`) and demangling the result
gives us the original C++ class name for free — no signatures, no
heuristics, no PDBs. CS2 is built with `/GR` (RTTI on), so on the live
build every single interface vtable resolves cleanly.

The recovered class becomes the C++ namespace name in `vtables.hpp`,
with the original interface version preserved in a banner comment:

```cpp
// CSource2Client (iface: Source2Client002) | vtable @ client.dll+0x1A9AA18 (231 methods)
namespace CSource2Client {
    inline constexpr std::ptrdiff_t method_0 = 0; // client.dll + 0x...
    // ...
}
```

Falls back to the interface version if a vtable doesn't carry valid
RTTI (rare — usually means a compiler thunk or a vftable that failed
our signature/self-RVA sanity checks).

## Hooking by index

```cpp
#include "cs2sdk/cs2sdk.hpp"

using CreateMove_t = bool(__thiscall*)(void* self, int slot, float ft, bool active);

void hook_create_move(void* iface) {
    auto** vt = *reinterpret_cast<void***>(iface);
    auto*  fn = vt[cs2::vtables::client_dll::Source2Client002::method_25];
    install_hook(fn, &your_create_move_hook);
}
```

The `vtable_module` field in `vtables.json` tells you which DLL hosts
the vftable bytes (sometimes different from the interface's DLL when
the implementation lives in a sibling module).

## Why this is safer than raw byte patterns

Vtable layouts are *much* more stable across patches than function-body
bytes. A slot index changes only when Valve adds, removes, or reorders
a virtual method on the interface — far rarer than the dozens of
codegen tweaks that move byte patterns every patch. Where a signature
gives you "the function that does X today", a vtable index gives you
"the slot that has done X for years".

## Where to look

| Concern                | File                                  |
| ---------------------- | ------------------------------------- |
| Interface enumeration  | `src/analysis/interfaces.rs`          |
| Vtable walker          | `src/analysis/vtables.rs`             |
| Emitter + name oracle  | `src/output/vtables.rs`               |
