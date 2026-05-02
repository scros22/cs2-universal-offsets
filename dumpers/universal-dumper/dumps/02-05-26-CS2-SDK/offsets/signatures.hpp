// Generated using https://github.com/a2x/cs2-dumper
// 2026-05-02 10:47:02.045261300 UTC

#pragma once

#include <cstddef>
#include <cstdint>

namespace cs2_dumper {
    namespace signatures {
        // Module: client.dll
        namespace client_dll {
            constexpr std::ptrdiff_t CreateSOSubclassEconItem = 0xFF7770;
            constexpr std::ptrdiff_t EquipItemInLoadout = 0x7C2150;
            constexpr std::ptrdiff_t GetItemInLoadout = 0x7C3D70;
            constexpr std::ptrdiff_t RegenerateWeaponSkin = 0x78C2A0;
            constexpr std::ptrdiff_t SetMeshGroupMask = 0xA2DB50;
            constexpr std::ptrdiff_t SetModel = 0x8DB1C0;
        }
        // Module: materialsystem2.dll
        namespace materialsystem2_dll {
            constexpr std::ptrdiff_t FindParameter = 0x11E30;
            constexpr std::ptrdiff_t PrepareSceneMaterial = 0x11BE0;
            constexpr std::ptrdiff_t UpdateParameter = 0x12370;
        }
    }
}
