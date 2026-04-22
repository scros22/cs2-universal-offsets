// Generated using https://github.com/a2x/cs2-dumper
// 2026-04-21 22:47:42.668580400 UTC

#pragma once

#include <cstddef>
#include <cstdint>

namespace cs2_dumper {
    namespace signatures {
        // Module: client.dll
        namespace client_dll {
            constexpr std::ptrdiff_t CreateSOSubclassEconItem = 0xFF07D0;
            constexpr std::ptrdiff_t EquipItemInLoadout = 0x7C2780;
            constexpr std::ptrdiff_t GetItemInLoadout = 0x7C43A0;
            constexpr std::ptrdiff_t RegenerateWeaponSkin = 0x78C6B0;
            constexpr std::ptrdiff_t SetMeshGroupMask = 0xA27EE0;
            constexpr std::ptrdiff_t SetModel = 0x8D56F0;
        }
        // Module: materialsystem2.dll
        namespace materialsystem2_dll {
            constexpr std::ptrdiff_t FindParameter = 0x11E30;
            constexpr std::ptrdiff_t PrepareSceneMaterial = 0x11BE0;
            constexpr std::ptrdiff_t UpdateParameter = 0x12370;
        }
    }
}
