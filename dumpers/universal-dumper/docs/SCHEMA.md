# Source 2 Schema Walking

Source 2 carries enough runtime metadata to fully reconstruct every
networked class layout, every enum, every netvar, and every metadata
attribute *without* PDBs. This document explains what the dumper reads
and why.

## 1. The Schema System at 30,000 ft

When the engine starts it builds a `CSchemaSystem` singleton. Each
loaded module that uses the schema system registers a
`CSchemaSystemTypeScope` — one scope per DLL — and inside each scope
lives a `CUtlTSHash` of every `CSchemaClassBinding` and
`CSchemaEnumBinding` declared by that module.

For every networked class the engine populates:

- **Class name** — the C++ class name as declared with `SCHEMA_CLASS()`.
- **Parent name** — single inheritance, optional.
- **Field list** — name, type-name string, byte offset within the
  instance.
- **Metadata list** — variable-length list of attributes attached at
  schema-compile time. The two we care about most are:
  - `MNetworkVarNames(name, type)` — declares a networked field.
  - `MNetworkChangeCallback(name)` — declares a change-notify callback.

Enums work the same way: name + alignment + member list with
`(name, value)` pairs.

## 2. How the Walker Works

The walker lives in `src/source2/` and has three layers:

1. **Pelite-backed PE parsing** locates the
   `g_pSchemaSystem`/`SchemaSystemTypeScope` symbols by signature (see
   `src/analysis/offsets.rs` for the patterns).
2. **memflow reads** chase the `CUtlTSHash` chain in the live process
   to enumerate every class binding and every enum binding per scope.
3. **Type translation** in `src/analysis/schemas.rs` flattens the
   bindings into the `Class { name, parent_name, fields, metadata }`
   and `Enum { name, alignment, size, members }` shapes the emitters
   consume.

## 3. What the Emitters Do With It

Two parallel emitters consume the schema:

| Emitter                                | Output                                         | Audience                          |
| -------------------------------------- | ---------------------------------------------- | --------------------------------- |
| `output::schemas` (original)           | `<module>.{cs,hpp,json,rs,zig}` constants only | offset-only consumers / dumpers   |
| `output::sdk_classes` (added in v1.2)  | `sdk/<module>.hpp` typed classes               | internal cheat developers         |

The original emitter writes:

```cpp
namespace cs2_dumper::schemas::client_dll::C_CSPlayerPawn {
    constexpr std::ptrdiff_t m_iHealth = 0x344;
}
```

The new SDK emitter writes:

```cpp
namespace cs2::sdk::client_dll {
    class C_CSPlayerPawn : public C_BasePlayerPawn {
    public:
        SCHEMA_FIELD(std::int32_t, m_iHealth, 0x344) // int32
    };
}
```

`SCHEMA_FIELD` is defined in `sdk/cs2sdk_macros.hpp` and expands to a
pair of inline accessors that produce a typed lvalue at
`reinterpret_cast<std::byte*>(this) + offset`. Zero overhead in
release builds.

## 4. Netvar Splitting

Networked fields are a tiny strict subset of all schema fields. The
`output::netvars` emitter filters the schema for classes that carry one
or more `MNetworkVarNames` metadata entries and writes a separate
`netvars.{json,hpp,cs}` file. Cheats that only need network state can
include this file alone and skip the (much larger) full schema dump.

## 5. Metadata We Currently Recognise

```rust
pub enum ClassMetadata {
    NetworkChangeCallback { name: String },
    NetworkVarNames       { name: String, type_name: String },
    Unknown               { name: String },
}
```

`Unknown` carries the raw attribute name so future versions of the
dumper can decode more attributes without breaking JSON consumers in
the meantime.

## 6. Where to Look in the Source

| Concern                                  | File                                    |
| ---------------------------------------- | --------------------------------------- |
| Walk schema system / scopes / hash table | `src/source2/`                          |
| Translate raw bindings to public structs | `src/analysis/schemas.rs`               |
| Original constants emitter (per-module)  | `src/output/schemas.rs`                 |
| New SDK class emitter (per-module)       | `src/output/sdk_classes.rs`             |
| Netvar split emitter                     | `src/output/netvars.rs`                 |
