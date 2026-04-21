# CS2 Signatures

_This file is regenerated on every successful run of `cs2-sdk`._

**181/221 signatures resolved across 8 module(s).**

## `animationsystem.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `Animation::ShouldUpdateSequences` | `raw` | `0x7FFF65EAF0A0` | `0x14F0A0` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 49 8B 40 48` |

## `client.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `AddNametagEntity` | `raw` | `0x7FFF56CBB480` | `0x78B480` | `40 55 53 56 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B DA` |
| `AddStattrakEntity` | `raw` | `0x7FFF56F76B20` | `0xA46B20` | `48 8B C4 48 89 58 08 48 89 70 10 57 48 83 EC 50 33 F6 8B FA 48 8B D1` |
| `AutowallInit` | `raw` | `0x7FFF56E0C410` | `0x8DC410` | `40 53 48 83 EC ? 48 8B D9 48 81 C1 ? ? ? ? E8 ? ? ? ?` |
| `AutowallTraceData` | `raw` | `0x7FFF56EB8DB0` | `0x988DB0` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B 09` |
| `AutowallTracePos` | `raw` | `0x7FFF56D34ED0` | `0x804ED0` | `40 55 56 41 54 41 55 41 57 48 8B EC` |
| `CAM_ThinkReturn` | `raw` | `0x7FFF5684A3DF` | `0x31A3DF` | `BA 04 00 00 00 FF 15 ? ? ? ? 84 C0 0F 84` |
| `CAttributeStringFill` | `rel32` | `0x7FFF573D7F10` | `0xEA7F10` | `E8 ? ? ? ? 41 83 CF 08` |
| `CAttributeStringInit` | `rel32` | `0x7FFF56B28A50` | `0x5F8A50` | `E8 ? ? ? ? 48 8D 05 ? ? ? ? 48 89 7D ? 48 89 45 ? 49 8D 4F` |
| `CBufferStringInit` | `raw` | `0x7FFF57D0BA70` | `0x17DBA70` | `48 89 5C 24 ? 57 48 83 EC ? 8B 41 ? 48 8D 79` |
| `CCSGOInput::CreateMove` | `raw` | `0x7FFF57187CA0` | `0xC57CA0` | `48 8B C4 4C 89 40 18 48 89 48 08 55 53 41 54 41 55` |
| `CCSInventoryManager::EquipItemInLoadout` | `raw` | `0x7FFF56CF2780` | `0x7C2780` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 89 54 24 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 0F B7 FA` |
| `CCSPlayerController` | `stringref` | `0x7FFF56D12CF0` | `0x7E2CF0` | `"CCSPlayerController"` |
| `CCSPlayerInventory::GetItemInLoadout` | `raw` | `0x7FFF56CF43A0` | `0x7C43A0` | `40 55 48 83 EC ? 49 63 E8` |
| `CCSPlayerPawn` | `stringref` | `0x7FFF570DC0C0` | `0xBAC0C0` | `"CCSPlayerPawn"` |
| `CCSPlayer_BulletServices` | `stringref` | `0x7FFF56D3FD50` | `0x80FD50` | `"CCSPlayer_BulletServices"` |
| `CCSPlayer_CameraServices` | `stringref` | `0x7FFF56D3BE30` | `0x80BE30` | `"CCSPlayer_CameraServices"` |
| `CCSPlayer_ItemServices` | `stringref` | `0x7FFF56D405A0` | `0x8105A0` | `"CCSPlayer_ItemServices"` |
| `CCSPlayer_MovementServices` | `stringref` | `0x7FFF56D68D70` | `0x838D70` | `"CCSPlayer_MovementServices"` |
| `CCSPlayer_WeaponServices` | `stringref` | `0x7FFF56DA4A70` | `0x874A70` | `"CCSPlayer_WeaponServices"` |
| `CEconItemSchema::GetAttributeDefinitionByName` | `raw` | `0x7FFF57575F00` | `0x1045F00` | `48 89 5C 24 10 48 89 6C 24 18 57 41 56 41 57 48 83 EC 60 48 8D 05` |
| `CEconItemView::GetCustomPaintKitIndex` | `raw` | `0x7FFF56DE5820` | `0x8B5820` | `48 89 5C 24 ? 57 48 83 EC ? 8B 15 ? ? ? ? 48 8B F9 65 48 8B 04 25` |
| `CFogController` | `stringref` | `0x7FFF567AEEC0` | `0x27EEC0` | `"CFogController"` |
| `CGameEntitySystem::OnAddEntity` | `raw` | `0x7FFF56E92AC0` | `0x962AC0` | `48 89 74 24 ? 57 48 83 EC ? 41 B9 ? ? ? ? 41 8B C0 41 23 C1 48 8B F2 41 83 F8 ? 48 8B F9 44 0F 45 C8 41 81 F9 ? ? ? ? 73 ? FF 81` |
| `CGameEntitySystem::OnRemoveEntity` | `raw` | `0x7FFF56E93320` | `0x963320` | `48 89 74 24 ? 57 48 83 EC ? 41 B9 ? ? ? ? 41 8B C0 41 23 C1 48 8B F2 41 83 F8 ? 48 8B F9 44 0F 45 C8 41 81 F9 ? ? ? ? 73 ? FF 89` |
| `CPostProcessingVolume` | `stringref` | `0x7FFF567D3C50` | `0x2A3C50` | `"CPostProcessingVolume"` |
| `CSBaseGunFireData_fn` | `raw` | `0x7FFF57A111E0` | `0x14E11E0` | `48 8B C4 55 53 56 57 41 54 41 55 41 56 41 57 48 8D 68 A8 48 81 EC ? ? ? ? 4C 8B 69` |
| `CSGOInput_resolved` | `riprel` | `0x7FFF5858C007` | `0x205C007` | `48 8B 0D ? ? ? ? 8B 10 E8 ? ? ? ? 45 32 FF` |
| `CSkeletonInstance::SetMeshGroupMask` | `raw` | `0x7FFF56F57EE0` | `0xA27EE0` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8D 99` |
| `CTonemapController2` | `stringref` | `0x7FFF56787B80` | `0x257B80` | `"CTonemapController2"` |
| `C_AttributeContainer` | `stringref` | `0x7FFF57142110` | `0xC12110` | `"C_AttributeContainer"` |
| `C_CSWeaponBase` | `stringref` | `0x7FFF56C728E0` | `0x7428E0` | `"C_CSWeaponBase"` |
| `C_EconItemView` | `stringref` | `0x7FFF56C3B950` | `0x70B950` | `"C_EconItemView"` |
| `CacheParticleEffect` | `raw` | `0x7FFF56737AB0` | `0x207AB0` | `4C 8B DC 53 48 81 EC ? ? ? ? F2 0F 10 05` |
| `CalcSpread` | `raw` | `0x7FFF571A80A0` | `0xC780A0` | `48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 4C 63 EA` |
| `CalcViewmodel` | `raw` | `0x7FFF56D7C3E0` | `0x84C3E0` | `40 55 53 56 41 56 41 57 48 8B EC` |
| `CalculateInterpolation` | `rel32` | `0x7FFF579F0F10` | `0x14C0F10` | `E8 ? ? ? ? 8B 45 ? 3B 45 60 75 04 32 D2 EB 09 BA 01 00 00 00 41 0F 4C D5 C0 EA 07 84 D2 0F 85 87` |
| `CalculateWorldSpaceBones` | `raw` | `0x7FFF56F35460` | `0xA05460` | `48 89 4C 24 ? 55 53 56 57 41 54 41 55 41 56 41 57 B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 48 8D 6C 24 ? 48 8B 81` |
| `ClearHUDWeaponIcon` | `rel32` | `0x7FFF573170C0` | `0xDE70C0` | `E8 ? ? ? ? 8B F8 C6 84 24 ? ? ? ? ?` |
| `ClientMode_ptr` | `riprel` | `0x7FFF588663C0` | `0x23363C0` | `48 8D 0D ? ? ? ? 48 69 C0 ? ? ? ? 48 03 C1 C3 CC CC` |
| `ConvarGet` | `raw` | `0x7FFF56DE9B32` | `0x8B9B32` | `8B D0 48 8D 0D ? ? ? ? E8 ? ? ? ? 0F 10 45 ? 83 F0 74` |
| `CreateBaseTypeCache` | `raw` | `0x7FFF57A39F40` | `0x1509F40` | `40 53 48 83 EC ? 4C 8B 49 ? 44 8B D2` |
| `CreateEntityByClassName` | `raw` | `0x7FFF57B2DCE6` | `0x15FDCE6` | `4C 8D 05 ? ? ? ? 4C 8B CF BA 03 00 00 00 FF 15 ? ? ? ? EB ? 0F B7 C8 48` |
| `CreateNewSubtickMoveStep` | `rel32` | `0x7FFF569E1C60` | `0x4B1C60` | `E8 ? ? ? ? 48 8B D0 48 8B CE E8 ? ? ? ? 48 8B C8` |
| `CreateParticleEffect` | `raw` | `0x7FFF56EB1410` | `0x981410` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? F3 0F 10 1D ? ? ? ? 41 8B F8 8B DA 4C 8D 05` |
| `CreateSOSubclassEconItem` | `raw` | `0x7FFF575207D0` | `0xFF07D0` | `48 83 EC 28 B9 48 00 00 00 E8 ? ? ? ? 48 85` |
| `DestroyParticle` | `raw` | `0x7FFF56E70940` | `0x940940` | `83 FA ? 0F 84 ? ? ? ? 41 54` |
| `DispatchEffect` | `raw` | `0x7FFF5688A450` | `0x35A450` | `48 89 5C 24 ? 57 48 83 EC ? 48 8B F9 48 8B DA 48 8D 4C 24` |
| `DrawCrosshair` | `raw` | `0x7FFF56CE1230` | `0x7B1230` | `48 89 5C 24 08 57 48 83 EC 20 48 8B D9 E8 ? ? ? ? 48 85` |
| `DrawOverHead` | `raw` | `0x7FFF56F91080` | `0xA61080` | `40 53 48 83 EC ? 48 8B D9 83 FA ? 75 ? 48 8B 0D ? ? ? ? 48 8D 54 24 ? 48 8B 01 FF 90 ? ? ? ? 8B 10` |
| `DrawScopeOverlay` | `raw` | `0x7FFF56D8A980` | `0x85A980` | `48 8B C4 53 57 48 83 EC ? 48 8B FA` |
| `DrawSmokeVertex` | `raw` | `0x7FFF571A4740` | `0xC74740` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 48 8B 9C 24 ? ? ? ? 4D 8B F8` |
| `FindHudElement` | `raw` | `0x7FFF572EB188` | `0xDBB188` | `48 8D 15 ? ? ? ? 45 33 C0 B9 ? ? ? ? FF 15 ? ? ? ? EB ? 48 8B 15` |
| `FindHudElement_panorama` | `raw` | `0x7FFF572ED160` | `0xDBD160` | `4C 8B DC 53 48 83 EC 50 48 8B 05` |
| `FindSOCache` | `raw` | `0x7FFF57D48120` | `0x1818120` | `48 89 5C 24 08 57 48 83 EC 30 4C 8B 52 08 48 8B D9 8B 0A` |
| `FirstPersonLegs` | `raw` | `0x7FFF576193C0` | `0x10E93C0` | `40 55 53 56 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? F2 0F 10 42` |
| `FlashOverlay` | `raw` | `0x7FFF572D45C0` | `0xDA45C0` | `85 D2 0F 88 ? ? ? ? 48 89 4C 24` |
| `ForceButtonsDown` | `raw` | `0x7FFF56EFA520` | `0x9CA520` | `40 53 57 41 56 48 81 EC ? ? ? ? 48 83 79` |
| `GameEntitySystemPtr` | `riprel` | `0x7FFF589F9710` | `0x24C9710` | `48 8B 1D ? ? ? ? 48 89 1D ? ? ? ?` |
| `GameRules_ptr` | `riprel` | `0x7FFF58853900` | `0x2323900` | `48 8B 1D ? ? ? ? 48 8D 54 24 ? 0F 28 D0 48 8D 4C 24 ?` |
| `GetBBox_ptr` | `riprel` | `0x7FFF58853900` | `0x2323900` | `48 8B 0D ? ? ? ? 48 85 C9 74 ? ? ? ? 48 FF A0 ? ? ? ? 48 8D 05` |
| `GetBaseEntity` | `raw` | `0x7FFF56E91A80` | `0x961A80` | `4C 8D 49 ? 81 FA` |
| `GetBonePositionByName` | `raw` | `0x7FFF56DF2710` | `0x8C2710` | `40 53 48 83 EC ? 48 8B 89 ? ? ? ? 48 8B DA 48 8B 01 FF 50 ? 48 8B C8` |
| `GetChatObject` | `rel32` | `0x7FFF575EC6D0` | `0x10BC6D0` | `E8 ? ? ? ? 48 8B E8 48 85 C0 0F 84 ? ? ? ? 4C 8D 05` |
| `GetClientSystem` | `rel32` | `0x7FFF5755F5D0` | `0x102F5D0` | `E8 ? ? ? ? 48 8B C8 E8 ? ? ? ? 8B D8 85 C0 74 33` |
| `GetControllerCmd` | `raw` | `0x7FFF56DE8130` | `0x8B8130` | `40 53 48 83 EC 20 8B DA E8 ? ? ? ? 4C` |
| `GetEconItemSystem` | `raw` | `0x7FFF568A9710` | `0x379710` | `48 83 EC 28 48 8B 05 ? ? ? ? 48 85 C0 0F 85 ? ? ? ? 48 89 5C 24` |
| `GetEntityHandle` | `raw` | `0x7FFF56E78D50` | `0x948D50` | `48 85 C9 74 32 48 8B 49 10 48 85 C9 74 29 44 8B 41 10 BA` |
| `GetGlowColor` | `raw` | `0x7FFF57038950` | `0xB08950` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B F2 48 8B F9 48 8B 54 24` |
| `GetInstanceS` | `riprel` | `0x7FFF587E2240` | `0x22B2240` | `48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 8B 91 ? ? ? ? B8` |
| `GetInt2_Event` | `raw` | `0x7FFF569DAA20` | `0x4AAA20` | `48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC 20 48 63 FA 41` |
| `GetInventoryManager` | `rel32` | `0x7FFF56CF6A60` | `0x7C6A60` | `E8 ? ? ? ? 48 8B D3 48 8B C8 4C 8B 00 41 FF 90 00 02` |
| `GetLocalControllerById` | `raw` | `0x7FFF56E0B5A0` | `0x8DB5A0` | `48 83 EC 28 83 F9 FF 75 ? 48 8B 0D ? ? ? ? 48 8D 54 24 ? 48 8B 01 FF 90 ? ? ? ? 8B 08 48 63 C1 4C 8D 05` |
| `GetMatrixForView` | `raw` | `0x7FFF56699B40` | `0x169B40` | `40 53 48 83 EC 60 0F 29 74 24 50 0F 57 DB F3 0F 10 ? ? ? ? ? 49 8B D8` |
| `GetPlayerInterp` | `raw` | `0x7FFF56DE3990` | `0x8B3990` | `40 53 48 83 EC ? 48 8B D9 48 8B 0D ? ? ? ? 48 83 C1` |
| `GetRemovedAimPunch_E8` | `rel32` | `0x7FFF56D7A010` | `0x84A010` | `E8 ? ? ? ? 4C 8B C0 48 8D 55 ? 48 8B CB E8 ? ? ? ? 48 8D 0D` |
| `GetRemovedAimpunch` | `raw` | `0x7FFF56642837` | `0x112837` | `F2 0F 10 44 24 ? F2 0F 11 84 24 ? ? ? ? FF 15` |
| `GetSurfaceData` | `rel32` | `0x7FFF56E7D9C0` | `0x94D9C0` | `E8 ? ? ? ? 80 78 18 00` |
| `GetTickBase` | `rel32` | `0x7FFF56DE7F30` | `0x8B7F30` | `E8 ? ? ? ? EB ? 48 8B 05 ? ? ? ? 8B 40` |
| `GetTraceInfo` | `raw` | `0x7FFF56D346A0` | `0x8046A0` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B E9 0F 29 74 24 ? 48 8B CA` |
| `GetUserCmdManager` | `raw` | `0x7FFF56DE81C0` | `0x8B81C0` | `41 56 41 57 48 83 EC ? 48 8D 54 24` |
| `GetViewAngles` | `raw` | `0x7FFF57000030` | `0xAD0030` | `4C 8B C1 85 D2 74 08 48 8D 05 ? ? ? ? C3` |
| `GetWeaponInAccuracyRecoveryTime` | `rel32` | `0x7FFF56CC6B70` | `0x796B70` | `E8 ? ? ? ? F3 0F 10 B7 ? ? ? ? F3 0F 5E F8` |
| `GlobalVariables_ptr` | `riprel` | `0x7FFF585744E8` | `0x20444E8` | `48 89 15 ? ? ? ? 48 89 42` |
| `HandleBulletPenetration` | `raw` | `0x7FFF56D4D8A0` | `0x81D8A0` | `48 8B C4 44 89 48 ? 48 89 50 ? 48 89 48 ? 55` |
| `HandleEntityList` | `rel32` | `0x7FFF566F35F0` | `0x1C35F0` | `E8 ? ? ? ? 84 C0 74 ? 48 63 03` |
| `HandleTeamIntro` | `raw` | `0x7FFF56C34290` | `0x704290` | `48 83 EC ? ? ? ? ? 44 38 89` |
| `HudChatPrintf` | `rel32` | `0x7FFF575EA150` | `0x10BA150` | `E8 ? ? ? ? 49 8B 4E 20 BA ? ? ? ?` |
| `InitFilter` | `raw` | `0x7FFF5685BAD0` | `0x32BAD0` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 0F B6 41 ? 33 FF 24 C9 C7 41 ?` |
| `InitPlayerMovementTraceFilter` | `raw` | `0x7FFF56D6B590` | `0x83B590` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 0F B6 41 ? 33 FF C7 41 ?` |
| `InitTraceInfo` | `raw` | `0x7FFF57B25340` | `0x15F5340` | `40 55 41 55 41 57 48 83 EC` |
| `IsGlowing` | `rel32` | `0x7FFF5703A500` | `0xB0A500` | `E8 ? ? ? ? 33 DB 84 C0 0F 84 ? ? ? ? 48 8B 4F` |
| `LevelInit` | `raw` | `0x7FFF56DFA630` | `0x8CA630` | `40 55 56 41 56 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48` |
| `LoadFileForMe` | `raw` | `0x7FFF56E46470` | `0x916470` | `40 55 57 41 56 48 83 EC 20 4C` |
| `LoadPath` | `rel32` | `0x7FFF56BEB5E0` | `0x6BB5E0` | `E8 ? ? ? ? 8B 44 24 2C` |
| `LookupBone` | `rel32` | `0x7FFF56DF2710` | `0x8C2710` | `E8 ? ? ? ? 48 8B 8D ? ? ? ? B3` |
| `ModulationUpdate` | `raw` | `0x7FFF56F04840` | `0x9D4840` | `48 89 5C 24 08 57 48 83 EC 20 8B FA 48 8B D9 E8 ? ? ? ? 84 C0 0F 84` |
| `NoSpread1` | `raw` | `0x7FFF571A7780` | `0xC77780` | `48 89 5C 24 08 57 48 81 EC F0 00` |
| `ParticleCollection` | `raw` | `0x7FFF56724C80` | `0x1F4C80` | `48 89 5C 24 ? 57 48 83 EC ? 0F 28 05` |
| `ParticleManager_ptr` | `riprel` | `0x7FFF585588E8` | `0x20288E8` | `48 8B 0D ? ? ? ? 41 B8 ? ? ? ? F3 0F 11 74 24 ? 48 C7 44 24 ? ? ? ? ?` |
| `PhysicsRunThink_Ctrl` | `raw` | `0x7FFF56E01840` | `0x8D1840` | `48 89 5C 24 ? 57 48 81 EC ? ? ? ? ? ? ? 48 8B F9 FF 90` |
| `PhysicsRunThink_Pawn` | `raw` | `0x7FFF5703E820` | `0xB0E820` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 8B 81 ? ? ? ? 48 8B F9` |
| `PlayVSound_client` | `raw` | `0x7FFF57A37DA0` | `0x1507DA0` | `48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 48 8D 6C 24 ? 48 81 EC ? ? ? ? 33 FF` |
| `ProcessImpacts` | `raw` | `0x7FFF56EF8E40` | `0x9C8E40` | `48 8B C4 53 56 41 55` |
| `ProcessMovement` | `rel32` | `0x7FFF56F03E20` | `0x9D3E20` | `E8 ? ? ? ? 48 8B 06 48 8B CE FF 90 ? ? ? ? 48 85 DB` |
| `RegenerateWeaponSkin` | `raw` | `0x7FFF56CBC6B0` | `0x78C6B0` | `40 55 53 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 44 0F B6 FA 48 8B D9 BA ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ?` |
| `RegenerateWeaponSkins` | `raw` | `0x7FFF56CE1380` | `0x7B1380` | `48 83 EC ? E8 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? 48 8B 10` |
| `ReportHit` | `rel32` | `0x7FFF56B32660` | `0x602660` | `E8 ? ? ? ? 48 8B AC 24 D8 00 00 00 48 81 C4` |
| `RunCommand` | `raw` | `0x7FFF56F05EE0` | `0x9D5EE0` | `48 8B C4 48 81 EC ? ? ? ? 48 89 58 10` |
| `Scope_callsite` | `rel32` | `0x7FFF56D8A980` | `0x85A980` | `E8 ? ? ? ? 80 7C 24 34 ? 74 ?` |
| `SendChatMessage` | `rel32` | `0x7FFF575EA150` | `0x10BA150` | `E8 ? ? ? ? 49 8B 4E 20 BA ? ? ? ?` |
| `SetAbsOrigin_Pawn` | `raw` | `0x7FFF5674EE40` | `0x21EE40` | `48 89 5C 24 ? 57 48 83 EC ? ? ? ? 48 8B FA 48 8B D9 FF 90 ? ? ? ? 84 C0 0F 85` |
| `SetBodyGroup_inv` | `raw` | `0x7FFF572C05A0` | `0xD905A0` | `85 D2 0F 88 ? ? ? ? 53 55` |
| `SetCollisionBounds` | `raw` | `0x7FFF56D309D0` | `0x8009D0` | `48 83 EC ? F2 0F 10 02 8B 42 08` |
| `SetDynamicAttributeValue` | `raw` | `0x7FFF5752DFC0` | `0xFFDFC0` | `48 89 6C 24 ? 57 41 56 41 57 48 81 EC ? ? ? ? 48 8B FA C7 44 24 ? ? ? ? ? 4D 8B F8` |
| `SetDynamicAttributeValue_raw` | `raw` | `0x7FFF5752DFC0` | `0xFFDFC0` | `48 89 6C 24 ? 57 41 56 41 57 48 81 EC ? ? ? ? 48 8B FA C7 44 24` |
| `SetMeshGroupMask` | `raw` | `0x7FFF56F57EE0` | `0xA27EE0` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8D 99 ? ? ? ? 48 8B 71` |
| `SetModel` | `raw` | `0x7FFF56E056F0` | `0x8D56F0` | `40 53 48 83 EC ? 48 8B D9 4C 8B C2 48 8B 0D ? ? ? ? 48 8D 54 24` |
| `SetPlayerReady` | `raw` | `0x7FFF57446EB0` | `0xF16EB0` | `40 53 48 83 EC 20 48 8B DA 48 8D 15 ? ? ? ? 48 8B CB FF 15 ? ? ? ? 85 C0 75 14 BA` |
| `SetTraceData` | `rel32` | `0x7FFF56D03600` | `0x7D3600` | `E8 ? ? ? ? 8B 85 ? ? ? ? 48 8D 54 24 ? F2 0F 10 45` |
| `SetTypeKV3` | `raw` | `0x7FFF57D43F50` | `0x1813F50` | `40 53 48 83 EC 30 4C 8B 11 41 B9 ? ? ? ? 49 83 CA 01 0F B6 C2 80 FA 06 48 8B D9 44 0F 45 C8` |
| `SetViewAngle` | `raw` | `0x7FFF5700F070` | `0xADF070` | `85 D2 75 3D 48 63 81 ? ? ? ?` |
| `SetupCmd` | `raw` | `0x7FFF56DE5450` | `0x8B5450` | `48 83 EC 28 E8 ? ? ? ? 8B 80` |
| `SetupMove` | `raw` | `0x7FFF57246400` | `0xD16400` | `48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 83 EC ? 48 8B EA 4C 8B F1 E8 ? ? ? ? 48 8D 15` |
| `SetupMovementMoves` | `raw` | `0x7FFF576AFD3F` | `0x117FD3F` | `48 8B ? E8 ? ? ? ? 48 8B 5C 24 ? 48 8B 6C 24 ? 48 83 C4 30` |
| `SomeTimingFromPawn` | `raw` | `0x7FFF56F81640` | `0xA51640` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 49 63 D8 48 8B F1` |
| `TestSurfaces` | `raw` | `0x7FFF56D34580` | `0x804580` | `40 53 57 41 56 48 83 EC 50 8B` |
| `TracePlayerBBox` | `raw` | `0x7FFF5709B0A0` | `0xB6B0A0` | `48 89 5C 24 ? 55 57 41 54 41 55 41 56` |
| `TraceShape` | `raw` | `0x7FFF56EB8E90` | `0x988E90` | `48 89 5C 24 ? 48 89 4C 24 ? 55 57` |
| `TraceToExit` | `raw` | `0x7FFF56D32700` | `0x802700` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? F2 0F 10 02` |
| `UpdatePostProcessing` | `raw` | `0x7FFF5744B040` | `0xF1B040` | `48 85 D2 0F 84 ? ? ? ? 48 89 5C 24 08 57 48 83 EC 60 80` |
| `UpdateSubClass` | `raw` | `0x7FFF5672A82B` | `0x1FA82B` | `48 8B 41 10 48 8B D9 8B 50 30` |
| `UpdateTurningInAccuracy` | `rel32` | `0x7FFF56CE03E0` | `0x7B03E0` | `E8 ? ? ? ? F3 0F 10 87 ? ? ? ? 44 0F 2F C8` |
| `VPhys2World_ptr` | `riprel` | `0x7FFF58558558` | `0x2028558` | `4C 8B 25 ? ? ? ? 24` |
| `ViewRender_ptr` | `riprel` | `0x7FFF588592E8` | `0x23292E8` | `48 89 05 ? ? ? ? 48 8B C8 48 85 C0` |
| `create_move_v2` | `raw` | `0x7FFF56FF64B0` | `0xAC64B0` | `85 D2 0F 85 ? ? ? ? 48 8B C4 44 88 40` |
| `draw_smoke_array` | `raw` | `0x7FFF571A4830` | `0xC74830` | `40 55 41 54 41 55 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 4C 8B E2` |
| `draw_view_punch_v2` | `raw` | `0x7FFF56D31140` | `0x801140` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 49 8B E9 49 8B F8` |
| `entity_list_ptr` | `riprel` | `0x7FFF589F9818` | `0x24C9818` | `48 8B 1D ? ? ? ? 48 8D 46` |
| `frame_stage_notify` | `raw` | `0x7FFF56FFD0C1` | `0xACD0C1` | `4C 8B 0D ? ? ? ? 48 8D 15 ? ? ? ? 48 8B 8F ? ? ? ? F3 41 0F 10 51 ? 45 8B 49 ? 0F 5A D2 66 49 0F 7E D0 FF 15 ? ? ? ? 48 8B 97 ? ? ? ? 48 8B 0D ? ? ? ? E8 ? ? ? ? E9` |
| `get_fov` | `raw` | `0x7FFF56D31140` | `0x801140` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 49 8B E9 49 8B F8` |
| `get_map_name` | `raw` | `0x7FFF57406790` | `0xED6790` | `48 83 EC ? 48 8B 0D ? ? ? ? ? ? ? FF 90 ? ? ? ? 48 8B C8 48 83 C4` |
| `get_view_angles_v2` | `raw` | `0x7FFF56FFE990` | `0xACE990` | `4D 85 C0 74 ? 85 D2 74` |
| `get_view_model` | `raw` | `0x7FFF56D7C3E0` | `0x84C3E0` | `40 55 53 56 41 56 41 57 48 8B EC` |
| `global_vars_v2` | `riprel` | `0x7FFF58853900` | `0x2323900` | `48 89 1D ? ? ? ? FF 15 ? ? ? ? 84 C0 74 ? 8B 0D ? ? ? ? 4C 8D 0D ? ? ? ? 4C 8D 05 ? ? ? ? BA ? ? ? ? FF 15 ? ? ? ? 48 8B 74 24 ? 48 8B C3` |
| `is_demo_or_hltv` | `raw` | `0x7FFF57427BC0` | `0xEF7BC0` | `48 83 EC ? 48 8B 0D ? ? ? ? ? ? ? FF 90 ? ? ? ? 84 C0 75 ? 38 05` |
| `level_init_v2` | `raw` | `0x7FFF57024E40` | `0xAF4E40` | `40 55 56 41 56 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 0D` |
| `level_shutdown` | `raw` | `0x7FFF570250C0` | `0xAF50C0` | `48 83 EC ? 48 8B 0D ? ? ? ? 48 8D 15 ? ? ? ? 45 33 C9 45 33 C0 ? ? ? FF 50 ? 48 85 C0 74 ? 48 8B 0D ? ? ? ? 48 8B D0 ? ? ? 41 FF 50 ? 48 83 C4` |
| `local_controller` | `riprel` | `0x7FFF58832E80` | `0x2302E80` | `48 8B 05 ? ? ? ? 41 89 BE` |
| `mark_interp_latch_flags_dirty` | `raw` | `0x7FFF56747F60` | `0x217F60` | `40 53 56 57 48 83 EC ? 80 3D ? ? ? ? 00` |
| `on_add_entity_v2` | `raw` | `0x7FFF56E93030` | `0x963030` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 8B 81 ? ? ? ? 49 8B F0` |
| `override_view_short` | `raw` | `0x7FFF57188CF0` | `0xC58CF0` | `40 57 48 83 EC ? 48 8B FA E8 ? ? ? ? BA` |
| `paintkit_prefab` | `stringref` | `0x7FFF57586410` | `0x1056410` | `"set item texture prefab"` |
| `paintkit_seed` | `stringref` | `0x7FFF5741A5C0` | `0xEEA5C0` | `"set item texture seed"` |
| `paintkit_wear` | `stringref` | `0x7FFF5741A5C0` | `0xEEA5C0` | `"set item texture wear"` |
| `planted_c4_ptr` | `riprel` | `0x7FFF587D1928` | `0x22A1928` | `48 8B 15 ? ? ? ? 48 8B 5C 24 ? FF C0 89 05 ? ? ? ? 48 8B C6 ? ? ? ? 80 BE ? ? ? ? 00` |
| `remove_legs` | `raw` | `0x7FFF576193C0` | `0x10E93C0` | `40 55 53 56 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? F2 0F 10 42` |
| `statTrak_killEater` | `stringref` | `0x7FFF5741A5C0` | `0xEEA5C0` | `"kill eater"` |
| `statTrak_scoreType` | `stringref` | `0x7FFF5664B6E0` | `0x11B6E0` | `"kill eater score type"` |
| `unlock_inventory` | `raw` | `0x7FFF56C315A0` | `0x7015A0` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B E9 48 8B 0D ? ? ? ? ? ? ? FF 50` |
| `update_global_vars` | `raw` | `0x7FFF5700EAC0` | `0xADEAC0` | `48 8B 0D ? ? ? ? 4C 8D 05 ? ? ? ? 48 85 D2` |
| `update_post_processing_v2` | `raw` | `0x7FFF5744F5F6` | `0xF1F5F6` | `48 89 AC 24 ? ? ? ? 45 33 ED` |
| `view_matrix_ptr` | `riprel` | `0x7FFF588590D0` | `0x23290D0` | `48 8D 0D ? ? ? ? 48 89 44 24 ? 48 89 4C 24 ? 4C 8D 0D` |

## `engine2.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `Engine::GetScreenAspectRatio` | `raw` | `0x7FFF9F6169D0` | `0x769D0` | `48 89 5C 24 08 57 48 83 EC 20 8B FA 48 8D 0D` |
| `Engine::PVSManager_ptr` | `riprel` | `0x7FFF9FBB33F0` | `0x6133F0` | `48 8D 0D ? ? ? ? 33 D2 FF 50` |
| `Engine::RunPrediction` | `raw` | `0x7FFF9F606490` | `0x66490` | `40 55 41 56 48 83 EC ? 80 B9` |

