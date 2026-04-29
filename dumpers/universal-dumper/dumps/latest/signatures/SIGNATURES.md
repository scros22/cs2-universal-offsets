# CS2 Signatures

_This file is regenerated on every successful run of `cs2-sdk`._

**354/400 signatures resolved across 16 module(s).**

## `animationsystem.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `Animation::ShouldUpdateSequences` | `raw` | `0x7FFE8B50F0A0` | `0x14F0A0` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 49 8B 40 48` |
| `AnimationSystemUtils_ptr` | `riprel` | `0x7FFE8BBD2170` | `0x812170` | `48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 48 83 EC 28 48 8B CA 48 8D 15` |

## `client.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `AddNametagEntity` | `raw` | `0x7FFE5B57AF80` | `0x78AF80` | `40 55 53 56 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B DA` |
| `AddStattrakEntity` | `raw` | `0x7FFE5B83AEF0` | `0xA4AEF0` | `48 8B C4 48 89 58 08 48 89 70 10 57 48 83 EC 50 33 F6 8B FA 48 8B D1` |
| `AutowallInit` | `raw` | `0x7FFE5B6D0770` | `0x8E0770` | `40 53 48 83 EC ? 48 8B D9 48 81 C1 ? ? ? ? E8 ? ? ? ?` |
| `AutowallTraceData` | `raw` | `0x7FFE5B77D120` | `0x98D120` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B 09` |
| `AutowallTracePos` | `raw` | `0x7FFE5B5F6CC0` | `0x806CC0` | `40 55 56 41 54 41 55 41 57 48 8B EC` |
| `BulkRegenIterator` | `raw` | `0x7FFE5B57E481` | `0x78E481` | `57 48 83 EC 40 0F B6 F9 E8 ? ? ? ? 48 85 C0 0F 84` |
| `CAM_ThinkReturn` | `raw` | `0x7FFE5B10A4FF` | `0x31A4FF` | `BA 04 00 00 00 FF 15 ? ? ? ? 84 C0 0F 84` |
| `CAttributeStringFill` | `rel32` | `0x7FFE5BC9C4A0` | `0xEAC4A0` | `E8 ? ? ? ? 41 83 CF 08` |
| `CAttributeStringInit` | `rel32` | `0x7FFE5B3E86B0` | `0x5F86B0` | `E8 ? ? ? ? 48 8D 05 ? ? ? ? 48 89 7D ? 48 89 45 ? 49 8D 4F` |
| `CBaseEntity_TakeDamageOld` | `raw` | `0x7FFE5B013D20` | `0x223D20` | `40 55 53 56 57 41 54 48 8D 6C 24 E0 48 81 EC 20 01 00 00 4D 8B E0 48 8B FA 48 8B F1 E8` |
| `CBaseModelEntity_SetBodygroup` | `raw` | `0x7FFE5B6C8700` | `0x8D8700` | `85 D2 0F 88 CB 01 00 00 55 53 56 41 56 48 8B EC 48 83 EC 78 45 8B F0 8B DA 48 8B F1 E8 ? ? ?` |
| `CBodyComponent` | `stringref` | `0x7FFE5AFAC160` | `0x1BC160` | `"CBodyComponent"` |
| `CBodyComponentSkeletonInstance` | `stringref` | `0x7FFE5AFB3040` | `0x1C3040` | `"CBodyComponentSkeletonInstance"` |
| `CBufferStringInit` | `raw` | `0x7FFE5C5D0250` | `0x17E0250` | `48 89 5C 24 ? 57 48 83 EC ? 8B 41 ? 48 8D 79` |
| `CCSGOInput::CreateMove` | `raw` | `0x7FFE5BA4C1A0` | `0xC5C1A0` | `48 8B C4 4C 89 40 18 48 89 48 08 55 53 41 54 41 55` |
| `CCSGameRules` | `stringref` | `0x7FFE5AE6E160` | `0x7E160` | `"CCSGameRules"` |
| `CCSGameRulesProxy` | `stringref` | `0x7FFE5B4D9500` | `0x6E9500` | `"CCSGameRulesProxy"` |
| `CCSInventoryManager::EquipItemInLoadout` | `raw` | `0x7FFE5B5B2130` | `0x7C2130` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 89 54 24 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 0F B7 FA` |
| `CCSPlayerController` | `stringref` | `0x7FFE5B5D5020` | `0x7E5020` | `"CCSPlayerController"` |
| `CCSPlayerController` | `stringref` | `0x7FFE5B5D5020` | `0x7E5020` | `"CCSPlayerController"` |
| `CCSPlayerController_ActionTrackingServices` | `stringref` | `0x7FFE5B5D5020` | `0x7E5020` | `"CCSPlayerController_ActionTrackingServices"` |
| `CCSPlayerController_DamageServices` | `stringref` | `0x7FFE5B5D5020` | `0x7E5020` | `"CCSPlayerController_DamageServices"` |
| `CCSPlayerController_InGameMoneyServices` | `stringref` | `0x7FFE5B5D5020` | `0x7E5020` | `"CCSPlayerController_InGameMoneyServices"` |
| `CCSPlayerController_InventoryServices` | `stringref` | `0x7FFE5B5D5020` | `0x7E5020` | `"CCSPlayerController_InventoryServices"` |
| `CCSPlayerInventory::GetItemInLoadout` | `raw` | `0x7FFE5B5B3D50` | `0x7C3D50` | `40 55 48 83 EC ? 49 63 E8` |
| `CCSPlayerPawn` | `stringref` | `0x7FFE5B99F5A0` | `0xBAF5A0` | `"CCSPlayerPawn"` |
| `CCSPlayer_BulletServices` | `stringref` | `0x7FFE5B602D60` | `0x812D60` | `"CCSPlayer_BulletServices"` |
| `CCSPlayer_BulletServices` | `stringref` | `0x7FFE5B602D60` | `0x812D60` | `"CCSPlayer_BulletServices"` |
| `CCSPlayer_CameraServices` | `stringref` | `0x7FFE5B5FEE70` | `0x80EE70` | `"CCSPlayer_CameraServices"` |
| `CCSPlayer_HostageServices` | `stringref` | `0x7FFE5B602D60` | `0x812D60` | `"CCSPlayer_HostageServices"` |
| `CCSPlayer_ItemServices` | `stringref` | `0x7FFE5B63F6F0` | `0x84F6F0` | `"CCSPlayer_ItemServices"` |
| `CCSPlayer_MovementServices` | `stringref` | `0x7FFE5B62CBD0` | `0x83CBD0` | `"CCSPlayer_MovementServices"` |
| `CCSPlayer_MovementServices` | `stringref` | `0x7FFE5B62CBD0` | `0x83CBD0` | `"CCSPlayer_MovementServices"` |
| `CCSPlayer_PingServices` | `stringref` | `0x7FFE5B640900` | `0x850900` | `"CCSPlayer_PingServices"` |
| `CCSPlayer_RunCommand_Context` | `raw` | `0x7FFE5B7CA250` | `0x9DA250` | `48 8B C4 48 81 EC C8 00 00 00 48 89 58 10 48 89 68 18 48 8B EA 48 89 70 20 48 8B F1 48 89 78 F8` |
| `CCSPlayer_UseServices` | `stringref` | `0x7FFE5B670A60` | `0x880A60` | `"CCSPlayer_UseServices"` |
| `CCSPlayer_WaterServices` | `stringref` | `0x7FFE5B665CF0` | `0x875CF0` | `"CCSPlayer_WaterServices"` |
| `CCSPlayer_WeaponServices` | `stringref` | `0x7FFE5B6660A0` | `0x8760A0` | `"CCSPlayer_WeaponServices"` |
| `CCSPlayer_WeaponServices` | `stringref` | `0x7FFE5B6660A0` | `0x8760A0` | `"CCSPlayer_WeaponServices"` |
| `CCSWeaponBase` | `stringref` | `0x7FFE5B56F3C0` | `0x77F3C0` | `"CCSWeaponBase"` |
| `CCSWeaponBaseGun` | `stringref` | `0x7FFE5B56F460` | `0x77F460` | `"CCSWeaponBaseGun"` |
| `CCSWeaponBaseVData` | `stringref` | `0x7FFE5B54A2A0` | `0x75A2A0` | `"CCSWeaponBaseVData"` |
| `CCollisionProperty` | `stringref` | `0x7FFE5B0D0F90` | `0x2E0F90` | `"CCollisionProperty"` |
| `CCompositeMaterialManager_AddPanoramaPanelRenderRequest_Caller` | `stringref` | `0x7FFE5C1A9154` | `0x13B9154` | `"CCompositeMaterialManager::AddNewPanoramaPanelRenderRequest"` |
| `CDecoyProjectile` | `stringref` | `0x7FFE5B53E1D0` | `0x74E1D0` | `"CDecoyProjectile"` |
| `CEconItemSchema::GetAttributeDefinitionByName` | `raw` | `0x7FFE5BE3A720` | `0x104A720` | `48 89 5C 24 10 48 89 6C 24 18 57 41 56 41 57 48 83 EC 60 48 8D 05` |
| `CEconItemView::GetCustomPaintKitIndex` | `raw` | `0x7FFE5B6A9B80` | `0x8B9B80` | `48 89 5C 24 ? 57 48 83 EC ? 8B 15 ? ? ? ? 48 8B F9 65 48 8B 04 25` |
| `CFlashbangProjectile` | `stringref` | `0x7FFE5BDCDC70` | `0xFDDC70` | `"CFlashbangProjectile"` |
| `CFogController` | `stringref` | `0x7FFE5B06EFD0` | `0x27EFD0` | `"CFogController"` |
| `CGameEntitySystem::OnAddEntity` | `raw` | `0x7FFE5B756E30` | `0x966E30` | `48 89 74 24 ? 57 48 83 EC ? 41 B9 ? ? ? ? 41 8B C0 41 23 C1 48 8B F2 41 83 F8 ? 48 8B F9 44 0F 45 C8 41 81 F9 ? ? ? ? 73 ? FF 81` |
| `CGameEntitySystem::OnRemoveEntity` | `raw` | `0x7FFE5B757690` | `0x967690` | `48 89 74 24 ? 57 48 83 EC ? 41 B9 ? ? ? ? 41 8B C0 41 23 C1 48 8B F2 41 83 F8 ? 48 8B F9 44 0F 45 C8 41 81 F9 ? ? ? ? 73 ? FF 89` |
| `CGameSceneNode` | `stringref` | `0x7FFE5AF938F0` | `0x1A38F0` | `"CGameSceneNode"` |
| `CGameSceneNode_BuildBoneMergeWork` | `raw` | `0x7FFE5B72E2D0` | `0x93E2D0` | `40 55 56 57 41 54 41 55 41 56 41 57 48 83 EC 50 48 8D 6C 24 50 80 A1 06 01 00 00 FB 4C 8B F9 80` |
| `CGameSceneNode_StartHierarchicalAttachment` | `raw` | `0x7FFE5B77AD40` | `0x98AD40` | `48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20 57 41 54 41 55 41 56 41 57 48 83 EC 30 48 8B F9 8B` |
| `CGameTrace_TraceShape_Client` | `raw` | `0x7FFE5B77D200` | `0x98D200` | `48 89 5C 24 20 48 89 4C 24 08 55 57 41 54 41 55 41 56 48 8D AC 24 10 E0 FF FF B8 F0 20 00 00` |
| `CGlowProperty` | `stringref` | `0x7FFE5B0D11A0` | `0x2E11A0` | `"CGlowProperty"` |
| `CGlowProperty_OnGlowTypeChanged` | `raw` | `0x7FFE5B8FB4F0` | `0xB0B4F0` | `48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 48 8B 05 ? ? ? ? 48 8B D9 F3 0F 10 41 4C` |
| `CHEGrenadeProjectile` | `stringref` | `0x7FFE5BDCDD10` | `0xFDDD10` | `"CHEGrenadeProjectile"` |
| `CInputPtrGlobal` | `riprel` | `0x7FFE5CE512C0` | `0x20612C0` | `4C 8B 05 ? ? ? ? 41 8B 80 50 0B 00 00 85 C0` |
| `CMolotovProjectile` | `stringref` | `0x7FFE5B53E3B0` | `0x74E3B0` | `"CMolotovProjectile"` |
| `CPaintKitDefinitions_FindOrCreateByName` | `stringref` | `0x7FFE5BE47F10` | `0x1057F10` | `"Kit "[%s]" specified, but doesn't exist!! You're probably missing an entry in items_paintkits.txt or items_stickerkits.txt or need to run with -use_local_item_data\n"` |
| `CPaintKitDefinitions_LoadDefaultKit` | `stringref` | `0x7FFE5BE19FE0` | `0x1029FE0` | `"Unable to find "default" paint kit in "paint_kits_rarity""` |
| `CPostProcessingVolume` | `stringref` | `0x7FFE5B093D60` | `0x2A3D60` | `"CPostProcessingVolume"` |
| `CS2ItemEditor_BuildTemplateMaterialFromFile` | `raw` | `0x7FFE5C1AA2D0` | `0x13BA2D0` | `48 89 54 24 10 55 53 41 55 41 57 48 8D AC 24 18 F9 FF FF 48 81 EC E8 07 00 00 4C 8B FA 48 85 D2` |
| `CSBaseGunFireData_fn` | `raw` | `0x7FFE5C2D59C0` | `0x14E59C0` | `48 8B C4 55 53 56 57 41 54 41 55 41 56 41 57 48 8D 68 A8 48 81 EC ? ? ? ? 4C 8B 69` |
| `CSGOInput_ptr` | `riprel` | `0x7FFE5CE512C0` | `0x20612C0` | `48 8B 0D ? ? ? ? 4C 8B C6 8B 10 E8` |
| `CSGOInput_resolved` | `riprel` | `0x7FFE5CE512C7` | `0x20612C7` | `48 8B 0D ? ? ? ? 8B 10 E8 ? ? ? ? 45 32 FF` |
| `CSkeletonInstance` | `stringref` | `0x7FFE5AF93A20` | `0x1A3A20` | `"CSkeletonInstance"` |
| `CSkeletonInstance::SetMeshGroupMask` | `raw` | `0x7FFE5B81C2B0` | `0xA2C2B0` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8D 99` |
| `CSkeletonInstance_GetTransformsForHitboxList` | `raw` | `0x7FFE5B808E20` | `0xA18E20` | `48 89 5C 24 18 55 56 57 41 55 41 57 48 81 EC A0 00 00 00 49 63 28 4D 8B F8 48 8B FA 48 8B D9 85` |
| `CSkeletonInstance_OnBodyGroupChoiceChanged` | `raw` | `0x7FFE5B813A70` | `0xA23A70` | `48 89 5C 24 08 57 48 83 EC 20 49 63 D8 49 8B F9 45 85 C0 78 20 3B 99 18 02 00 00 7D 18` |
| `CSkeletonInstance_OnSkeletonModelChanged` | `raw` | `0x7FFE5B813C80` | `0xA23C80` | `49 8B 00 48 89 81 B8 00 00 00 C6 81 B0 00 00 00 01 C3` |
| `CSkeletonInstance_PostDataUpdate` | `raw` | `0x7FFE5B814C10` | `0xA24C10` | `48 8B C4 4C 89 40 18 89 50 10 55 57 48 8D A8 68 FE FF FF 48 81 EC 88 02 00 00 48 89 70 E0 48 8B` |
| `CSkeletonInstance_SetMaterialGroup` | `raw` | `0x7FFE5B81AF90` | `0xA2AF90` | `3B 91 C4 03 00 00 74 24 89 91 C4 03 00 00 48 8B 81 28 02 00 00 48 85 C0 74 12` |
| `CSkeletonInstance_SetMeshGroupMask` | `raw` | `0x7FFE5B813BE0` | `0xA23BE0` | `48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 49 8B 00 49 8B F8 48 8B F2 48 8B D9 48 39 81 C8 01` |
| `CSmokeGrenadeProjectile` | `stringref` | `0x7FFE5B53E450` | `0x74E450` | `"CSmokeGrenadeProjectile"` |
| `CTonemapController2` | `stringref` | `0x7FFE5B047C90` | `0x257C90` | `"CTonemapController2"` |
| `CUtlVector_CompositeMaterialInput_AddToTail` | `raw` | `0x7FFE5B579B60` | `0x789BB2` | `41 B9 88 02 00 00 8B 57 14 81 E2 FF FF FF 3F 8D 71 01 44 8B C6 FF 15` |
| `C_AttributeContainer` | `stringref` | `0x7FFE5BA064A0` | `0xC164A0` | `"C_AttributeContainer"` |
| `C_BaseEntity` | `stringref` | `0x7FFE5AE3E260` | `0x4E260` | `"C_BaseEntity"` |
| `C_BaseModelEntity` | `stringref` | `0x7FFE5AF48010` | `0x158010` | `"C_BaseModelEntity"` |
| `C_BasePlayerPawn` | `stringref` | `0x7FFE5AE5DA20` | `0x6DA20` | `"C_BasePlayerPawn"` |
| `C_C4` | `stringref` | `0x7FFE5AE8A420` | `0x9A420` | `"C_C4"` |
| `C_CSPlayerPawn` | `stringref` | `0x7FFE5B4B2430` | `0x6C2430` | `"C_CSPlayerPawn"` |
| `C_CSPlayerPawnBase` | `stringref` | `0x7FFE5B9C5680` | `0xBD5680` | `"C_CSPlayerPawnBase"` |
| `C_CSWeaponBase` | `stringref` | `0x7FFE5B532160` | `0x742160` | `"C_CSWeaponBase"` |
| `C_EconEntity_BuildLegacyGloveSkinMaterial` | `stringref` | `0x7FFE5B9AF9C0` | `0xBBF9C0` | `"MapPlayerPreview gloves"` |
| `C_EconEntity_BuildLegacyWeaponSkinMaterial` | `stringref` | `0x7FFE5B57C1B0` | `0x78C1B0` | `"workshop preview weapon"` |
| `C_EconEntity_BuildModernWeaponSkinMaterial` | `raw` | `0x7FFE5BB72810` | `0xD82810` | `48 85 C9 0F 84 ? ? 00 00 48 8B C4 48 89 50 10 48 89 48 08 55 41 54 41 56 41 57 48 8D A8 B8 FA` |
| `C_EconEntity_BuildNametagOverlayMaterial` | `stringref` | `0x7FFE5B57AF80` | `0x78AF80` | `"low-res nametag"` |
| `C_EconItemView` | `stringref` | `0x7FFE5B4FB4B0` | `0x70B4B0` | `"C_EconItemView"` |
| `C_EconWearable_OnNewCustomMaterials` | `stringref` | `0x7FFE5BEA6910` | `0x10B6910` | `"Invalid EconItemView -- Can't create custom materials for wearable, debug this.\n"` |
| `C_Hostage` | `stringref` | `0x7FFE5AED7480` | `0xE7480` | `"C_Hostage"` |
| `C_Inferno` | `stringref` | `0x7FFE5AEE7440` | `0xF7440` | `"C_Inferno"` |
| `C_PlantedC4` | `stringref` | `0x7FFE5AEE07A0` | `0xF07A0` | `"C_PlantedC4"` |
| `C_SmokeGrenadeProjectile` | `stringref` | `0x7FFE5AE85A10` | `0x95A10` | `"C_SmokeGrenadeProjectile"` |
| `CacheParticleEffect` | `raw` | `0x7FFE5AFF7BC0` | `0x207BC0` | `4C 8B DC 53 48 81 EC ? ? ? ? F2 0F 10 05` |
| `CalcSpread` | `raw` | `0x7FFE5BA6C5F0` | `0xC7C5F0` | `48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 4C 63 EA` |
| `CalcViewmodel` | `raw` | `0x7FFE5B63E020` | `0x84E020` | `40 55 53 56 41 56 41 57 48 8B EC` |
| `CalcViewmodelTransform_v2` | `raw` | `0x7FFE5B592500` | `0x7A2500` | `48 89 5C 24 20 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 80 48 81 EC 80 01 00 00 48 8B FA` |
| `CalcViewmodelView` | `raw` | `0x7FFE5BA598D0` | `0xC698D0` | `40 53 48 83 EC 60 48 8B 41 08 49 8B D8 8B 48 30 48 C1 E9 0C F6 C1 01 0F 85 48 01 00 00 41 B8 07` |
| `CalculateInterpolation` | `rel32` | `0x7FFE5C2B56F0` | `0x14C56F0` | `E8 ? ? ? ? 8B 45 ? 3B 45 60 75 04 32 D2 EB 09 BA 01 00 00 00 41 0F 4C D5 C0 EA 07 84 D2 0F 85 87` |
| `CalculateWorldSpaceBones` | `raw` | `0x7FFE5B7F97D0` | `0xA097D0` | `48 89 4C 24 ? 55 53 56 57 41 54 41 55 41 56 41 57 B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 48 8D 6C 24 ? 48 8B 81` |
| `ClearHUDWeaponIcon` | `rel32` | `0x7FFE5BBDB650` | `0xDEB650` | `E8 ? ? ? ? 8B F8 C6 84 24 ? ? ? ? ?` |
| `ClientModeCSNormal_OnEvent` | `raw` | `0x7FFE5BA4A010` | `0xC5A010` | `40 53 57 48 81 EC 78 02 00 00 48 8B CA 48 8B FA` |
| `ClientMode_ptr` | `riprel` | `0x7FFE5D12B960` | `0x233B960` | `48 8D 0D ? ? ? ? 48 69 C0 ? ? ? ? 48 03 C1 C3 CC CC` |
| `Client_DispatchSpawn` | `raw` | `0x7FFE5C2C3390` | `0x14D3390` | `4C 8B DC 55 56 48 83 EC 78 49 8B 68 08 48 8B F1 48 85 ED 0F 84 72 01 00 00 49 89 5B 08 49 8D 4B` |
| `CompositeMaterialPanoramaPanel_Init` | `stringref` | `0x7FFE5B97F9C0` | `0xB8F9C0` | `"CompositeMaterialPanoramaPanel_t::Init"` |
| `ConCommand_firstperson` | `raw` | `0x7FFE5B8B8A10` | `0xAC8A10` | `48 83 EC 28 48 8B 0D ? ? ? ? 48 8D 54 24 ? 48 8B 01 FF 90 08 03 00 00 83 7C 24 ? 00 75 ? 48 8B 05 ? ? ? ? C6 80 29 02 00 00 00 C7 80 A8 06 00 00 00` |
| `ConCommand_thirdperson` | `raw` | `0x7FFE5B8B8AF0` | `0xAC8AF0` | `48 83 EC 38 48 8B 0D ? ? ? ? 48 8D 54 24 ? 48 8B 01 FF 90 08 03 00 00 83 7C 24 ? 00 0F 85 ? ? ? ? 4C 8B 05 ? ? ? ? 41 8B 80 50 0B 00 00` |
| `ConvarGet` | `raw` | `0x7FFE5B6ADE92` | `0x8BDE92` | `8B D0 48 8D 0D ? ? ? ? E8 ? ? ? ? 0F 10 45 ? 83 F0 74` |
| `CreateBaseTypeCache` | `raw` | `0x7FFE5C2FE720` | `0x150E720` | `40 53 48 83 EC ? 4C 8B 49 ? 44 8B D2` |
| `CreateEntityByClassName` | `raw` | `0x7FFE5C3F24C6` | `0x16024C6` | `4C 8D 05 ? ? ? ? 4C 8B CF BA 03 00 00 00 FF 15 ? ? ? ? EB ? 0F B7 C8 48` |
| `CreateInterface` | `raw` | `0x7FFE5C623010` | `0x1833010` | `4C 8B 0D ? ? ? ? 4C 8B D2 4C 8B D9 4D 85 C9 74 ? 49 8B 41 08` |
| `CreateNewSubtickMoveStep` | `rel32` | `0x7FFE5B2A1D80` | `0x4B1D80` | `E8 ? ? ? ? 48 8B D0 48 8B CE E8 ? ? ? ? 48 8B C8` |
| `CreateParticleEffect` | `raw` | `0x7FFE5B775780` | `0x985780` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? F3 0F 10 1D ? ? ? ? 41 8B F8 8B DA 4C 8D 05` |
| `CreateSOSubclassEconItem` | `raw` | `0x7FFE5BDE4FF0` | `0xFF4FF0` | `48 83 EC 28 B9 48 00 00 00 E8 ? ? ? ? 48 85` |
| `DestroyParticle` | `raw` | `0x7FFE5B734CB0` | `0x944CB0` | `83 FA ? 0F 84 ? ? ? ? 41 54` |
| `DispatchEffect` | `raw` | `0x7FFE5B14A570` | `0x35A570` | `48 89 5C 24 ? 57 48 83 EC ? 48 8B F9 48 8B DA 48 8D 4C 24` |
| `DispatchSpawn_caller` | `raw` | `0x7FFE5C2C3390` | `0x14D3390` | `4C 8B DC 55 56 48 83 EC 78 49 8B 68 08 48 8B F1 48 85 ED 0F 84 72 01 00 00` |
| `DrawCrosshair` | `raw` | `0x7FFE5B5A0C00` | `0x7B0C00` | `48 89 5C 24 08 57 48 83 EC 20 48 8B D9 E8 ? ? ? ? 48 85` |
| `DrawOverHead` | `raw` | `0x7FFE5B855450` | `0xA65450` | `40 53 48 83 EC ? 48 8B D9 83 FA ? 75 ? 48 8B 0D ? ? ? ? 48 8D 54 24 ? 48 8B 01 FF 90 ? ? ? ? 8B 10` |
| `DrawScopeOverlay` | `raw` | `0x7FFE5B64BF20` | `0x85BF20` | `48 8B C4 53 57 48 83 EC ? 48 8B FA` |
| `DrawSmokeVertex` | `raw` | `0x7FFE5BA68C90` | `0xC78C90` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 48 8B 9C 24 ? ? ? ? 4D 8B F8` |
| `FX_FireBullets` | `raw` | `0x7FFE5BA6BD80` | `0xC7BD80` | `48 8B C4 4C 89 48 20 48 89 50 10 55 53 57 41 54 41 55 48 8D A8 58 FB FF FF 48 81 EC A0 05` |
| `FX_FireBullets` | `raw` | `0x7FFE5BA6BD80` | `0xC7BD80` | `48 8B C4 4C 89 48 20 48 89 50 10 55 53 57 41 54 41 55 48 8D A8 58 FB FF FF 48 81 EC A0 05 00 00` |
| `FindHudElement` | `raw` | `0x7FFE5BBAF718` | `0xDBF718` | `48 8D 15 ? ? ? ? 45 33 C0 B9 ? ? ? ? FF 15 ? ? ? ? EB ? 48 8B 15` |
| `FindHudElement_panorama` | `raw` | `0x7FFE5BBB16F0` | `0xDC16F0` | `4C 8B DC 53 48 83 EC 50 48 8B 05` |
| `FindSOCache` | `raw` | `0x7FFE5C60C900` | `0x181C900` | `48 89 5C 24 08 57 48 83 EC 30 4C 8B 52 08 48 8B D9 8B 0A` |
| `FirstPersonLegs` | `raw` | `0x7FFE5BEDDC90` | `0x10EDC90` | `40 55 53 56 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? F2 0F 10 42` |
| `FlashOverlay` | `raw` | `0x7FFE5BB98B40` | `0xDA8B40` | `85 D2 0F 88 ? ? ? ? 48 89 4C 24` |
| `ForceButtonsDown` | `raw` | `0x7FFE5B7BE890` | `0x9CE890` | `40 53 57 41 56 48 81 EC ? ? ? ? 48 83 79` |
| `GameEntitySystemPtr` | `riprel` | `0x7FFE5D2BEC60` | `0x24CEC60` | `48 8B 1D ? ? ? ? 48 89 1D ? ? ? ?` |
| `GameEventManager_AddListener` | `raw` | `0x7FFE5B728880` | `0x938880` | `48 89 5C 24 10 48 89 6C 24 18 56 57 41 56 48 83 EC 50 41 0F B6 E9 48 8D 99 E0 00 00 00 49 8B F0` |
| `GameEventManager_UnserializeEvent` | `raw` | `0x7FFE5B781060` | `0x991060` | `48 8B C4 48 89 50 10 55 41 54 41 55 41 56 48 8D 68 D8 48 81 EC 08 01 00 00 48 89 58 D8 4C 8D B1` |
| `GameRules_ptr` | `riprel` | `0x7FFE5D118E38` | `0x2328E38` | `48 8B 1D ? ? ? ? 48 8D 54 24 ? 0F 28 D0 48 8D 4C 24 ?` |
| `GetBBox_ptr` | `riprel` | `0x7FFE5D118E38` | `0x2328E38` | `48 8B 0D ? ? ? ? 48 85 C9 74 ? ? ? ? 48 FF A0 ? ? ? ? 48 8D 05` |
| `GetBaseEntity` | `raw` | `0x7FFE5B755DF0` | `0x965DF0` | `4C 8D 49 ? 81 FA` |
| `GetBonePositionByName` | `raw` | `0x7FFE5B6B6A70` | `0x8C6A70` | `40 53 48 83 EC ? 48 8B 89 ? ? ? ? 48 8B DA 48 8B 01 FF 50 ? 48 8B C8` |
| `GetChatObject` | `rel32` | `0x7FFE5BEB0EF0` | `0x10C0EF0` | `E8 ? ? ? ? 48 8B E8 48 85 C0 0F 84 ? ? ? ? 4C 8D 05` |
| `GetClientSystem` | `rel32` | `0x7FFE5BE23DF0` | `0x1033DF0` | `E8 ? ? ? ? 48 8B C8 E8 ? ? ? ? 8B D8 85 C0 74 33` |
| `GetControllerCmd` | `raw` | `0x7FFE5B6AC490` | `0x8BC490` | `40 53 48 83 EC 20 8B DA E8 ? ? ? ? 4C` |
| `GetEconItemSystem` | `raw` | `0x7FFE5B169830` | `0x379830` | `48 83 EC 28 48 8B 05 ? ? ? ? 48 85 C0 0F 85 ? ? ? ? 48 89 5C 24` |
| `GetEntityHandle` | `raw` | `0x7FFE5B73D0C0` | `0x94D0C0` | `48 85 C9 74 32 48 8B 49 10 48 85 C9 74 29 44 8B 41 10 BA` |
| `GetGlowColor` | `raw` | `0x7FFE5B8F9320` | `0xB09320` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B F2 48 8B F9 48 8B 54 24` |
| `GetInstanceS` | `riprel` | `0x7FFE5D0A7580` | `0x22B7580` | `48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 8B 91 ? ? ? ? B8` |
| `GetInt2_Event` | `raw` | `0x7FFE5B29AB40` | `0x4AAB40` | `48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC 20 48 63 FA 41` |
| `GetInventoryManager` | `rel32` | `0x7FFE5B5B6410` | `0x7C6410` | `E8 ? ? ? ? 48 8B D3 48 8B C8 4C 8B 00 41 FF 90 00 02` |
| `GetLocalControllerById` | `raw` | `0x7FFE5B6CF900` | `0x8DF900` | `48 83 EC 28 83 F9 FF 75 ? 48 8B 0D ? ? ? ? 48 8D 54 24 ? 48 8B 01 FF 90 ? ? ? ? 8B 08 48 63 C1 4C 8D 05` |
| `GetLocalPlayer_dispatcher` | `raw` | `0x7FFE5B169200` | `0x379200` | `48 83 EC 38 48 8B 05 ? ? ? ? 48 85 C0 0F 85 14 06 00 00 48 89 5C 24 40 B9 50 00 00 00 48 89` |
| `GetMatrixForView` | `raw` | `0x7FFE5AF59C50` | `0x169C50` | `40 53 48 83 EC 60 0F 29 74 24 50 0F 57 DB F3 0F 10 ? ? ? ? ? 49 8B D8` |
| `GetPlayerByIndex_export` | `raw` | `0x7FFE5BCEE190` | `0xEFE190` | `48 83 EC 28 4C 8D 05 ? ? ? ? 48 8D 15 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 4C 8D` |
| `GetPlayerInterp` | `raw` | `0x7FFE5B6A7CF0` | `0x8B7CF0` | `40 53 48 83 EC ? 48 8B D9 48 8B 0D ? ? ? ? 48 83 C1` |
| `GetRemovedAimPunch_E8` | `rel32` | `0x7FFE5B63C2D0` | `0x84C2D0` | `E8 ? ? ? ? 4C 8B C0 48 8D 55 ? 48 8B CB E8 ? ? ? ? 48 8D 0D` |
| `GetRemovedAimpunch` | `raw` | `0x7FFE5AF02947` | `0x112947` | `F2 0F 10 44 24 ? F2 0F 11 84 24 ? ? ? ? FF 15` |
| `GetSurfaceData` | `rel32` | `0x7FFE5B741D30` | `0x951D30` | `E8 ? ? ? ? 80 78 18 00` |
| `GetTickBase` | `rel32` | `0x7FFE5B6AC290` | `0x8BC290` | `E8 ? ? ? ? EB ? 48 8B 05 ? ? ? ? 8B 40` |
| `GetTraceInfo` | `raw` | `0x7FFE5B5F6490` | `0x806490` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B E9 0F 29 74 24 ? 48 8B CA` |
| `GetUserCmdManager` | `raw` | `0x7FFE5B6AC520` | `0x8BC520` | `41 56 41 57 48 83 EC ? 48 8D 54 24` |
| `GetViewAngles` | `raw` | `0x7FFE5B8C4400` | `0xAD4400` | `4C 8B C1 85 D2 74 08 48 8D 05 ? ? ? ? C3` |
| `GetWeaponInAccuracyRecoveryTime` | `rel32` | `0x7FFE5B586610` | `0x796610` | `E8 ? ? ? ? F3 0F 10 B7 ? ? ? ? F3 0F 5E F8` |
| `GetWorldFovResolver` | `raw` | `0x7FFE5B5FBF30` | `0x80BF30` | `40 53 48 83 EC 50 48 8B D9 E8 ? ? ? ? 48 85 C0 74 ? 48 8B C8 48 83 C4 50 5B E9` |
| `GlobalVariables_ptr` | `riprel` | `0x7FFE5CE395A0` | `0x20495A0` | `48 89 15 ? ? ? ? 48 89 42` |
| `GloveApply_PerTick` | `raw` | `0x7FFE5B9AF9C0` | `0xBBF9C0` | `40 55 56 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B B9 A0 00 00 00` |
| `GlowManager_ptr` | `riprel` | `0x7FFE5D115C30` | `0x2325C30` | `48 8B 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 8B 41` |
| `GlowObjectManager_GetInstance` | `raw` | `0x7FFE5B8F9430` | `0xB09430` | `48 8B 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 8B 41 38 C3` |
| `HandleBulletPenetration` | `raw` | `0x7FFE5B6103B0` | `0x8203B0` | `48 8B C4 44 89 48 ? 48 89 50 ? 48 89 48 ? 55` |
| `HandleEntityList` | `rel32` | `0x7FFE5AFB3700` | `0x1C3700` | `E8 ? ? ? ? 84 C0 74 ? 48 63 03` |
| `HandleTeamIntro` | `raw` | `0x7FFE5B4F3EB0` | `0x703EB0` | `48 83 EC ? ? ? ? ? 44 38 89` |
| `HudChatPrintf` | `rel32` | `0x7FFE5BEAE970` | `0x10BE970` | `E8 ? ? ? ? 49 8B 4E 20 BA ? ? ? ?` |
| `InfoForResourceTypeCCompositeMaterialKit_TypeManager` | `stringref` | `0x7FFE5C1C6930` | `0x13D6930` | `"InfoForResourceTypeCCompositeMaterialKit"` |
| `InfoForResourceTypeCCompositeMaterial_TypeManager` | `raw` | `0x7FFE5C1C6E80` | `0x13D6E80` | `40 55 41 56 48 83 EC 68 48 8B EA 83 F9 06 0F 87 B4 02 00 00` |
| `InitFilter` | `raw` | `0x7FFE5B11BBF0` | `0x32BBF0` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 0F B6 41 ? 33 FF 24 C9 C7 41 ?` |
| `InitPlayerMovementTraceFilter` | `raw` | `0x7FFE5B62F330` | `0x83F330` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 0F B6 41 ? 33 FF C7 41 ?` |
| `InitTraceInfo` | `raw` | `0x7FFE5C3E9B20` | `0x15F9B20` | `40 55 41 55 41 57 48 83 EC` |
| `IsGlowing` | `rel32` | `0x7FFE5B8FAA60` | `0xB0AA60` | `E8 ? ? ? ? 33 DB 84 C0 0F 84 ? ? ? ? 48 8B 4F` |
| `LevelInit` | `raw` | `0x7FFE5B6BE990` | `0x8CE990` | `40 55 56 41 56 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48` |
| `LoadFileForMe` | `raw` | `0x7FFE5B70A7D0` | `0x91A7D0` | `40 55 57 41 56 48 83 EC 20 4C` |
| `LoadPath` | `rel32` | `0x7FFE5B4AB200` | `0x6BB200` | `E8 ? ? ? ? 8B 44 24 2C` |
| `LocalPlayerController_ptr` | `riprel` | `0x7FFE5D0F84C0` | `0x23084C0` | `48 8B 05 ? ? ? ? 41 89 BE` |
| `LookupBone` | `rel32` | `0x7FFE5B6B6A70` | `0x8C6A70` | `E8 ? ? ? ? 48 8B 8D ? ? ? ? B3` |
| `ModulationUpdate` | `raw` | `0x7FFE5B7C8BB0` | `0x9D8BB0` | `48 89 5C 24 08 57 48 83 EC 20 8B FA 48 8B D9 E8 ? ? ? ? 84 C0 0F 84` |
| `NoClipOnChange` | `raw` | `0x7FFE5AF56C00` | `0x166C00` | `48 89 5C 24 10 48 89 74 24 18 48 89 7C 24 20 55 48 8B EC 48 83 EC 30 48 8D 05` |
| `NoSpread1` | `raw` | `0x7FFE5BA6BCD0` | `0xC7BCD0` | `48 89 5C 24 08 57 48 81 EC F0 00` |
| `ParticleCollection` | `raw` | `0x7FFE5AFE4D90` | `0x1F4D90` | `48 89 5C 24 ? 57 48 83 EC ? 0F 28 05` |
| `ParticleManager_ptr` | `riprel` | `0x7FFE5CE1D968` | `0x202D968` | `48 8B 0D ? ? ? ? 41 B8 ? ? ? ? F3 0F 11 74 24 ? 48 C7 44 24 ? ? ? ? ?` |
| `PhysicsRunThink_Ctrl` | `raw` | `0x7FFE5B6C5BA0` | `0x8D5BA0` | `48 89 5C 24 ? 57 48 81 EC ? ? ? ? ? ? ? 48 8B F9 FF 90` |
| `PhysicsRunThink_Pawn` | `raw` | `0x7FFE5B8FD4B0` | `0xB0D4B0` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 8B 81 ? ? ? ? 48 8B F9` |
| `PlayVSound_client` | `raw` | `0x7FFE5C2FC580` | `0x150C580` | `48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 48 8D 6C 24 ? 48 81 EC ? ? ? ? 33 FF` |
| `Prediction_ptr` | `riprel` | `0x7FFE5CE445B0` | `0x20545B0` | `48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 40 53 56 41 54` |
| `ProcessImpacts` | `raw` | `0x7FFE5B7BD1B0` | `0x9CD1B0` | `48 8B C4 53 56 41 55` |
| `ProcessMovement` | `rel32` | `0x7FFE5B7C8190` | `0x9D8190` | `E8 ? ? ? ? 48 8B 06 48 8B CE FF 90 ? ? ? ? 48 85 DB` |
| `RegenerateWeaponSkin` | `raw` | `0x7FFE5B57C1B0` | `0x78C1B0` | `40 55 53 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 44 0F B6 FA 48 8B D9 BA ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ?` |
| `RegenerateWeaponSkin_v2` | `raw` | `0x7FFE5B57C1B0` | `0x78C1B0` | `40 55 53 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 44 0F B6 FA 48 8B D9 BA ? ? ? ? 48 8D 0D ? ? ? ? E8` |
| `RegenerateWeaponSkins` | `raw` | `0x7FFE5B5A0D50` | `0x7B0D50` | `48 83 EC ? E8 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? 48 8B 10` |
| `RenderDecals` | `raw` | `0x7FFE5BEDA2D0` | `0x10EA2D0` | `44 88 4C 24 ? 55 53` |
| `ReportHit` | `rel32` | `0x7FFE5B3F2290` | `0x602290` | `E8 ? ? ? ? 48 8B AC 24 D8 00 00 00 48 81 C4` |
| `RunCommand` | `raw` | `0x7FFE5B7CA250` | `0x9DA250` | `48 8B C4 48 81 EC ? ? ? ? 48 89 58 10` |
| `RunCommand_processor` | `raw` | `0x7FFE5B7CA250` | `0x9DA250` | `48 8B C4 48 81 EC C8 00 00 00 48 89 58 10 48 89 68 18 48 8B EA 48 89 70 20 48 8B F1 48 89 78 F8` |
| `Scope_callsite` | `rel32` | `0x7FFE5B64BF20` | `0x85BF20` | `E8 ? ? ? ? 80 7C 24 34 ? 74 ?` |
| `SendChatMessage` | `rel32` | `0x7FFE5BEAE970` | `0x10BE970` | `E8 ? ? ? ? 49 8B 4E 20 BA ? ? ? ?` |
| `Sensitivity_ptr` | `riprel` | `0x7FFE5D116740` | `0x2326740` | `48 8D 0D ? ? ? ? 66 0F 6E CD` |
| `SetAbsOrigin_Pawn` | `raw` | `0x7FFE5B00EF50` | `0x21EF50` | `48 89 5C 24 ? 57 48 83 EC ? ? ? ? 48 8B FA 48 8B D9 FF 90 ? ? ? ? 84 C0 0F 85` |
| `SetBodyGroup_inv` | `raw` | `0x7FFE5BB84B20` | `0xD94B20` | `85 D2 0F 88 ? ? ? ? 53 55` |
| `SetCollisionBounds` | `raw` | `0x7FFE5B5F3620` | `0x803620` | `48 83 EC ? F2 0F 10 02 8B 42 08` |
| `SetDynamicAttributeValue` | `raw` | `0x7FFE5BDF27E0` | `0x10027E0` | `48 89 6C 24 ? 57 41 56 41 57 48 81 EC ? ? ? ? 48 8B FA C7 44 24 ? ? ? ? ? 4D 8B F8` |
| `SetDynamicAttributeValue_raw` | `raw` | `0x7FFE5BDF27E0` | `0x10027E0` | `48 89 6C 24 ? 57 41 56 41 57 48 81 EC ? ? ? ? 48 8B FA C7 44 24` |
| `SetMeshGroupMask` | `raw` | `0x7FFE5B81C2B0` | `0xA2C2B0` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8D 99 ? ? ? ? 48 8B 71` |
| `SetModel` | `raw` | `0x7FFE5B6C9A50` | `0x8D9A50` | `40 53 48 83 EC ? 48 8B D9 4C 8B C2 48 8B 0D ? ? ? ? 48 8D 54 24` |
| `SetPlayerReady` | `raw` | `0x7FFE5BD0B610` | `0xF1B610` | `40 53 48 83 EC 20 48 8B DA 48 8D 15 ? ? ? ? 48 8B CB FF 15 ? ? ? ? 85 C0 75 14 BA` |
| `SetTraceData` | `rel32` | `0x7FFE5B5C47F0` | `0x7D47F0` | `E8 ? ? ? ? 8B 85 ? ? ? ? 48 8D 54 24 ? F2 0F 10 45` |
| `SetTypeKV3` | `raw` | `0x7FFE5C608730` | `0x1818730` | `40 53 48 83 EC 30 4C 8B 11 41 B9 ? ? ? ? 49 83 CA 01 0F B6 C2 80 FA 06 48 8B D9 44 0F 45 C8` |
| `SetViewAngle` | `raw` | `0x7FFE5B8D3440` | `0xAE3440` | `85 D2 75 3D 48 63 81 ? ? ? ?` |
| `SetupCmd` | `raw` | `0x7FFE5B6A97B0` | `0x8B97B0` | `48 83 EC 28 E8 ? ? ? ? 8B 80` |
| `SetupMove` | `raw` | `0x7FFE5BB0A960` | `0xD1A960` | `48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 83 EC ? 48 8B EA 4C 8B F1 E8 ? ? ? ? 48 8D 15` |
| `SetupMovementMoves` | `raw` | `0x7FFE5BF7450F` | `0x118450F` | `48 8B ? E8 ? ? ? ? 48 8B 5C 24 ? 48 8B 6C 24 ? 48 83 C4 30` |
| `SomeTimingFromPawn` | `raw` | `0x7FFE5B845A10` | `0xA55A10` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 49 63 D8 48 8B F1` |
| `Spawner_PerTickOrchestrator` | `raw` | `0x7FFE5B9B2540` | `0xBC2540` | `48 8B C4 55 53 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 80 B9 B1 13 00 00 00` |
| `SpectatorInput` | `raw` | `0x7FFE5B5C91D0` | `0x7D91D0` | `48 89 5C 24 10 55 56 57 41 56 41 57 48 8B EC 48 83 EC 60 48 8B 01 41 8B F8 48 8B DA 48 8B F1 FF` |
| `TestSurfaces` | `raw` | `0x7FFE5B5F6370` | `0x806370` | `40 53 57 41 56 48 83 EC 50 8B` |
| `TracePlayerBBox` | `raw` | `0x7FFE5B95F590` | `0xB6F590` | `48 89 5C 24 ? 55 57 41 54 41 55 41 56` |
| `TraceShape` | `raw` | `0x7FFE5B77D200` | `0x98D200` | `48 89 5C 24 ? 48 89 4C 24 ? 55 57` |
| `TraceShape_Client` | `raw` | `0x7FFE5B77D200` | `0x98D200` | `48 89 5C 24 20 48 89 4C 24 08 55 57 41 54 41 55 41 56 48 8D AC 24 10 E0 FF FF B8 F0 20 00 00` |
| `TraceToExit` | `raw` | `0x7FFE5B5F44E0` | `0x8044E0` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? F2 0F 10 02` |
| `UpdatePostProcessing` | `raw` | `0x7FFE5BD0F7A0` | `0xF1F7A0` | `48 85 D2 0F 84 ? ? ? ? 48 89 5C 24 08 57 48 83 EC 60 80` |
| `UpdateSubClass` | `raw` | `0x7FFE5AFEA93B` | `0x1FA93B` | `48 8B 41 10 48 8B D9 8B 50 30` |
| `UpdateTurningInAccuracy` | `rel32` | `0x7FFE5B59FDB0` | `0x7AFDB0` | `E8 ? ? ? ? F3 0F 10 87 ? ? ? ? 44 0F 2F C8` |
| `VPhys2World_ptr` | `riprel` | `0x7FFE5CE1D648` | `0x202D648` | `4C 8B 25 ? ? ? ? 24` |
| `ViewModelHideZoomed` | `raw` | `0x7FFE5B590470` | `0x7A0470` | `48 89 5C 24 20 55 56 57 41 54 41 56 48 8B EC 48 83 EC 50 48 8D 05` |
| `ViewRender_ptr` | `riprel` | `0x7FFE5D11DBB8` | `0x232DBB8` | `48 89 05 ? ? ? ? 48 8B C8 48 85 C0` |
| `WeaponC4_ptr` | `riprel` | `0x7FFE5D096C68` | `0x22A6C68` | `48 8B 15 ? ? ? ? 48 8B 5C 24 ? FF C0 89 05 ? ? ? ? 48 8B C6 48 89 34 EA 80 BE` |
| `WriteSubtickFromEntry` | `raw` | `0x7FFE5BA43CE0` | `0xC53CE0` | `48 89 5C 24 ? 55 57 41 56 48 8D 6C 24 ? 48 81 EC B0 00 00 00 8B 01 48 8B F9 81 4A 10 00 02` |
| `create_move_v2` | `raw` | `0x7FFE5B8BA880` | `0xACA880` | `85 D2 0F 85 ? ? ? ? 48 8B C4 44 88 40` |
| `draw_smoke_array` | `raw` | `0x7FFE5BA68D80` | `0xC78D80` | `40 55 41 54 41 55 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 4C 8B E2` |
| `draw_view_punch_v2` | `raw` | `0x7FFE5B5F3DA0` | `0x803DA0` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 49 8B E9 49 8B F8` |
| `entity_list_ptr` | `riprel` | `0x7FFE5D2BED68` | `0x24CED68` | `48 8B 1D ? ? ? ? 48 8D 46` |
| `frame_stage_notify` | `raw` | `0x7FFE5B8C1491` | `0xAD1491` | `4C 8B 0D ? ? ? ? 48 8D 15 ? ? ? ? 48 8B 8F ? ? ? ? F3 41 0F 10 51 ? 45 8B 49 ? 0F 5A D2 66 49 0F 7E D0 FF 15 ? ? ? ? 48 8B 97 ? ? ? ? 48 8B 0D ? ? ? ? E8 ? ? ? ? E9` |
| `get_fov` | `raw` | `0x7FFE5B5F3DA0` | `0x803DA0` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 49 8B E9 49 8B F8` |
| `get_map_name` | `raw` | `0x7FFE5BCCAD70` | `0xEDAD70` | `48 83 EC ? 48 8B 0D ? ? ? ? ? ? ? FF 90 ? ? ? ? 48 8B C8 48 83 C4` |
| `get_view_angles_v2` | `raw` | `0x7FFE5B8C2D60` | `0xAD2D60` | `4D 85 C0 74 ? 85 D2 74` |
| `get_view_model` | `raw` | `0x7FFE5B63E020` | `0x84E020` | `40 55 53 56 41 56 41 57 48 8B EC` |
| `global_vars_v2` | `riprel` | `0x7FFE5D118E38` | `0x2328E38` | `48 89 1D ? ? ? ? FF 15 ? ? ? ? 84 C0 74 ? 8B 0D ? ? ? ? 4C 8D 0D ? ? ? ? 4C 8D 05 ? ? ? ? BA ? ? ? ? FF 15 ? ? ? ? 48 8B 74 24 ? 48 8B C3` |
| `is_demo_or_hltv` | `raw` | `0x7FFE5BCEC230` | `0xEFC230` | `48 83 EC ? 48 8B 0D ? ? ? ? ? ? ? FF 90 ? ? ? ? 84 C0 75 ? 38 05` |
| `level_init_v2` | `raw` | `0x7FFE5B8E90F0` | `0xAF90F0` | `40 55 56 41 56 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 0D` |
| `level_shutdown` | `raw` | `0x7FFE5B8E9370` | `0xAF9370` | `48 83 EC ? 48 8B 0D ? ? ? ? 48 8D 15 ? ? ? ? 45 33 C9 45 33 C0 ? ? ? FF 50 ? 48 85 C0 74 ? 48 8B 0D ? ? ? ? 48 8B D0 ? ? ? 41 FF 50 ? 48 83 C4` |
| `local_controller` | `riprel` | `0x7FFE5D0F84C0` | `0x23084C0` | `48 8B 05 ? ? ? ? 41 89 BE` |
| `mark_interp_latch_flags_dirty` | `raw` | `0x7FFE5B008070` | `0x218070` | `40 53 56 57 48 83 EC ? 80 3D ? ? ? ? 00` |
| `on_add_entity_v2` | `raw` | `0x7FFE5B7573A0` | `0x9673A0` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 8B 81 ? ? ? ? 49 8B F0` |
| `override_view_short` | `raw` | `0x7FFE5BA4D1F0` | `0xC5D1F0` | `40 57 48 83 EC ? 48 8B FA E8 ? ? ? ? BA` |
| `paintkit_prefab` | `stringref` | `0x7FFE5BE4AC30` | `0x105AC30` | `"set item texture prefab"` |
| `paintkit_seed` | `stringref` | `0x7FFE5BCDEBB0` | `0xEEEBB0` | `"set item texture seed"` |
| `paintkit_wear` | `stringref` | `0x7FFE5BCDEBB0` | `0xEEEBB0` | `"set item texture wear"` |
| `planted_c4_ptr` | `riprel` | `0x7FFE5D096C68` | `0x22A6C68` | `48 8B 15 ? ? ? ? 48 8B 5C 24 ? FF C0 89 05 ? ? ? ? 48 8B C6 ? ? ? ? 80 BE ? ? ? ? 00` |
| `remove_legs` | `raw` | `0x7FFE5BEDDC90` | `0x10EDC90` | `40 55 53 56 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? F2 0F 10 42` |
| `statTrak_killEater` | `stringref` | `0x7FFE5BCDEBB0` | `0xEEEBB0` | `"kill eater"` |
| `statTrak_scoreType` | `stringref` | `0x7FFE5AF0B7F0` | `0x11B7F0` | `"kill eater score type"` |
| `unlock_inventory` | `raw` | `0x7FFE5B4F11C0` | `0x7011C0` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B E9 48 8B 0D ? ? ? ? ? ? ? FF 50` |
| `update_global_vars` | `raw` | `0x7FFE5B8D2E90` | `0xAE2E90` | `48 8B 0D ? ? ? ? 4C 8D 05 ? ? ? ? 48 85 D2` |
| `update_post_processing_v2` | `raw` | `0x7FFE5BD13D56` | `0xF23D56` | `48 89 AC 24 ? ? ? ? 45 33 ED` |
| `view_matrix_ptr` | `riprel` | `0x7FFE5D11E9C0` | `0x232E9C0` | `48 8D 0D ? ? ? ? 48 89 44 24 ? 48 89 4C 24 ? 4C 8D 0D` |

## `engine2.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `BuildNumber_addr` | `riprel` | `0x7FFE8F1ACC74` | `0x60CC74` | `89 05 ? ? ? ? 48 8D 0D ? ? ? ? FF 15 ? ? ? ? 48 8B 0D` |
| `CCommand_Tokenize` | `raw` | `0x7FFE8EF9D710` | `0x3FD710` | `48 89 6C 24 20 4C 89 44 24 18 56 57 41 54 41 56 41 57 48 83 EC 70 48 8B F2 49 8B E8 8B 51 08 4C` |
| `CGameClient_ClientCommand` | `raw` | `0x7FFE8EC41240` | `0xA1240` | `48 8B C4 4C 89 40 18 4C 89 48 20 55 53 57 48 8D 68 A1 48 81 EC C0 00 00 00 33 FF 48 63 DA 48 39` |
| `CHLTVClient_ExecuteStringCommand` | `raw` | `0x7FFE8ECC0D70` | `0x120D70` | `40 53 56 48 81 EC 48 07 00 00 48 8B F1 48 8B DA 48 8B 4A 48 48 83 E1 FC 48 83 79 18 0F 76 03 48` |
| `CSplitScreenSlot` | `stringref` | `0x7FFE8EDEA250` | `0x24A250` | `"CSplitScreenSlot"` |
| `Cvar_RegisterConCommand` | `raw` | `0x7FFE8EF9D270` | `0x3FD270` | `48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 60 44 8B 15 ? ? ? ? 48 8B D9 65 48` |
| `Cvar_RegisterConVar` | `raw` | `0x7FFE8EF9C080` | `0x3FC080` | `48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 48 89 7C 24 20 41 54 41 56 41 57 48 81 EC D0 00 00` |
| `Engine::GetScreenAspectRatio` | `raw` | `0x7FFE8EC169D0` | `0x769D0` | `48 89 5C 24 08 57 48 83 EC 20 8B FA 48 8D 0D` |
| `Engine::PVSManager_ptr` | `riprel` | `0x7FFE8F1B33F0` | `0x6133F0` | `48 8D 0D ? ? ? ? 33 D2 FF 50` |
| `Engine::RunPrediction` | `raw` | `0x7FFE8EC06490` | `0x66490` | `40 55 41 56 48 83 EC ? 80 B9` |
| `Engine_Disconnect_main` | `raw` | `0x7FFE8ED71510` | `0x1D1510` | `48 89 5C 24 20 55 57 41 54 48 8B EC 48 83 EC 70 45 33 E4 48 C7 05` |
| `Engine_HLTVClient_ExecuteStringCommand` | `raw` | `0x7FFE8ECC0D70` | `0x120D70` | `40 53 56 48 81 EC 48 07 00 00 48 8B F1 48 8B DA 48 8B 4A 48 48 83 E1 FC 48 83 79 18 0F 76 03 48` |
| `Engine_HostStateMgr_QueueNewRequest` | `raw` | `0x7FFE8EDBAFC0` | `0x21AFC0` | `48 89 6C 24 18 48 89 7C 24 20 41 56 48 83 EC 30 48 8B EA 48 8B F9 8B 0D ? ? ? ? BA 02 00 00` |
| `Engine_HostStateMgr_QueueNewRequest` | `raw` | `0x7FFE8EDBAFC0` | `0x21AFC0` | `48 89 6C 24 18 48 89 7C 24 20 41 56 48 83 EC 30 48 8B EA 48 8B F9 8B 0D ? ? ? ? BA 02 00 00` |
| `Engine_LoadGameInfo` | `raw` | `0x7FFE8ED2D760` | `0x18D760` | `40 55 56 41 56 48 8D 6C 24 F0 48 81 EC 10 01 00 00 4C 8B F1 C7 44 24 40 00 00 00 00 48 8B CA 48` |
| `Engine_MountAddon` | `raw` | `0x7FFE8ED33440` | `0x193440` | `48 85 D2 0F 84 DA 0A 00 00 48 8B C4 44 88 40 18 55 57 41 54 41 57 48 8D A8 C8 FC FF FF 48 81 EC` |
| `Engine_NetTimeoutDisconnect` | `raw` | `0x7FFE8EC09780` | `0x69780` | `40 53 55 56 57 41 56 48 81 EC 80 00 00 00 0F 29 74 24 70 49 8B F8` |
| `Engine_NetworkGameClient_Connect` | `raw` | `0x7FFE8EC1F400` | `0x7F400` | `48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 40 44 89 81 3C 02 00 00 49 8B E9 44 8B` |
| `Engine_NetworkGameClient_SetSignonState` | `raw` | `0x7FFE8EC00F80` | `0x60F80` | `44 89 44 24 18 89 54 24 10 55 53 56 57 41 55 41 56 41 57 48 8D 6C 24 D9 48 81 EC D0 00 00 00 8B` |
| `Engine_RegisterConCommand` | `raw` | `0x7FFE8EF9D270` | `0x3FD270` | `48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 60 44 8B 15` |
| `Engine_RegisterConVar` | `raw` | `0x7FFE8EF9C080` | `0x3FC080` | `48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 48 89 7C 24 20 41 54 41 56 41 57 48 81 EC D0 00 00` |
| `NetworkGameClient_ptr` | `riprel` | `0x7FFE8F4AA0C0` | `0x90A0C0` | `48 89 3D ? ? ? ? FF 87` |
| `WindowHeight_addr` | `riprel` | `0x7FFE8F4AE4EC` | `0x90E4EC` | `8B 05 ? ? ? ? 89 03` |
| `WindowWidth_addr` | `riprel` | `0x7FFE8F4AE4E8` | `0x90E4E8` | `8B 05 ? ? ? ? 89 07` |

