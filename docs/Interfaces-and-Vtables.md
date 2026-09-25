# Interfaces and Vtables

Every Source 2 module exports `CreateInterface`, and every interface registered through it (`Source2Client002`, `EngineTraceClient001`, `InputSystemVersion001`, …) is a C++ object with a vtable. The dumper enumerates the registrations, resolves each factory to the object it returns, walks that object's primary vtable, and recovers the implementing class name from RTTI. On build 14184 that is 110 interfaces, all of them named.

## `interfaces/interfaces.hpp`

One pure-virtual struct per interface in `ifc::<module>::<Class>`, in vtable-slot order, so you call methods like ordinary virtuals:

```cpp
namespace ifc::inputsystem {
    // CInputSystem (iface: InputSystemVersion001) | 106 methods
    struct CInputSystem {
        virtual void method_0() = 0;
        …
        virtual void SetRelativeMouseMode(bool a0) = 0; // slot 76
        …
        void* pSetRelativeMouseMode() { return (*reinterpret_cast<void***>(this))[76]; }
    };
}
```

Slots with a known meaning are typed and named (34 on this build); the rest are `method_<N>` placeholders that keep the layout correct. Each named slot also gets a `p<Name>()` accessor that returns the slot's function pointer, for hooking.

The instance address is in `offsets.hpp` (kind `interface`, see [Offsets](Offsets.md)):

```cpp
auto base = reinterpret_cast<std::uintptr_t>(GetModuleHandleA("inputsystem.dll"));
auto* is  = reinterpret_cast<ifc::inputsystem::CInputSystem*>(base + offsets::inputsystem::InputSystemVersion001);
is->SetRelativeMouseMode(false);
```

Only the **primary** vtable (`*(void**)instance`) is walked. A class reached solely through a secondary vtable (multiple inheritance) is not covered.

## `interfaces/vtables.json`

```json
{
  "modules": {
    "client.dll": {
      "Source2Client002": {
        "rtti_class": "CSource2Client",
        "vtable_module": "client.dll",
        "vtable_rva": 29745216,
        "methods": [
          { "index": 0, "module": "client.dll", "rva": 11946880, "name": null },
          …
        ]
      }
    }
  }
}
```

RVAs in this file are **decimal integers**. `vtable_module` is the DLL that hosts the vtable bytes, which can differ from the module that registered the interface when the implementation lives in a sibling DLL; each method's `module` says where that slot's code is. `name` is the signature-database name when the slot's target is a function the signature pass resolved (33 slots on this build), otherwise `null`.

## Why a slot index, not bytes

Vtable layouts move far less often than function bodies. A slot index changes only when Valve adds, removes or reorders a virtual on that interface; byte patterns move with every codegen change. For interface methods, prefer the slot from `interfaces.hpp` / `vtables.json`, and keep a [signature](Signatures.md) for the functions that are not virtual.

## API

- `GET /api/vtables` — the whole table; `?module=client.dll` to filter.
- The **Interfaces** tab on cs2-sdk.com lists every interface with its RTTI class and method count.
