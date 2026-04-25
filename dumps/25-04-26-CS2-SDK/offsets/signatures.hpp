// Generated using https://github.com/a2x/cs2-dumper
// 2026-04-25 13:18:49.344222200 UTC

#pragma once

#include <cstddef>
#include <cstdint>

namespace cs2_dumper {
    namespace signatures {
        // Module: client.dll
        namespace client_dll {
            constexpr std::ptrdiff_t CreateSOSubclassEconItem = 0xFF4EB0;
            constexpr std::ptrdiff_t EquipItemInLoadout = 0x7C2090;
            constexpr std::ptrdiff_t GetItemInLoadout = 0x7C3CB0;
            constexpr std::ptrdiff_t RegenerateWeaponSkin = 0x78C050;
            constexpr std::ptrdiff_t SetMeshGroupMask = 0xA2C3F0;
            constexpr std::ptrdiff_t SetModel = 0x8D9B40;
        }
        // Module: materialsystem2.dll
        namespace materialsystem2_dll {
            constexpr std::ptrdiff_t FindParameter = 0x11E30;
            constexpr std::ptrdiff_t PrepareSceneMaterial = 0x11BE0;
            constexpr std::ptrdiff_t UpdateParameter = 0x12370;
        }
    }
}