## `filesystem_stdio.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `FullFileSystem_ptr` | `riprel` | `0x7FFE8E7657A0` | `0x2157A0` | `8B 41 28 C3 CC CC CC CC CC CC CC CC CC CC CC CC 48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 48 8D 05 ? ? ? ? C3` |

## `inputsystem.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `InputSystemSvc_ptr` | `riprel` | `0x7FFEFF072B50` | `0x42B50` | `48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 40 53 48 83 EC 20 33 DB` |
| `InputSystem_ptr` | `riprel` | `0x7FFEFF072B50` | `0x42B50` | `48 89 05 ? ? ? ? 33 C0` |

## `matchmaking.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `GameTypes_ptr` | `riprel` | `0x7FFE6F150F80` | `0x1B0F80` | `48 8D 0D ? ? ? ? FF 90` |

## `materialsystem2.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `CMaterial2_CompileComboAndGetVariables_DynamicShaderCompile` | `stringref` | `0x7FFEBE7E3FA0` | `0x13FA0` | `"CompileComboAndGetVariables_DynamicShaderCompile(), C:\buildworker\csgo_rel_win64\build\src\materialsystem2\material2.cpp:2786"` |
| `CMaterial2_GetMode` | `raw` | `0x7FFEBE7DBD40` | `0xBD40` | `48 89 5C 24 18 57 48 83 EC 30 8B 02 48 8B D9 39 05 ? ? ? ? 48 8B 0D ? ? ? ? 48 89 74 24` |
| `CMaterial2_GetVertexShaderInputSignature` | `raw` | `0x7FFEBE7DC8C0` | `0xC8C0` | `48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 48 89 7C 24 20 41 56 48 83 EC 30 F6 41 0B 01 4C 8B` |
| `CMaterial2_LoadShadersAndSetupModes` | `raw` | `0x7FFEBE7E0040` | `0x10040` | `44 89 44 24 18 48 89 54 24 10 53 56 41 54 41 55 48 81 EC 88 00 00 00 4C 8B E9 48 C7 44 24 60` |
| `CMaterialLayer_ApplyMaterialVarsForBatch` | `raw` | `0x7FFEBE7E8B80` | `0x18B80` | `4C 89 4C 24 20 4C 89 44 24 18 48 89 54 24 10 53 55 56 57 41 54 41 55 41 56 41 57 48 83 EC 78` |
| `CMaterialLayer_BuildPassCommandData` | `raw` | `0x7FFEBE7E8F80` | `0x18F80` | `89 54 24 10 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 58 FE FF FF 48 81 EC A8 02 00 00` |
| `CMaterialLayer_ComputeWorkItemsToSetupStaticCombosForMode` | `stringref` | `0x7FFEBE7E5F3C` | `0x15F3C` | `"CMaterialLayer::ComputeWorkItemsToSetupStaticCombosForMode(3154): Failed call to FindOrLoadStaticComboData()!\n"` |
| `CMaterialLayer_CreateCommandBuffer` | `stringref` | `0x7FFEBE7E9820` | `0x19820` | `"\nCMaterialLayer::CreateCommandBuffer(4446): Find a graphics programmer! Trying to bind a "%s" shader that doesn't exist! for %s\n"` |
| `CMaterialSystem2_BindIdentityInstanceIDBufferAndSetRenderState` | `stringref` | `0x7FFEBE840000` | `0x70000` | `"BindIdentityInstanceIDBufferAndSetRenderState: GetMode == NULL? Can't Render\n"` |
| `CMaterialSystem2_DynamicShaderCompile_UnloadAllMaterials` | `stringref` | `0x7FFEBE809AA0` | `0x39AA0` | `"CMaterialSystem2::DynamicShaderCompile_UnloadAllMaterials(1084): ERROR!!! Shaders not freed before shader reload! (See spew above)\n\n"` |
| `CMaterialSystem2_FrameUpdate` | `raw` | `0x7FFEBE80BAC0` | `0x3BAC0` | `48 89 4C 24 08 55 53 56 57 41 54 41 56 48 8B EC 48 83 EC 68 48 8D 05 ? ? ? ? 48 C7 45 C0` |
| `CMaterialSystem2_GetErrorMaterial` | `stringref` | `0x7FFEBE7E74D7` | `0x174D7` | `"CMaterialSystem2::GetErrorMaterial(529): GetErrorMaterial() called when m_pMaterialTypeManager == NULL!\n"` |
| `CMaterialSystem2_Init` | `stringref` | `0x7FFEBE806E40` | `0x36E40` | `"MaterialSystem2"` |
| `CMaterial_SetVariableAndRenderState` | `stringref` | `0x7FFEBE7FF9B0` | `0x2F9B0` | `"SetRenderStateValueFromVariable(1172): Unsupported render state type in material "%s"!\n"` |
| `CVfxProgramData_FindOrCreateStaticComboDataInCache` | `stringref` | `0x7FFEBE87E0E0` | `0xAE0E0` | `"CVfxProgramData::FindOrCreateStaticComboDataInCache(4448): Error! Ref count !=0 for static combo data cache entry!\n"` |
| `FindParameter` | `raw` | `0x7FFEBE7E1E30` | `0x11E30` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B 59 20 48` |
| `MatSys::PrepareSceneMaterial` | `raw` | `0x7FFEBE7E1BE0` | `0x11BE0` | `48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 30 48 8B 59 ? 48 8B F2 48 63 79 ? 48 C1 E7 06` |
| `UpdateParameter` | `raw` | `0x7FFEBE7E2370` | `0x12370` | `48 89 7C 24 ? 41 56 48 83 EC ? 8B 81` |

## `networksystem.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `CNetChan_ProcessMessages` | `raw` | `0x7FFE8C02B280` | `0xBB280` | `48 8B C4 53 57 41 54 41 56 48 81 EC A8 00 00 00 48 89 70 D0 45 33 E4 4C 89 68 C8 48 8B D9 48 89` |
| `CNetChan_SendNetMessage` | `raw` | `0x7FFE8C02D670` | `0xBD670` | `48 89 5C 24 10 48 89 6C 24 18 56 57 41 56 48 83 EC 40 41 0F B6 F0 48 8D 99 F8 73 00 00 4C 8B F2` |
| `CNetworkSystem_Init` | `raw` | `0x7FFE8C05C0C0` | `0xEC0C0` | `40 55 53 57 41 54 41 55 41 57 48 8D AC 24 98 FC FF FF 48 81 EC 68 04 00 00 4C 8B E9` |
| `CNetworkSystem_RegisterNetMessageHandlerAbstract` | `raw` | `0x7FFE8C02BC00` | `0xBBC00` | `48 89 5C 24 10 48 89 6C 24 18 57 41 56 41 57 48 83 EC 50 4C 8B B4 24 90 00 00 00 41 8B D9` |
| `NetSystem_CNetChan_ProcessMessages` | `raw` | `0x7FFE8C02B280` | `0xBB280` | `48 8B C4 53 57 41 54 41 56 48 81 EC A8 00 00 00 48 89 70 D0 45 33 E4 4C 89 68 C8 48 8B D9 48 89` |
| `NetSystem_CNetChan_SendNetMessage` | `raw` | `0x7FFE8C02D670` | `0xBD670` | `48 89 5C 24 10 48 89 6C 24 18 56 57 41 56 48 83 EC 40 41 0F B6 F0 48 8D 99 F8 73 00 00 4C 8B F2` |
| `NetworkSystem_ptr` | `riprel` | `0x7FFE8C1F6E50` | `0x286E50` | `48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 48 83 EC 28 BA FF FF FF` |

