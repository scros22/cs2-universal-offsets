#pragma once

// ---------------------------------------------------------------
// Signature patterns for hooks.
// Kept separate so they can be reviewed / patched in one place.
// ---------------------------------------------------------------

namespace Signatures
{
    // ----------------------------------------------------------------
    // client.dll
    // ----------------------------------------------------------------

    // CCSGOInput::CreateMove
    constexpr const char* CreateMove =
        "48 8B C4 4C 89 40 18 48 89 48 08 55 53 41 54 41 55";

    // Third-person camera reset branch
    constexpr const char* ThirdPersonReset =
        "48 8B 40 08 44 38 20 75 10 44 88 67 01";

    // RegenerateWeaponSkin — sub_18078C050 (RVA 0x78C050, current build,
    // IDA-verified 2026-04-25). Call directly with (weapon, false) AFTER
    // writing m_nFallbackPaintKit/Seed/Wear/StatTrak and m_iItemIDHigh.
    // OLD sig (48 83 EC ? E8 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? 48 8B 10) is
    // DEAD on current build — it was a wrapper that no longer exists.
    constexpr const char* RegenWeaponSkins =
        "40 55 53 41 57 48 8D AC 24 00 FE FF FF 48 81 EC";

    // SetMeshGroupMask — CSkeletonInstance method for fixing weapon mesh
    // rendering (sub_180A2C3F0, RVA 0xA2C3F0, IDA-verified 2026-04-25,
    // drift +0x60 from build 14154). Call on entity->m_pGameSceneNode.
    // Called on skeleton instance (entity + m_pGameSceneNode)
    // Param: uint64 mask — 2 = Legacy, 1 = !Legacy
    constexpr const char* SetMeshGroupMask =
        "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8D 99";

    // OverrideView (CCSGOViewAdviceService) — camera FOV/angles
    // NOTE: This sig is DEAD on build 14154 (returns 0 hits). Kept for
    // archaeology. Third-person no longer relies on it — see
    // ThirdPersonOnHandler / ThirdPersonOffHandler below.
    constexpr const char* OverrideView =
        "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 48 8B FA E8";

    // ConCommand handler for `thirdperson` (sub_180AC8C30 on current build,
    // was sub_180AC8BD0 on 14154 — drift +0x60).
    // Pattern keys on the `mov [r8+229h], 1` flag write + the unique
    // `mov dword [r8+6A8h], 0` transition reset that follows. Verified
    // unique across client.dll (IDA 2026-04-25).
    constexpr const char* ThirdPersonOnHandler =
        "48 83 EC 38 48 8B 0D ? ? ? ? 48 8D 54 24 ? 48 8B 01 FF 90 08 03 00 00 "
        "83 7C 24 ? 00 0F 85 ? ? ? ? 4C 8B 05 ? ? ? ? 41 8B 80 50 0B 00 00";

    // ConCommand handler for `firstperson` (sub_180AC8B50 on current build,
    // was sub_180AC8AF0 on 14154 — drift +0x60).
    // Sister of the above; keys on `mov [rax+229h], 0` + transition
    // reset. Verified unique across client.dll (IDA 2026-04-25).
    constexpr const char* ThirdPersonOffHandler =
        "48 83 EC 28 48 8B 0D ? ? ? ? 48 8D 54 24 ? 48 8B 01 FF 90 08 03 00 00 "
        "83 7C 24 ? 00 75 ? 48 8B 05 ? ? ? ? C6 80 29 02 00 00 00 "
        "C7 80 A8 06 00 00 00";

    // SetWorldFov — E8-CALL site used to derive the GetWorldFov hook
    // target. NOTE: DEAD on build 14154 (0 hits, IDA-verified 2026-04-25).
    // The world_effects FOV path falls back to writing
    // m_iDesiredFOV_OnController + m_iFOV/m_iFOVStart on the camera
    // services every tick — that path is correct against dumper-latest
    // and is what's actually changing FOV in-game today. Re-derive a
    // fresh sig if/when we want the cleaner hook back.
    constexpr const char* SetWorldFov =
        "E8 ? ? ? ? F3 0F 11 45 ? 48 8B 5C 24";