## `materialsystem2.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `FindParameter` | `raw` | `0x7FFF89661E30` | `0x11E30` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B 59 20 48` |
| `MatSys::PrepareSceneMaterial` | `raw` | `0x7FFF89661BE0` | `0x11BE0` | `48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 30 48 8B 59 ? 48 8B F2 48 63 79 ? 48 C1 E7 06` |
| `UpdateParameter` | `raw` | `0x7FFF89662370` | `0x12370` | `48 89 7C 24 ? 41 56 48 83 EC ? 8B 81` |

## `particles.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `Particles::DrawArray` | `raw` | `0x7FFF60C020B0` | `0x220B0` | `40 55 53 56 57 48 8D 6C 24` |
| `Particles::FindKeyVar` | `raw` | `0x7FFF60C1A650` | `0x3A650` | `48 89 5C 24 ? 57 48 81 EC ? ? ? ? 33 C0 8B DA` |
| `Particles::SetMaterialShaderType` | `raw` | `0x7FFF60C7D8D0` | `0x9D8D0` | `48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 54 41 56 41 57 48 81 EC ? ? ? ? 4C 63 32` |

## `scenesystem.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `CSceneAnimatableObject::GeneratePrimitives` | `raw` | `0x7FFF61433520` | `0x73520` | `48 8B C4 48 89 58 08 48 89 50 10 55 56 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ?` |
| `DrawObject_legacy` | `raw` | `0x7FFF61415BC0` | `0x55BC0` | `48 8B C4 53 57 41 54 48 81 EC D0 00 00 00 49 63 F9 49` |
| `DrawSkyboxArray` | `raw` | `0x7FFF6150FB90` | `0x14FB90` | `45 85 C9 0F 8E ? ? ? ? 4C 8B DC 55` |
| `SceneSystem::DrawAggeregateObject` | `raw` | `0x7FFF614ECF50` | `0x12CF50` | `48 8B C4 4C 89 48 20 4C 89 40 ? 48 89 50 ? 55 53 41 57 48 8D A8` |
| `SceneSystem::DrawArrayLight` | `raw` | `0x7FFF6143AAC0` | `0x7AAC0` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 54 24` |

## `soundsystem.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `SoundSystem::PlayVSound` | `raw` | `0x7FFF654D9820` | `0x349820` | `48 8B C4 48 89 58 08 57 48 81 EC ? ? ? ? 33 FF 48 8B D9` |
| `SoundSystem::SomeUtlSymbolFunc` | `raw` | `0x7FFF65240720` | `0xB0720` | `48 89 74 24 18 57 48 83 EC 20 48 63 F2 48 8B F9 3B 71 30` |

## `tier0.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `Tier0::LoadKeyValues` | `rel32` | `0x7FFFA0029160` | `0x129160` | `E8 ? ? ? ? 8B 4C 24 34 0F B6 D8` |
| `Tier0::UtlBuffer` | `raw` | `0x7FFF9FF53F10` | `0x53F10` | `48 89 5C 24 ? 57 48 83 EC ? 8B 41 ? 8D 7A` |