## `particles.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `Particles::DrawArray` | `raw` | `0x7FFE70F320B0` | `0x220B0` | `40 55 53 56 57 48 8D 6C 24` |
| `Particles::FindKeyVar` | `raw` | `0x7FFE70F4A650` | `0x3A650` | `48 89 5C 24 ? 57 48 81 EC ? ? ? ? 33 C0 8B DA` |
| `Particles::SetMaterialShaderType` | `raw` | `0x7FFE70FAD8D0` | `0x9D8D0` | `48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 54 41 56 41 57 48 81 EC ? ? ? ? 4C 63 32` |

## `rendersystemdx11.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `CRenderDeviceBase_CreateConstantBuffer` | `stringref` | `0x7FFE8E0AF500` | `0x2F500` | `"CRenderDeviceBase::CreateConstantBuffer(1571): "` |
| `CRenderDeviceDx11_BeginSubmittingDisplayLists` | `stringref` | `0x7FFE8E0BC4E0` | `0x3C4E0` | `"CRenderDeviceDx11::BeginSubmittingDisplayLists(1162): "` |
| `CRenderDeviceDx11_CompileShaderSourceMain` | `stringref` | `0x7FFE8E0BFAF0` | `0x3FAF0` | `"Shader compilation failed! Reported no errors.\n"` |
| `CSwapChainDx11_QueuePresentAndWait` | `raw` | `0x7FFE8E0B4650` | `0x34650` | `40 55 53 57 41 54 41 55 48 8D 6C 24 C9 48 81 EC C0 00 00 00 48 8D 05 ? ? ? ? 4C 89 B4 24` |
| `CSwapChainDx11_ResizeBuffers` | `raw` | `0x7FFE8E0BDD20` | `0x3DD20` | `48 8B C4 55 53 56 57 41 54 48 8B EC 48 83 EC 70 4C 89 68 10 4D 8B E0 4C 89 70 18 4C 8B EA 4C 89` |
| `RenderDeviceMgr_ptr` | `riprel` | `0x7FFE8E4AB530` | `0x42B530` | `8B 5C 24 38 48 83 C4 20 5E C3 CC CC CC CC CC CC 48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 48 8D 05 ? ? ? ? C3` |
| `RenderSystemDx11_QueuePresentAndWait` | `raw` | `0x7FFE8E0B4650` | `0x34650` | `40 55 53 57 41 54 41 55 48 8D 6C 24 C9 48 81 EC C0 00 00 00 48 8D 05 ? ? ? ? 4C 89 B4 24` |
| `RenderSystemDx11_SetHardwareGammaRamp` | `raw` | `0x7FFE8E0BF790` | `0x3F790` | `48 89 5C 24 18 57 B8 B0 40 00 00 E8 ? ? ? ? 48 2B E0 0F 29 BC 24 90 40 00 00 0F 57 C9 0F 28` |
| `RenderSystemDx11_SetMode` | `raw` | `0x7FFE8E0B99E0` | `0x399E0` | `44 89 4C 24 20 44 89 44 24 18 89 54 24 10 55 53 56 57 41 54 41 55 41 56 41 57 48 81 EC D8 02 00` |