    // GetWorldFov resolver — sub_18080BE50 (RVA 0x80BE50, IDA-verified
    // 2026-04-25). This is the function the renderer calls to get the
    // final view FOV. It internally:
    //   1) Honours the `fov_cs_debug` cheat ConVar.
    //   2) Calls the camera vfunc[33] to get the base world FOV.
    //   3) Applies weapon zoom / desired-FOV math.
    //   4) Returns the final float.
    // Hooking here lets us override the FOV cleanly without writing to
    // m_iFOV every tick. Matches the unique prologue + tail-call jmp.
    constexpr const char* GetWorldFovResolver =
        "40 53 48 83 EC 50 48 8B D9 E8 ? ? ? ? 48 85 C0 74 ? "
        "48 8B C8 48 83 C4 50 5B E9";

    // DrawSkyboxArray — scenesystem.dll, skybox render function
    constexpr const char* DrawSkyboxArray =
        "45 85 C9 0F 8E ? ? ? ? 4C 8B DC 55";

    // DisableViewClustering / PVS singleton accessor — engine2.dll
    //   lea rcx, [g_visMgr]   ; load singleton ptr-to-ptr
    //   xor edx, edx          ; arg = 0 (disable)
    //   call qword ptr [rax+30h] ; vtable[6]
    // Inside CRenderingWorldSession::OnLoopActivate; gated behind a
    // `-disable_pvs` style cmdline parm in vanilla. We call vtable[6]
    // ourselves to walk the visibility tree and stamp every leaf
    // visible. After this, the engine no longer culls geometry/entities
    // by PVS leaf — chams render at any distance through any wall.
    constexpr const char* DisablePvsAccessor =
        "48 8D 0D ? ? ? ? 33 D2 FF 50";

    // CSceneAnimatableObject::GeneratePrimitives — scenesystem.dll
    //   Per-renderable mesh-submit virtual. Fired once per scene-object
    //   per draw-pass. Hooking here gives us:
    //     - the actual scene object (CSceneAnimatableObject*) being drawn
    //     - the materials/meshes it is about to submit
    //     - which lets us swap material on a per-entity basis
    //       (proper friend/enemy color separation, unlike a D3D11 hook
    //        which only sees raw vertex/index buffers).
    //   Verified live on build 14155 (RVA 0x73520, single match).
    //   Source: kauht GeneratePrimitives reference + universal-dumper
    //   v1.20.7 (see /memories/repo/cs2_research_index.md entry "u").
    constexpr const char* CSceneAnimatableObject_GeneratePrimitives =
        "48 8B C4 48 89 58 08 48 89 50 10 55 56 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ?";

    // RenderDecals — client.dll
    //   Per-view decal-render dispatch. Returning nullptr from the
    //   detour skips ALL decal submission for that view. Net effect:
    //   no blood splatters, no bullet impacts, no scorch marks, no
    //   sprays — anywhere in the world. Verified single match on
    //   build 14155 (sub_1810EA0E0). Args: (render_ctx, render_view,
    //   pass_flag_A, pass_flag_B). Returns _BYTE* (vanilla returns the
    //   render-list ptr; nullptr = "I drew nothing here, move on").
    constexpr const char* RenderDecals =
        "44 88 4C 24 ? 55 53";

    // KillFeedbackEmitter — client.dll
    //   Emits one of four "Player.Death*.AttackerFeedback" sound
    //   events to the local attacker on a confirmed kill:
    //     Player.DeathHeadShotArmor.AttackerFeedback   (HS + helmet)
    //     Player.DeathHeadShot.AttackerFeedback        (HS, no helmet)
    //     Player.DeathBodyArmor.AttackerFeedback       (body + armor)
    //     Player.DeathBody.AttackerFeedback            (body, no armor)
    //   This IS the iconic CS2 "headshot ding" call site — and the
    //   thud body-kill ack too. Detouring it lets us suppress every
    //   Valve kill ack and play our own custom sound on confirmed
    //   kills (most reliable kill-detection point in the game — fires
    //   from the engine's damage flow, not from our aimbot lock state).
    //   Verified single match on build 14155 (sub_180849CE0). 32-byte
    //   sig pulled from function prologue — no relative branches.
    constexpr const char* KillFeedbackEmitter =
        "48 89 5C 24 08 48 89 74 24 18 48 89 7C 24 20 55 41 56 41 57 48 8B EC 48 81 EC 80 00 00 00 44 8B";

