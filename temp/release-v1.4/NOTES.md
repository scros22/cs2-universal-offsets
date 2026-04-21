## CS2 Universal Offsets — v1.4

**RTTI vtable class-name recovery.** Every C++ interface vtable now carries its real demangled class name, recovered from the MSVC `RTTICompleteObjectLocator` chain. No PDBs, no signatures, no heuristics — just clean reflection of what the compiler already wrote into the binary.

### Coverage on live build 14152

- **129 / 129** interface vtables resolved
- **7,176** virtual method slots indexed
- **220 / 267** signatures resolved (each with a 24-byte paste-ready `bytes` field)

### Before vs. after

Before (v1.3.1):
```cpp
namespace cs2::vtables::client_dll::Source2Client002 {
    inline constexpr std::ptrdiff_t method_25 = 25; // client.dll + 0xADEC10
}
```

After (v1.4):
```cpp
// CSource2Client (iface: Source2Client002) | vtable @ client.dll+0x1A9AA18 (231 methods)
namespace CSource2Client {
    inline constexpr std::ptrdiff_t method_25 = 25; // client.dll + 0xADEC10
}
```

Examples now named correctly out-of-the-box: `CSource2Client`, `CGameEventSystem`, `CGameResourceService`, `CEngineServiceMgr`, `CPrediction`, `CCsgoClientUI`, `CClientToolsInfo`, `CEmptyWorldService`, `CCSGameConfiguration`, `CBenchmarkService`, `CBugService`, `CClientServerSharedHandleSystem` — and many more.

### Running

1. Launch CS2 and reach the main menu.
2. Run `cs2-sdk.exe` as Administrator.
3. Output appears under `dumps/<DD-MM-YY>-CS2-SDK/` and is mirrored to `dumps/latest/`.

### Assets

- **cs2-sdk-v1.4-windows-x64.zip** — exe + sample `vtables.hpp` + sample `signatures.json`
- **cs2-sdk.exe** — standalone binary (~2.5 MB, no runtime deps)

Source: https://github.com/scros22/cs2-universal-offsets