## `resourcesystem.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `ResourceSystem_BlockingLoadResourceByName` | `raw` | `0x7FFEBFD37360` | `0x17360` | `40 53 55 57 48 81 EC 80 00 00 00 48 8B 01 49 8B E8 48 8B FA 48 8B D9 FF 90 98 01 00 00 83 F8 03` |
| `ResourceSystem_FindOrRegisterResourceByName` | `raw` | `0x7FFEBFD36D80` | `0x16D80` | `48 89 5C 24 18 48 89 74 24 20 57 48 81 EC A0 00 00 00 F7 02 FF FF FF 3F 41 0F B6 F8 48 8B DA 48` |
| `ResourceSystem_FrameUpdate` | `raw` | `0x7FFEBFD3C010` | `0x1C010` | `44 88 4C 24 20 44 89 44 24 18 89 54 24 10 55 56 41 54 41 55 41 56 48 8D 6C 24 A0 48 81 EC 60 01` |

## `scenesystem.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `CSceneAnimatableObject::GeneratePrimitives` | `raw` | `0x7FFE86A933F0` | `0x733F0` | `48 8B C4 48 89 58 08 48 89 50 10 55 56 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ?` |
| `CSceneSkyBoxObject_DrawSkyboxArray` | `raw` | `0x7FFE86B6FA60` | `0x14FA60` | `45 85 C9 0F 8E ? ? ? ? 4C 8B DC 55 41 56 49 8D AB 58 FC FF FF 48 81 EC 98 04 00 00` |
| `CSceneSystem_CreateStaticShape` | `raw` | `0x7FFE86AD19C0` | `0xB19C0` | `48 8B C4 48 89 48 08 55 41 54 41 56 48 8D 68 D8 48 81 EC 10 01 00 00 4C 8B 65 50 48 8D 4D 80` |
| `CSceneSystem_InitGfxObjects` | `raw` | `0x7FFE86AD3D00` | `0xB3D00` | `40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 08 FE FF FF 48 81 EC F8 02 00 00` |
| `CSceneSystem_RenderViewLayer_Dispatch` | `raw` | `0x7FFE86B0DC50` | `0xEDC50` | `48 8B C4 48 89 48 08 55 53 56 57 41 54 41 55 41 56 41 57 48 8D A8 B8 FE FF FF 48 81 EC 08 02 00` |
| `CSceneSystem_Thread_CullView` | `stringref` | `0x7FFE86B091C0` | `0xE91C0` | `"CSceneSystem::Thread_CullView(), C:\buildworker\csgo_rel_win64\build\src\scenesystem\scenesystem.cpp:3312"` |
| `DrawObject_legacy` | `raw` | `0x7FFE86A75AC0` | `0x55AC0` | `48 8B C4 53 57 41 54 48 81 EC D0 00 00 00 49 63 F9 49` |
| `DrawSkyboxArray` | `raw` | `0x7FFE86B6FA60` | `0x14FA60` | `45 85 C9 0F 8E ? ? ? ? 4C 8B DC 55` |
| `SceneSystem::DrawAggeregateObject` | `raw` | `0x7FFE86B4CE20` | `0x12CE20` | `48 8B C4 4C 89 48 20 4C 89 40 ? 48 89 50 ? 55 53 41 57 48 8D A8` |
| `SceneSystem::DrawArrayLight` | `raw` | `0x7FFE86A9A990` | `0x7A990` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 54 24` |
| `SceneSystem_Thread_RenderSceneDrawList` | `raw` | `0x7FFE86B0D900` | `0xED900` | `40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 E1 48 81 EC D8 00 00 00 4C 8B 71 28 48 8B D9` |
| `SceneSystem_ptr` | `riprel` | `0x7FFE872FB490` | `0x8DB490` | `48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 48 8D 0D ? ? ? ? E9` |

## `schemasystem.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `CSchemaSystem_InstallSchemaBindings` | `raw` | `0x7FFEBEB875D0` | `0x375D0` | `40 53 48 83 EC 20 48 8B DA 48 8B D1 48 8D 0D ? ? ? ? E8 ? ? ? ? 85 C0 74 08 32 C0` |
| `CSchemaSystem_RegisterModuleAndBuiltins` | `raw` | `0x7FFEBEB606F0` | `0x106F0` | `48 89 54 24 10 53 56 57 41 55 41 56 41 57 48 83 EC 48 45 33 ED 49 63 C0 33 FF 44 89 AC 24 90 00` |
| `CSchemaSystem_VerifySchemaBindingConsistency` | `raw` | `0x7FFEBEB558F0` | `0x58F0` | `88 54 24 10 55 53 57 41 54 41 55 48 8B EC 48 81 EC 80 00 00 00 65 48 8B 04 25 58 00 00 00` |
| `SchemaSystem_ptr` | `riprel` | `0x7FFEBEBC6800` | `0x76800` | `48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 48 89 5C 24 08 48 89 74` |