    // DrawSmokeVertex — smoke particle rendering
    constexpr const char* DrawSmokeVertex =
        "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 48 8B 9C 24 ? ? ? ? 4D 8B F8";

    // CalcViewmodel — viewmodel animation/position
    constexpr const char* CalcViewmodel =
        "40 55 53 56 41 56 41 57 48 8B EC";

    // NoSpread1 — spread calculation entry
    constexpr const char* NoSpread1 =
        "48 89 5C 24 08 57 48 81 EC F0 00";

    // CalcSpread — full spread computation
    constexpr const char* CalcSpread =
        "48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 4C 63 EA";

    // ----------------------------------------------------------------
    // Skin / Knife / Glove changer (from UC thread, March 2026)
    // ----------------------------------------------------------------

    // CBaseEntity::SetBodyGroup — required for glove application
    constexpr const char* SetBodyGroup =
        "85 D2 0F 88 5C";

    // CCSPlayerInventory::GetItemInLoadout (sub_1807C3CB0, drift -0x40
    // from build 14154, IDA-verified 2026-04-25)
    constexpr const char* GetItemInLoadout =
        "40 55 48 83 EC ? 49 63 E8";

    // CCSInventoryManager::EquipItemInLoadout (sub_1807C2090, drift -0x40
    // from build 14154, IDA-verified 2026-04-25)
    constexpr const char* EquipItemInLoadout =
        "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 89 54 24 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 0F B7 FA";

    // CEconItemView::GetPaintKitIndex — resolves paint kit from econ item
    constexpr const char* GetPaintKitIndex =
        "48 89 5C 24 ? 57 48 83 EC ? 8B 15 ? ? ? ? 48 8B F9 65 48 8B 04 25";

    // ----------------------------------------------------------------
    // scenesystem.dll
    // ----------------------------------------------------------------

    // DrawObject (legacy)
    constexpr const char* DrawObject =
        "48 8B C4 53 57 41 54 48 81 EC D0 00 00 00 49 63 F9 49";

    // GeneratePrimitives — CSceneAnimatableObject vtable[4], main chams hook point
    constexpr const char* GeneratePrimitives =
        "48 8B C4 48 89 58 08 48 89 50 10 55 56 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ?";

    // ----------------------------------------------------------------
    // materialsystem2.dll
    // CATALOG-ONLY. Chams uses a D3D11 DrawIndexedInstanced hook (see
    // render/hooks.h + features/chams.h) — these sigs are not invoked
    // by any active feature. Verified 2026-04-25 (IDA materialsystem2):
    //   * CreateMaterial   — DEAD on build 14154 (0 hits)
    //   * FindParameter    — unique @ ms2!0x180011E30
    //   * UpdateParameter  — unique @ ms2!0x180012370
    // Kept for archaeology / future material-pipeline experiments.
    // ----------------------------------------------------------------
    constexpr const char* CreateMaterial =
        "48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 81 EC ? ? ? ? 48 8B 05";

    constexpr const char* FindParameter =
        "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B 59 20 48";

    constexpr const char* UpdateParameter =
        "48 89 7C 24 ? 41 56 48 83 EC ? 8B 81";

    // ----------------------------------------------------------------
    // tier0.dll — CATALOG-ONLY (no active call site).
    // ----------------------------------------------------------------
    constexpr const char* LoadKV3 =
        "48 8D 0D ? ? ? ? FF 15 ? ? ? ? 49 8B 06";
}