## `soundsystem.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `SoundSystem::PlayVSound` | `raw` | `0x7FFE8AB39840` | `0x349840` | `48 8B C4 48 89 58 08 57 48 81 EC ? ? ? ? 33 FF 48 8B D9` |
| `SoundSystem::SomeUtlSymbolFunc` | `raw` | `0x7FFE8A8A0740` | `0xB0740` | `48 89 74 24 18 57 48 83 EC 20 48 63 F2 48 8B F9 3B 71 30` |
| `SoundSystem_ptr` | `riprel` | `0x7FFE8AD02360` | `0x512360` | `48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 48 89 15` |

## `tier0.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `CVar_ptr` | `riprel` | `0x7FFEBE0D93B0` | `0x3A93B0` | `48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC E9` |
| `LoadKV3` | `raw` | `0x7FFEBDE59090` | `0x129090` | `48 89 5C 24 08 57 48 83 EC 70 4C 8B D1 48 C7 C0 FF FF FF FF 48 FF C0 41 80 3C 00 00 75 F6` |
| `Tier0::LoadKeyValues` | `rel32` | `0x7FFEBDE59160` | `0x129160` | `E8 ? ? ? ? 8B 4C 24 34 0F B6 D8` |
| `Tier0::UtlBuffer` | `raw` | `0x7FFEBDD83F10` | `0x53F10` | `48 89 5C 24 ? 57 48 83 EC ? 8B 41 ? 8D 7A` |

## `vphysics2.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `VPhysics2_Startup` | `raw` | `0x7FFE8AEEAF20` | `0x6AF20` | `48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 48 89 7C 24 20 41 54 41 56 41 57 48 83 EC 70 48 83 3D` |

