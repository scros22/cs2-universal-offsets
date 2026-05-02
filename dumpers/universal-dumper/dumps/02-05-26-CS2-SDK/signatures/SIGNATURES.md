# CS2 Signatures

_This file is regenerated on every successful run of `cs2-sdk`._

**354/400 signatures resolved across 16 module(s).**

## `animationsystem.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `Animation::ShouldUpdateSequences` | `raw` | `0x7FFF189DF0A0` | `0x14F0A0` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 49 8B 40 48` |
| `AnimationSystemUtils_ptr` | `riprel` | `0x7FFF190A2170` | `0x812170` | `48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 48 83 EC 28 48 8B CA 48 8D 15` |

## `client.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `AddNametagEntity` | `raw` | `0x7FFF0153B070` | `0x78B070` | `40 55 53 56 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B DA` |
| `AddStattrakEntity` | `raw` | `0x7FFF017FC790` | `0xA4C790` | `48 8B C4 48 89 58 08 48 89 70 10 57 48 83 EC 50 33 F6 8B FA 48 8B D1` |
| `AutowallInit` | `raw` | `0x7FFF01691EE0` | `0x8E1EE0` | `40 53 48 83 EC ? 48 8B D9 48 81 C1 ? ? ? ? E8 ? ? ? ?` |
| `AutowallTraceData` | `raw` | `0x7FFF0173E9C0` | `0x98E9C0` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B 09` |
| `AutowallTracePos` | `raw` | `0x7FFF015B7780` | `0x807780` | `40 55 56 41 54 41 55 41 57 48 8B EC` |
| `BulkRegenIterator` | `raw` | `0x7FFF0153E571` | `0x78E571` | `57 48 83 EC 40 0F B6 F9 E8 ? ? ? ? 48 85 C0 0F 84` |
| `CAM_ThinkReturn` | `raw` | `0x7FFF010CA4FF` | `0x31A4FF` | `BA 04 00 00 00 FF 15 ? ? ? ? 84 C0 0F 84` |
| `CAttributeStringFill` | `rel32` | `0x7FFF01C5EC20` | `0xEAEC20` | `E8 ? ? ? ? 41 83 CF 08` |
| `CAttributeStringInit` | `rel32` | `0x7FFF013A86B0` | `0x5F86B0` | `E8 ? ? ? ? 48 8D 05 ? ? ? ? 48 89 7D ? 48 89 45 ? 49 8D 4F` |
| `CBaseEntity_TakeDamageOld` | `raw` | `0x7FFF00FD3D20` | `0x223D20` | `40 55 53 56 57 41 54 48 8D 6C 24 E0 48 81 EC 20 01 00 00 4D 8B E0 48 8B FA 48 8B F1 E8` |
| `CBaseModelEntity_SetBodygroup` | `raw` | `0x7FFF01689E70` | `0x8D9E70` | `85 D2 0F 88 CB 01 00 00 55 53 56 41 56 48 8B EC 48 83 EC 78 45 8B F0 8B DA 48 8B F1 E8 ? ? ?` |
| `CBodyComponent` | `stringref` | `0x7FFF00F6C160` | `0x1BC160` | `"CBodyComponent"` |
| `CBodyComponentSkeletonInstance` | `stringref` | `0x7FFF00F73040` | `0x1C3040` | `"CBodyComponentSkeletonInstance"` |
| `CBufferStringInit` | `raw` | `0x7FFF025929D0` | `0x17E29D0` | `48 89 5C 24 ? 57 48 83 EC ? 8B 41 ? 48 8D 79` |
| `CCSGOInput::CreateMove` | `raw` | `0x7FFF01A0E7F0` | `0xC5E7F0` | `48 8B C4 4C 89 40 18 48 89 48 08 55 53 41 54 41 55` |
| `CCSGameRules` | `stringref` | `0x7FFF00E2E160` | `0x7E160` | `"CCSGameRules"` |
| `CCSGameRulesProxy` | `stringref` | `0x7FFF01499500` | `0x6E9500` | `"CCSGameRulesProxy"` |
| `CCSInventoryManager::EquipItemInLoadout` | `raw` | `0x7FFF01572150` | `0x7C2150` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 89 54 24 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 0F B7 FA` |
| `CCSPlayerController` | `stringref` | `0x7FFF01595220` | `0x7E5220` | `"CCSPlayerController"` |
| `CCSPlayerController` | `stringref` | `0x7FFF01595220` | `0x7E5220` | `"CCSPlayerController"` |
| `CCSPlayerController_ActionTrackingServices` | `stringref` | `0x7FFF01595220` | `0x7E5220` | `"CCSPlayerController_ActionTrackingServices"` |
| `CCSPlayerController_DamageServices` | `stringref` | `0x7FFF01595220` | `0x7E5220` | `"CCSPlayerController_DamageServices"` |
| `CCSPlayerController_InGameMoneyServices` | `stringref` | `0x7FFF01595220` | `0x7E5220` | `"CCSPlayerController_InGameMoneyServices"` |
| `CCSPlayerController_InventoryServices` | `stringref` | `0x7FFF01595220` | `0x7E5220` | `"CCSPlayerController_InventoryServices"` |
| `CCSPlayerInventory::GetItemInLoadout` | `raw` | `0x7FFF01573D70` | `0x7C3D70` | `40 55 48 83 EC ? 49 63 E8` |
| `CCSPlayerPawn` | `stringref` | `0x7FFF01960E40` | `0xBB0E40` | `"CCSPlayerPawn"` |
| `CCSPlayer_BulletServices` | `stringref` | `0x7FFF015C3BA0` | `0x813BA0` | `"CCSPlayer_BulletServices"` |
| `CCSPlayer_BulletServices` | `stringref` | `0x7FFF015C3BA0` | `0x813BA0` | `"CCSPlayer_BulletServices"` |
| `CCSPlayer_CameraServices` | `stringref` | `0x7FFF015BFCB0` | `0x80FCB0` | `"CCSPlayer_CameraServices"` |
| `CCSPlayer_HostageServices` | `stringref` | `0x7FFF015C3BA0` | `0x813BA0` | `"CCSPlayer_HostageServices"` |
| `CCSPlayer_ItemServices` | `stringref` | `0x7FFF01600B00` | `0x850B00` | `"CCSPlayer_ItemServices"` |
| `CCSPlayer_MovementServices` | `stringref` | `0x7FFF015EDE80` | `0x83DE80` | `"CCSPlayer_MovementServices"` |
| `CCSPlayer_MovementServices` | `stringref` | `0x7FFF015EDE80` | `0x83DE80` | `"CCSPlayer_MovementServices"` |
| `CCSPlayer_PingServices` | `stringref` | `0x7FFF01600ED0` | `0x850ED0` | `"CCSPlayer_PingServices"` |
| `CCSPlayer_RunCommand_Context` | `raw` | `0x7FFF0178BAF0` | `0x9DBAF0` | `48 8B C4 48 81 EC C8 00 00 00 48 89 58 10 48 89 68 18 48 8B EA 48 89 70 20 48 8B F1 48 89 78 F8` |
| `CCSPlayer_UseServices` | `stringref` | `0x7FFF016321D0` | `0x8821D0` | `"CCSPlayer_UseServices"` |
| `CCSPlayer_WaterServices` | `stringref` | `0x7FFF01627460` | `0x877460` | `"CCSPlayer_WaterServices"` |
| `CCSPlayer_WeaponServices` | `stringref` | `0x7FFF01627810` | `0x877810` | `"CCSPlayer_WeaponServices"` |
| `CCSPlayer_WeaponServices` | `stringref` | `0x7FFF01627810` | `0x877810` | `"CCSPlayer_WeaponServices"` |
| `CCSWeaponBase` | `stringref` | `0x7FFF0152F3D0` | `0x77F3D0` | `"CCSWeaponBase"` |
| `CCSWeaponBaseGun` | `stringref` | `0x7FFF0152F470` | `0x77F470` | `"CCSWeaponBaseGun"` |
| `CCSWeaponBaseVData` | `stringref` | `0x7FFF0150A2B0` | `0x75A2B0` | `"CCSWeaponBaseVData"` |
| `CCollisionProperty` | `stringref` | `0x7FFF01090F90` | `0x2E0F90` | `"CCollisionProperty"` |
| `CCompositeMaterialManager_AddPanoramaPanelRenderRequest_Caller` | `stringref` | `0x7FFF0216B8D4` | `0x13BB8D4` | `"CCompositeMaterialManager::AddNewPanoramaPanelRenderRequest"` |
| `CDecoyProjectile` | `stringref` | `0x7FFF014FE1E0` | `0x74E1E0` | `"CDecoyProjectile"` |
| `CEconItemSchema::GetAttributeDefinitionByName` | `raw` | `0x7FFF01DFCEA0` | `0x104CEA0` | `48 89 5C 24 10 48 89 6C 24 18 57 41 56 41 57 48 83 EC 60 48 8D 05` |
| `CEconItemView::GetCustomPaintKitIndex` | `raw` | `0x7FFF0166B2F0` | `0x8BB2F0` | `48 89 5C 24 ? 57 48 83 EC ? 8B 15 ? ? ? ? 48 8B F9 65 48 8B 04 25` |
| `CFlashbangProjectile` | `stringref` | `0x7FFF01D903F0` | `0xFE03F0` | `"CFlashbangProjectile"` |
| `CFogController` | `stringref` | `0x7FFF0102EFD0` | `0x27EFD0` | `"CFogController"` |
| `CGameEntitySystem::OnAddEntity` | `raw` | `0x7FFF01718640` | `0x968640` | `48 89 74 24 ? 57 48 83 EC ? 41 B9 ? ? ? ? 41 8B C0 41 23 C1 48 8B F2 41 83 F8 ? 48 8B F9 44 0F 45 C8 41 81 F9 ? ? ? ? 73 ? FF 81` |
| `CGameEntitySystem::OnRemoveEntity` | `raw` | `0x7FFF01718EA0` | `0x968EA0` | `48 89 74 24 ? 57 48 83 EC ? 41 B9 ? ? ? ? 41 8B C0 41 23 C1 48 8B F2 41 83 F8 ? 48 8B F9 44 0F 45 C8 41 81 F9 ? ? ? ? 73 ? FF 89` |
| `CGameSceneNode` | `stringref` | `0x7FFF00F538F0` | `0x1A38F0` | `"CGameSceneNode"` |
| `CGameSceneNode_BuildBoneMergeWork` | `raw` | `0x7FFF016EFA40` | `0x93FA40` | `40 55 56 57 41 54 41 55 41 56 41 57 48 83 EC 50 48 8D 6C 24 50 80 A1 06 01 00 00 FB 4C 8B F9 80` |
| `CGameSceneNode_StartHierarchicalAttachment` | `raw` | `0x7FFF0173C5E0` | `0x98C5E0` | `48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20 57 41 54 41 55 41 56 41 57 48 83 EC 30 48 8B F9 8B` |
| `CGameTrace_TraceShape_Client` | `raw` | `0x7FFF0173EAA0` | `0x98EAA0` | `48 89 5C 24 20 48 89 4C 24 08 55 57 41 54 41 55 41 56 48 8D AC 24 10 E0 FF FF B8 F0 20 00 00` |
| `CGlowProperty` | `stringref` | `0x7FFF010911A0` | `0x2E11A0` | `"CGlowProperty"` |
| `CGlowProperty_OnGlowTypeChanged` | `raw` | `0x7FFF018BCD90` | `0xB0CD90` | `48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 48 8B 05 ? ? ? ? 48 8B D9 F3 0F 10 41 4C` |
| `CHEGrenadeProjectile` | `stringref` | `0x7FFF01D90490` | `0xFE0490` | `"CHEGrenadeProjectile"` |
| `CInputPtrGlobal` | `riprel` | `0x7FFF02E14330` | `0x2064330` | `4C 8B 05 ? ? ? ? 41 8B 80 50 0B 00 00 85 C0` |
| `CMolotovProjectile` | `stringref` | `0x7FFF014FE3C0` | `0x74E3C0` | `"CMolotovProjectile"` |
| `CPaintKitDefinitions_FindOrCreateByName` | `stringref` | `0x7FFF01E0A690` | `0x105A690` | `"Kit "[%s]" specified, but doesn't exist!! You're probably missing an entry in items_paintkits.txt or items_stickerkits.txt or need to run with -use_local_item_data\n"` |
| `CPaintKitDefinitions_LoadDefaultKit` | `stringref` | `0x7FFF01DDC760` | `0x102C760` | `"Unable to find "default" paint kit in "paint_kits_rarity""` |
| `CPostProcessingVolume` | `stringref` | `0x7FFF01053D60` | `0x2A3D60` | `"CPostProcessingVolume"` |
| `CS2ItemEditor_BuildTemplateMaterialFromFile` | `raw` | `0x7FFF0216CA50` | `0x13BCA50` | `48 89 54 24 10 55 53 41 55 41 57 48 8D AC 24 18 F9 FF FF 48 81 EC E8 07 00 00 4C 8B FA 48 85 D2` |
| `CSBaseGunFireData_fn` | `raw` | `0x7FFF02298140` | `0x14E8140` | `48 8B C4 55 53 56 57 41 54 41 55 41 56 41 57 48 8D 68 A8 48 81 EC ? ? ? ? 4C 8B 69` |
| `CSGOInput_ptr` | `riprel` | `0x7FFF02E14330` | `0x2064330` | `48 8B 0D ? ? ? ? 4C 8B C6 8B 10 E8` |
| `CSGOInput_resolved` | `riprel` | `0x7FFF02E14337` | `0x2064337` | `48 8B 0D ? ? ? ? 8B 10 E8 ? ? ? ? 45 32 FF` |
| `CSkeletonInstance` | `stringref` | `0x7FFF00F53A20` | `0x1A3A20` | `"CSkeletonInstance"` |
| `CSkeletonInstance::SetMeshGroupMask` | `raw` | `0x7FFF017DDB50` | `0xA2DB50` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8D 99` |
| `CSkeletonInstance_GetTransformsForHitboxList` | `raw` | `0x7FFF017CA6C0` | `0xA1A6C0` | `48 89 5C 24 18 55 56 57 41 55 41 57 48 81 EC A0 00 00 00 49 63 28 4D 8B F8 48 8B FA 48 8B D9 85` |
| `CSkeletonInstance_OnBodyGroupChoiceChanged` | `raw` | `0x7FFF017D5310` | `0xA25310` | `48 89 5C 24 08 57 48 83 EC 20 49 63 D8 49 8B F9 45 85 C0 78 20 3B 99 18 02 00 00 7D 18` |
| `CSkeletonInstance_OnSkeletonModelChanged` | `raw` | `0x7FFF017D5520` | `0xA25520` | `49 8B 00 48 89 81 B8 00 00 00 C6 81 B0 00 00 00 01 C3` |
| `CSkeletonInstance_PostDataUpdate` | `raw` | `0x7FFF017D64B0` | `0xA264B0` | `48 8B C4 4C 89 40 18 89 50 10 55 57 48 8D A8 68 FE FF FF 48 81 EC 88 02 00 00 48 89 70 E0 48 8B` |
| `CSkeletonInstance_SetMaterialGroup` | `raw` | `0x7FFF017DC830` | `0xA2C830` | `3B 91 C4 03 00 00 74 24 89 91 C4 03 00 00 48 8B 81 28 02 00 00 48 85 C0 74 12` |
| `CSkeletonInstance_SetMeshGroupMask` | `raw` | `0x7FFF017D5480` | `0xA25480` | `48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 49 8B 00 49 8B F8 48 8B F2 48 8B D9 48 39 81 C8 01` |
| `CSmokeGrenadeProjectile` | `stringref` | `0x7FFF014FE460` | `0x74E460` | `"CSmokeGrenadeProjectile"` |
| `CTonemapController2` | `stringref` | `0x7FFF01007C90` | `0x257C90` | `"CTonemapController2"` |
| `CUtlVector_CompositeMaterialInput_AddToTail` | `raw` | `0x7FFF01539C50` | `0x789CA2` | `41 B9 88 02 00 00 8B 57 14 81 E2 FF FF FF 3F 8D 71 01 44 8B C6 FF 15` |
| `C_AttributeContainer` | `stringref` | `0x7FFF019C8BB0` | `0xC18BB0` | `"C_AttributeContainer"` |
| `C_BaseEntity` | `stringref` | `0x7FFF00DFE260` | `0x4E260` | `"C_BaseEntity"` |
| `C_BaseModelEntity` | `stringref` | `0x7FFF00F08010` | `0x158010` | `"C_BaseModelEntity"` |
| `C_BasePlayerPawn` | `stringref` | `0x7FFF00E1DA20` | `0x6DA20` | `"C_BasePlayerPawn"` |
| `C_C4` | `stringref` | `0x7FFF00E4A420` | `0x9A420` | `"C_C4"` |
| `C_CSPlayerPawn` | `stringref` | `0x7FFF01472430` | `0x6C2430` | `"C_CSPlayerPawn"` |
| `C_CSPlayerPawnBase` | `stringref` | `0x7FFF01987140` | `0xBD7140` | `"C_CSPlayerPawnBase"` |
| `C_CSWeaponBase` | `stringref` | `0x7FFF014F2170` | `0x742170` | `"C_CSWeaponBase"` |
| `C_EconEntity_BuildLegacyGloveSkinMaterial` | `stringref` | `0x7FFF01971460` | `0xBC1460` | `"MapPlayerPreview gloves"` |
| `C_EconEntity_BuildLegacyWeaponSkinMaterial` | `stringref` | `0x7FFF0153C2A0` | `0x78C2A0` | `"workshop preview weapon"` |
| `C_EconEntity_BuildModernWeaponSkinMaterial` | `raw` | `0x7FFF01B34F90` | `0xD84F90` | `48 85 C9 0F 84 ? ? 00 00 48 8B C4 48 89 50 10 48 89 48 08 55 41 54 41 56 41 57 48 8D A8 B8 FA` |
| `C_EconEntity_BuildNametagOverlayMaterial` | `stringref` | `0x7FFF0153B070` | `0x78B070` | `"low-res nametag"` |
| `C_EconItemView` | `stringref` | `0x7FFF014BB570` | `0x70B570` | `"C_EconItemView"` |
| `C_EconWearable_OnNewCustomMaterials` | `stringref` | `0x7FFF01E69090` | `0x10B9090` | `"Invalid EconItemView -- Can't create custom materials for wearable, debug this.\n"` |
| `C_Hostage` | `stringref` | `0x7FFF00E97480` | `0xE7480` | `"C_Hostage"` |
| `C_Inferno` | `stringref` | `0x7FFF00EA7440` | `0xF7440` | `"C_Inferno"` |
| `C_PlantedC4` | `stringref` | `0x7FFF00EA07A0` | `0xF07A0` | `"C_PlantedC4"` |
| `C_SmokeGrenadeProjectile` | `stringref` | `0x7FFF00E45A10` | `0x95A10` | `"C_SmokeGrenadeProjectile"` |
| `CacheParticleEffect` | `raw` | `0x7FFF00FB7BC0` | `0x207BC0` | `4C 8B DC 53 48 81 EC ? ? ? ? F2 0F 10 05` |
| `CalcSpread` | `raw` | `0x7FFF01A2EBF0` | `0xC7EBF0` | `48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 4C 63 EA` |
| `CalcViewmodel` | `raw` | `0x7FFF015FF430` | `0x84F430` | `40 55 53 56 41 56 41 57 48 8B EC` |
| `CalcViewmodelTransform_v2` | `raw` | `0x7FFF015524F0` | `0x7A24F0` | `48 89 5C 24 20 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 80 48 81 EC 80 01 00 00 48 8B FA` |
| `CalcViewmodelView` | `raw` | `0x7FFF01A1BF20` | `0xC6BF20` | `40 53 48 83 EC 60 48 8B 41 08 49 8B D8 8B 48 30 48 C1 E9 0C F6 C1 01 0F 85 48 01 00 00 41 B8 07` |
| `CalculateInterpolation` | `rel32` | `0x7FFF02277E70` | `0x14C7E70` | `E8 ? ? ? ? 8B 45 ? 3B 45 60 75 04 32 D2 EB 09 BA 01 00 00 00 41 0F 4C D5 C0 EA 07 84 D2 0F 85 87` |
| `CalculateWorldSpaceBones` | `raw` | `0x7FFF017BB070` | `0xA0B070` | `48 89 4C 24 ? 55 53 56 57 41 54 41 55 41 56 41 57 B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 48 8D 6C 24 ? 48 8B 81` |
| `ClearHUDWeaponIcon` | `rel32` | `0x7FFF01B9DDD0` | `0xDEDDD0` | `E8 ? ? ? ? 8B F8 C6 84 24 ? ? ? ? ?` |
| `ClientModeCSNormal_OnEvent` | `raw` | `0x7FFF01A0C660` | `0xC5C660` | `40 53 57 48 81 EC 78 02 00 00 48 8B CA 48 8B FA` |
| `ClientMode_ptr` | `riprel` | `0x7FFF030EEAE0` | `0x233EAE0` | `48 8D 0D ? ? ? ? 48 69 C0 ? ? ? ? 48 03 C1 C3 CC CC` |
| `Client_DispatchSpawn` | `raw` | `0x7FFF02285B10` | `0x14D5B10` | `4C 8B DC 55 56 48 83 EC 78 49 8B 68 08 48 8B F1 48 85 ED 0F 84 72 01 00 00 49 89 5B 08 49 8D 4B` |
| `CompositeMaterialPanoramaPanel_Init` | `stringref` | `0x7FFF01941260` | `0xB91260` | `"CompositeMaterialPanoramaPanel_t::Init"` |
| `ConCommand_firstperson` | `raw` | `0x7FFF0187A2B0` | `0xACA2B0` | `48 83 EC 28 48 8B 0D ? ? ? ? 48 8D 54 24 ? 48 8B 01 FF 90 08 03 00 00 83 7C 24 ? 00 75 ? 48 8B 05 ? ? ? ? C6 80 29 02 00 00 00 C7 80 A8 06 00 00 00` |
| `ConCommand_thirdperson` | `raw` | `0x7FFF0187A390` | `0xACA390` | `48 83 EC 38 48 8B 0D ? ? ? ? 48 8D 54 24 ? 48 8B 01 FF 90 08 03 00 00 83 7C 24 ? 00 0F 85 ? ? ? ? 4C 8B 05 ? ? ? ? 41 8B 80 50 0B 00 00` |
| `ConvarGet` | `raw` | `0x7FFF0166F602` | `0x8BF602` | `8B D0 48 8D 0D ? ? ? ? E8 ? ? ? ? 0F 10 45 ? 83 F0 74` |
| `CreateBaseTypeCache` | `raw` | `0x7FFF022C0EA0` | `0x1510EA0` | `40 53 48 83 EC ? 4C 8B 49 ? 44 8B D2` |
| `CreateEntityByClassName` | `raw` | `0x7FFF023B4C46` | `0x1604C46` | `4C 8D 05 ? ? ? ? 4C 8B CF BA 03 00 00 00 FF 15 ? ? ? ? EB ? 0F B7 C8 48` |
| `CreateInterface` | `raw` | `0x7FFF025E5790` | `0x1835790` | `4C 8B 0D ? ? ? ? 4C 8B D2 4C 8B D9 4D 85 C9 74 ? 49 8B 41 08` |
| `CreateNewSubtickMoveStep` | `rel32` | `0x7FFF01261D80` | `0x4B1D80` | `E8 ? ? ? ? 48 8B D0 48 8B CE E8 ? ? ? ? 48 8B C8` |
| `CreateParticleEffect` | `raw` | `0x7FFF01737020` | `0x987020` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? F3 0F 10 1D ? ? ? ? 41 8B F8 8B DA 4C 8D 05` |
| `CreateSOSubclassEconItem` | `raw` | `0x7FFF01DA7770` | `0xFF7770` | `48 83 EC 28 B9 48 00 00 00 E8 ? ? ? ? 48 85` |
| `DestroyParticle` | `raw` | `0x7FFF016F63E0` | `0x9463E0` | `83 FA ? 0F 84 ? ? ? ? 41 54` |
| `DispatchEffect` | `raw` | `0x7FFF0110A570` | `0x35A570` | `48 89 5C 24 ? 57 48 83 EC ? 48 8B F9 48 8B DA 48 8D 4C 24` |
| `DispatchSpawn_caller` | `raw` | `0x7FFF02285B10` | `0x14D5B10` | `4C 8B DC 55 56 48 83 EC 78 49 8B 68 08 48 8B F1 48 85 ED 0F 84 72 01 00 00` |
| `DrawCrosshair` | `raw` | `0x7FFF01560BF0` | `0x7B0BF0` | `48 89 5C 24 08 57 48 83 EC 20 48 8B D9 E8 ? ? ? ? 48 85` |
| `DrawOverHead` | `raw` | `0x7FFF01816CF0` | `0xA66CF0` | `40 53 48 83 EC ? 48 8B D9 83 FA ? 75 ? 48 8B 0D ? ? ? ? 48 8D 54 24 ? 48 8B 01 FF 90 ? ? ? ? 8B 10` |
| `DrawScopeOverlay` | `raw` | `0x7FFF0160D530` | `0x85D530` | `48 8B C4 53 57 48 83 EC ? 48 8B FA` |
| `DrawSmokeVertex` | `raw` | `0x7FFF01A2B290` | `0xC7B290` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 48 8B 9C 24 ? ? ? ? 4D 8B F8` |
| `FX_FireBullets` | `raw` | `0x7FFF01A2E380` | `0xC7E380` | `48 8B C4 4C 89 48 20 48 89 50 10 55 53 57 41 54 41 55 48 8D A8 58 FB FF FF 48 81 EC A0 05` |
| `FX_FireBullets` | `raw` | `0x7FFF01A2E380` | `0xC7E380` | `48 8B C4 4C 89 48 20 48 89 50 10 55 53 57 41 54 41 55 48 8D A8 58 FB FF FF 48 81 EC A0 05 00 00` |
| `FindHudElement` | `raw` | `0x7FFF01B71E98` | `0xDC1E98` | `48 8D 15 ? ? ? ? 45 33 C0 B9 ? ? ? ? FF 15 ? ? ? ? EB ? 48 8B 15` |
| `FindHudElement_panorama` | `raw` | `0x7FFF01B73E70` | `0xDC3E70` | `4C 8B DC 53 48 83 EC 50 48 8B 05` |
| `FindSOCache` | `raw` | `0x7FFF025CF080` | `0x181F080` | `48 89 5C 24 08 57 48 83 EC 30 4C 8B 52 08 48 8B D9 8B 0A` |
| `FirstPersonLegs` | `raw` | `0x7FFF01EA0410` | `0x10F0410` | `40 55 53 56 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? F2 0F 10 42` |
| `FlashOverlay` | `raw` | `0x7FFF01B5B2C0` | `0xDAB2C0` | `85 D2 0F 88 ? ? ? ? 48 89 4C 24` |
| `ForceButtonsDown` | `raw` | `0x7FFF01780130` | `0x9D0130` | `40 53 57 41 56 48 81 EC ? ? ? ? 48 83 79` |
| `GameEntitySystemPtr` | `riprel` | `0x7FFF03281DF0` | `0x24D1DF0` | `48 8B 1D ? ? ? ? 48 89 1D ? ? ? ?` |
| `GameEventManager_AddListener` | `raw` | `0x7FFF016E9FF0` | `0x939FF0` | `48 89 5C 24 10 48 89 6C 24 18 56 57 41 56 48 83 EC 50 41 0F B6 E9 48 8D 99 E0 00 00 00 49 8B F0` |
| `GameEventManager_UnserializeEvent` | `raw` | `0x7FFF01742900` | `0x992900` | `48 8B C4 48 89 50 10 55 41 54 41 55 41 56 48 8D 68 D8 48 81 EC 08 01 00 00 48 89 58 D8 4C 8D B1` |
| `GameRules_ptr` | `riprel` | `0x7FFF030DBFB8` | `0x232BFB8` | `48 8B 1D ? ? ? ? 48 8D 54 24 ? 0F 28 D0 48 8D 4C 24 ?` |
| `GetBBox_ptr` | `riprel` | `0x7FFF030DBFB8` | `0x232BFB8` | `48 8B 0D ? ? ? ? 48 85 C9 74 ? ? ? ? 48 FF A0 ? ? ? ? 48 8D 05` |
| `GetBaseEntity` | `raw` | `0x7FFF01717600` | `0x967600` | `4C 8D 49 ? 81 FA` |
| `GetBonePositionByName` | `raw` | `0x7FFF016781E0` | `0x8C81E0` | `40 53 48 83 EC ? 48 8B 89 ? ? ? ? 48 8B DA 48 8B 01 FF 50 ? 48 8B C8` |
| `GetChatObject` | `rel32` | `0x7FFF01E73670` | `0x10C3670` | `E8 ? ? ? ? 48 8B E8 48 85 C0 0F 84 ? ? ? ? 4C 8D 05` |
| `GetClientSystem` | `rel32` | `0x7FFF01DE6570` | `0x1036570` | `E8 ? ? ? ? 48 8B C8 E8 ? ? ? ? 8B D8 85 C0 74 33` |
| `GetControllerCmd` | `raw` | `0x7FFF0166DC00` | `0x8BDC00` | `40 53 48 83 EC 20 8B DA E8 ? ? ? ? 4C` |
| `GetEconItemSystem` | `raw` | `0x7FFF01129830` | `0x379830` | `48 83 EC 28 48 8B 05 ? ? ? ? 48 85 C0 0F 85 ? ? ? ? 48 89 5C 24` |
| `GetEntityHandle` | `raw` | `0x7FFF016FE8D0` | `0x94E8D0` | `48 85 C9 74 32 48 8B 49 10 48 85 C9 74 29 44 8B 41 10 BA` |
| `GetGlowColor` | `raw` | `0x7FFF018BABC0` | `0xB0ABC0` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B F2 48 8B F9 48 8B 54 24` |
| `GetInstanceS` | `riprel` | `0x7FFF0306A670` | `0x22BA670` | `48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 8B 91 ? ? ? ? B8` |
| `GetInt2_Event` | `raw` | `0x7FFF0125AB40` | `0x4AAB40` | `48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC 20 48 63 FA 41` |
| `GetInventoryManager` | `rel32` | `0x7FFF01576430` | `0x7C6430` | `E8 ? ? ? ? 48 8B D3 48 8B C8 4C 8B 00 41 FF 90 00 02` |
| `GetLocalControllerById` | `raw` | `0x7FFF01691070` | `0x8E1070` | `48 83 EC 28 83 F9 FF 75 ? 48 8B 0D ? ? ? ? 48 8D 54 24 ? 48 8B 01 FF 90 ? ? ? ? 8B 08 48 63 C1 4C 8D 05` |
| `GetLocalPlayer_dispatcher` | `raw` | `0x7FFF01129200` | `0x379200` | `48 83 EC 38 48 8B 05 ? ? ? ? 48 85 C0 0F 85 14 06 00 00 48 89 5C 24 40 B9 50 00 00 00 48 89` |
| `GetMatrixForView` | `raw` | `0x7FFF00F19C50` | `0x169C50` | `40 53 48 83 EC 60 0F 29 74 24 50 0F 57 DB F3 0F 10 ? ? ? ? ? 49 8B D8` |
| `GetPlayerByIndex_export` | `raw` | `0x7FFF01CB0910` | `0xF00910` | `48 83 EC 28 4C 8D 05 ? ? ? ? 48 8D 15 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 4C 8D` |
| `GetPlayerInterp` | `raw` | `0x7FFF01669460` | `0x8B9460` | `40 53 48 83 EC ? 48 8B D9 48 8B 0D ? ? ? ? 48 83 C1` |
| `GetRemovedAimPunch_E8` | `rel32` | `0x7FFF015FD6E0` | `0x84D6E0` | `E8 ? ? ? ? 4C 8B C0 48 8D 55 ? 48 8B CB E8 ? ? ? ? 48 8D 0D` |
| `GetRemovedAimpunch` | `raw` | `0x7FFF00EC2947` | `0x112947` | `F2 0F 10 44 24 ? F2 0F 11 84 24 ? ? ? ? FF 15` |
| `GetSurfaceData` | `rel32` | `0x7FFF01703540` | `0x953540` | `E8 ? ? ? ? 80 78 18 00` |
| `GetTickBase` | `rel32` | `0x7FFF0166DA00` | `0x8BDA00` | `E8 ? ? ? ? EB ? 48 8B 05 ? ? ? ? 8B 40` |
| `GetTraceInfo` | `raw` | `0x7FFF015B6F50` | `0x806F50` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B E9 0F 29 74 24 ? 48 8B CA` |
| `GetUserCmdManager` | `raw` | `0x7FFF0166DC90` | `0x8BDC90` | `41 56 41 57 48 83 EC ? 48 8D 54 24` |
| `GetViewAngles` | `raw` | `0x7FFF01885CA0` | `0xAD5CA0` | `4C 8B C1 85 D2 74 08 48 8D 05 ? ? ? ? C3` |
| `GetWeaponInAccuracyRecoveryTime` | `rel32` | `0x7FFF01546600` | `0x796600` | `E8 ? ? ? ? F3 0F 10 B7 ? ? ? ? F3 0F 5E F8` |
| `GetWorldFovResolver` | `raw` | `0x7FFF015BCEF0` | `0x80CEF0` | `40 53 48 83 EC 50 48 8B D9 E8 ? ? ? ? 48 85 C0 74 ? 48 8B C8 48 83 C4 50 5B E9` |
| `GlobalVariables_ptr` | `riprel` | `0x7FFF02DFC5D8` | `0x204C5D8` | `48 89 15 ? ? ? ? 48 89 42` |
| `GloveApply_PerTick` | `raw` | `0x7FFF01971460` | `0xBC1460` | `40 55 56 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B B9 A0 00 00 00` |
| `GlowManager_ptr` | `riprel` | `0x7FFF030D8DB0` | `0x2328DB0` | `48 8B 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 8B 41` |
| `GlowObjectManager_GetInstance` | `raw` | `0x7FFF018BACD0` | `0xB0ACD0` | `48 8B 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 8B 41 38 C3` |
| `HandleBulletPenetration` | `raw` | `0x7FFF015D11F0` | `0x8211F0` | `48 8B C4 44 89 48 ? 48 89 50 ? 48 89 48 ? 55` |
| `HandleEntityList` | `rel32` | `0x7FFF00F73700` | `0x1C3700` | `E8 ? ? ? ? 84 C0 74 ? 48 63 03` |
| `HandleTeamIntro` | `raw` | `0x7FFF014B3EB0` | `0x703EB0` | `48 83 EC ? ? ? ? ? 44 38 89` |
| `HudChatPrintf` | `rel32` | `0x7FFF01E710F0` | `0x10C10F0` | `E8 ? ? ? ? 49 8B 4E 20 BA ? ? ? ?` |
| `InfoForResourceTypeCCompositeMaterialKit_TypeManager` | `stringref` | `0x7FFF021890B0` | `0x13D90B0` | `"InfoForResourceTypeCCompositeMaterialKit"` |
| `InfoForResourceTypeCCompositeMaterial_TypeManager` | `raw` | `0x7FFF02189600` | `0x13D9600` | `40 55 41 56 48 83 EC 68 48 8B EA 83 F9 06 0F 87 B4 02 00 00` |
| `InitFilter` | `raw` | `0x7FFF010DBBF0` | `0x32BBF0` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 0F B6 41 ? 33 FF 24 C9 C7 41 ?` |
| `InitPlayerMovementTraceFilter` | `raw` | `0x7FFF015F0660` | `0x840660` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 0F B6 41 ? 33 FF C7 41 ?` |
| `InitTraceInfo` | `raw` | `0x7FFF023AC2A0` | `0x15FC2A0` | `40 55 41 55 41 57 48 83 EC` |
| `IsGlowing` | `rel32` | `0x7FFF018BC300` | `0xB0C300` | `E8 ? ? ? ? 33 DB 84 C0 0F 84 ? ? ? ? 48 8B 4F` |
| `LevelInit` | `raw` | `0x7FFF01680100` | `0x8D0100` | `40 55 56 41 56 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48` |
| `LoadFileForMe` | `raw` | `0x7FFF016CBF40` | `0x91BF40` | `40 55 57 41 56 48 83 EC 20 4C` |
| `LoadPath` | `rel32` | `0x7FFF0146B200` | `0x6BB200` | `E8 ? ? ? ? 8B 44 24 2C` |
| `LocalPlayerController_ptr` | `riprel` | `0x7FFF030BB5D0` | `0x230B5D0` | `48 8B 05 ? ? ? ? 41 89 BE` |
| `LookupBone` | `rel32` | `0x7FFF016781E0` | `0x8C81E0` | `E8 ? ? ? ? 48 8B 8D ? ? ? ? B3` |
| `ModulationUpdate` | `raw` | `0x7FFF0178A450` | `0x9DA450` | `48 89 5C 24 08 57 48 83 EC 20 8B FA 48 8B D9 E8 ? ? ? ? 84 C0 0F 84` |
| `NoClipOnChange` | `raw` | `0x7FFF00F16C00` | `0x166C00` | `48 89 5C 24 10 48 89 74 24 18 48 89 7C 24 20 55 48 8B EC 48 83 EC 30 48 8D 05` |
| `NoSpread1` | `raw` | `0x7FFF01A2E2D0` | `0xC7E2D0` | `48 89 5C 24 08 57 48 81 EC F0 00` |
| `ParticleCollection` | `raw` | `0x7FFF00FA4D90` | `0x1F4D90` | `48 89 5C 24 ? 57 48 83 EC ? 0F 28 05` |
| `ParticleManager_ptr` | `riprel` | `0x7FFF02DE09E8` | `0x20309E8` | `48 8B 0D ? ? ? ? 41 B8 ? ? ? ? F3 0F 11 74 24 ? 48 C7 44 24 ? ? ? ? ?` |
| `PhysicsRunThink_Ctrl` | `raw` | `0x7FFF01687310` | `0x8D7310` | `48 89 5C 24 ? 57 48 81 EC ? ? ? ? ? ? ? 48 8B F9 FF 90` |
| `PhysicsRunThink_Pawn` | `raw` | `0x7FFF018BED50` | `0xB0ED50` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 8B 81 ? ? ? ? 48 8B F9` |
| `PlayVSound_client` | `raw` | `0x7FFF022BED00` | `0x150ED00` | `48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 48 8D 6C 24 ? 48 81 EC ? ? ? ? 33 FF` |
| `Prediction_ptr` | `riprel` | `0x7FFF02E07630` | `0x2057630` | `48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 40 53 56 41 54` |
| `ProcessImpacts` | `raw` | `0x7FFF0177EA50` | `0x9CEA50` | `48 8B C4 53 56 41 55` |
| `ProcessMovement` | `rel32` | `0x7FFF01789A30` | `0x9D9A30` | `E8 ? ? ? ? 48 8B 06 48 8B CE FF 90 ? ? ? ? 48 85 DB` |
| `RegenerateWeaponSkin` | `raw` | `0x7FFF0153C2A0` | `0x78C2A0` | `40 55 53 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 44 0F B6 FA 48 8B D9 BA ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ?` |
| `RegenerateWeaponSkin_v2` | `raw` | `0x7FFF0153C2A0` | `0x78C2A0` | `40 55 53 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 44 0F B6 FA 48 8B D9 BA ? ? ? ? 48 8D 0D ? ? ? ? E8` |
| `RegenerateWeaponSkins` | `raw` | `0x7FFF01560D40` | `0x7B0D40` | `48 83 EC ? E8 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? 48 8B 10` |
| `RenderDecals` | `raw` | `0x7FFF01E9CA50` | `0x10ECA50` | `44 88 4C 24 ? 55 53` |
| `ReportHit` | `rel32` | `0x7FFF013B2290` | `0x602290` | `E8 ? ? ? ? 48 8B AC 24 D8 00 00 00 48 81 C4` |
| `RunCommand` | `raw` | `0x7FFF0178BAF0` | `0x9DBAF0` | `48 8B C4 48 81 EC ? ? ? ? 48 89 58 10` |
| `RunCommand_processor` | `raw` | `0x7FFF0178BAF0` | `0x9DBAF0` | `48 8B C4 48 81 EC C8 00 00 00 48 89 58 10 48 89 68 18 48 8B EA 48 89 70 20 48 8B F1 48 89 78 F8` |
| `Scope_callsite` | `rel32` | `0x7FFF0160D530` | `0x85D530` | `E8 ? ? ? ? 80 7C 24 34 ? 74 ?` |
| `SendChatMessage` | `rel32` | `0x7FFF01E710F0` | `0x10C10F0` | `E8 ? ? ? ? 49 8B 4E 20 BA ? ? ? ?` |
| `Sensitivity_ptr` | `riprel` | `0x7FFF030D98C0` | `0x23298C0` | `48 8D 0D ? ? ? ? 66 0F 6E CD` |
| `SetAbsOrigin_Pawn` | `raw` | `0x7FFF00FCEF50` | `0x21EF50` | `48 89 5C 24 ? 57 48 83 EC ? ? ? ? 48 8B FA 48 8B D9 FF 90 ? ? ? ? 84 C0 0F 85` |
| `SetBodyGroup_inv` | `raw` | `0x7FFF01B472A0` | `0xD972A0` | `85 D2 0F 88 ? ? ? ? 53 55` |
| `SetCollisionBounds` | `raw` | `0x7FFF015B3980` | `0x803980` | `48 83 EC ? F2 0F 10 02 8B 42 08` |
| `SetDynamicAttributeValue` | `raw` | `0x7FFF01DB4F60` | `0x1004F60` | `48 89 6C 24 ? 57 41 56 41 57 48 81 EC ? ? ? ? 48 8B FA C7 44 24 ? ? ? ? ? 4D 8B F8` |
| `SetDynamicAttributeValue_raw` | `raw` | `0x7FFF01DB4F60` | `0x1004F60` | `48 89 6C 24 ? 57 41 56 41 57 48 81 EC ? ? ? ? 48 8B FA C7 44 24` |
| `SetMeshGroupMask` | `raw` | `0x7FFF017DDB50` | `0xA2DB50` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8D 99 ? ? ? ? 48 8B 71` |
| `SetModel` | `raw` | `0x7FFF0168B1C0` | `0x8DB1C0` | `40 53 48 83 EC ? 48 8B D9 4C 8B C2 48 8B 0D ? ? ? ? 48 8D 54 24` |
| `SetPlayerReady` | `raw` | `0x7FFF01CCDD90` | `0xF1DD90` | `40 53 48 83 EC 20 48 8B DA 48 8D 15 ? ? ? ? 48 8B CB FF 15 ? ? ? ? 85 C0 75 14 BA` |
| `SetTraceData` | `rel32` | `0x7FFF01584810` | `0x7D4810` | `E8 ? ? ? ? 8B 85 ? ? ? ? 48 8D 54 24 ? F2 0F 10 45` |
| `SetTypeKV3` | `raw` | `0x7FFF025CAEB0` | `0x181AEB0` | `40 53 48 83 EC 30 4C 8B 11 41 B9 ? ? ? ? 49 83 CA 01 0F B6 C2 80 FA 06 48 8B D9 44 0F 45 C8` |
| `SetViewAngle` | `raw` | `0x7FFF01894CE0` | `0xAE4CE0` | `85 D2 75 3D 48 63 81 ? ? ? ?` |
| `SetupCmd` | `raw` | `0x7FFF0166AF20` | `0x8BAF20` | `48 83 EC 28 E8 ? ? ? ? 8B 80` |
| `SetupMove` | `raw` | `0x7FFF01ACD0E0` | `0xD1D0E0` | `48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 83 EC ? 48 8B EA 4C 8B F1 E8 ? ? ? ? 48 8D 15` |
| `SetupMovementMoves` | `raw` | `0x7FFF01F36C8F` | `0x1186C8F` | `48 8B ? E8 ? ? ? ? 48 8B 5C 24 ? 48 8B 6C 24 ? 48 83 C4 30` |
| `SomeTimingFromPawn` | `raw` | `0x7FFF018072B0` | `0xA572B0` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 49 63 D8 48 8B F1` |
| `Spawner_PerTickOrchestrator` | `raw` | `0x7FFF01973FE0` | `0xBC3FE0` | `48 8B C4 55 53 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 80 B9 B1 13 00 00 00` |
| `SpectatorInput` | `raw` | `0x7FFF015892E0` | `0x7D92E0` | `48 89 5C 24 10 55 56 57 41 56 41 57 48 8B EC 48 83 EC 60 48 8B 01 41 8B F8 48 8B DA 48 8B F1 FF` |
| `TestSurfaces` | `raw` | `0x7FFF015B6E30` | `0x806E30` | `40 53 57 41 56 48 83 EC 50 8B` |
| `TracePlayerBBox` | `raw` | `0x7FFF01920E30` | `0xB70E30` | `48 89 5C 24 ? 55 57 41 54 41 55 41 56` |
| `TraceShape` | `raw` | `0x7FFF0173EAA0` | `0x98EAA0` | `48 89 5C 24 ? 48 89 4C 24 ? 55 57` |
| `TraceShape_Client` | `raw` | `0x7FFF0173EAA0` | `0x98EAA0` | `48 89 5C 24 20 48 89 4C 24 08 55 57 41 54 41 55 41 56 48 8D AC 24 10 E0 FF FF B8 F0 20 00 00` |
| `TraceToExit` | `raw` | `0x7FFF015B4900` | `0x804900` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? F2 0F 10 02` |
| `UpdatePostProcessing` | `raw` | `0x7FFF01CD1F20` | `0xF21F20` | `48 85 D2 0F 84 ? ? ? ? 48 89 5C 24 08 57 48 83 EC 60 80` |
| `UpdateSubClass` | `raw` | `0x7FFF00FAA93B` | `0x1FA93B` | `48 8B 41 10 48 8B D9 8B 50 30` |
| `UpdateTurningInAccuracy` | `rel32` | `0x7FFF0155FDA0` | `0x7AFDA0` | `E8 ? ? ? ? F3 0F 10 87 ? ? ? ? 44 0F 2F C8` |
| `VPhys2World_ptr` | `riprel` | `0x7FFF02DE06C8` | `0x20306C8` | `4C 8B 25 ? ? ? ? 24` |
| `ViewModelHideZoomed` | `raw` | `0x7FFF01550460` | `0x7A0460` | `48 89 5C 24 20 55 56 57 41 54 41 56 48 8B EC 48 83 EC 50 48 8D 05` |
| `ViewRender_ptr` | `riprel` | `0x7FFF030E0D38` | `0x2330D38` | `48 89 05 ? ? ? ? 48 8B C8 48 85 C0` |
| `WeaponC4_ptr` | `riprel` | `0x7FFF03059D58` | `0x22A9D58` | `48 8B 15 ? ? ? ? 48 8B 5C 24 ? FF C0 89 05 ? ? ? ? 48 8B C6 48 89 34 EA 80 BE` |
| `WriteSubtickFromEntry` | `raw` | `0x7FFF01A06330` | `0xC56330` | `48 89 5C 24 ? 55 57 41 56 48 8D 6C 24 ? 48 81 EC B0 00 00 00 8B 01 48 8B F9 81 4A 10 00 02` |
| `create_move_v2` | `raw` | `0x7FFF0187C120` | `0xACC120` | `85 D2 0F 85 ? ? ? ? 48 8B C4 44 88 40` |
| `draw_smoke_array` | `raw` | `0x7FFF01A2B380` | `0xC7B380` | `40 55 41 54 41 55 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 4C 8B E2` |
| `draw_view_punch_v2` | `raw` | `0x7FFF015B41C0` | `0x8041C0` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 49 8B E9 49 8B F8` |
| `entity_list_ptr` | `riprel` | `0x7FFF03281EF8` | `0x24D1EF8` | `48 8B 1D ? ? ? ? 48 8D 46` |
| `frame_stage_notify` | `raw` | `0x7FFF01882D31` | `0xAD2D31` | `4C 8B 0D ? ? ? ? 48 8D 15 ? ? ? ? 48 8B 8F ? ? ? ? F3 41 0F 10 51 ? 45 8B 49 ? 0F 5A D2 66 49 0F 7E D0 FF 15 ? ? ? ? 48 8B 97 ? ? ? ? 48 8B 0D ? ? ? ? E8 ? ? ? ? E9` |
| `get_fov` | `raw` | `0x7FFF015B41C0` | `0x8041C0` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 49 8B E9 49 8B F8` |
| `get_map_name` | `raw` | `0x7FFF01C8D4F0` | `0xEDD4F0` | `48 83 EC ? 48 8B 0D ? ? ? ? ? ? ? FF 90 ? ? ? ? 48 8B C8 48 83 C4` |
| `get_view_angles_v2` | `raw` | `0x7FFF01884600` | `0xAD4600` | `4D 85 C0 74 ? 85 D2 74` |
| `get_view_model` | `raw` | `0x7FFF015FF430` | `0x84F430` | `40 55 53 56 41 56 41 57 48 8B EC` |
| `global_vars_v2` | `riprel` | `0x7FFF030DBFB8` | `0x232BFB8` | `48 89 1D ? ? ? ? FF 15 ? ? ? ? 84 C0 74 ? 8B 0D ? ? ? ? 4C 8D 0D ? ? ? ? 4C 8D 05 ? ? ? ? BA ? ? ? ? FF 15 ? ? ? ? 48 8B 74 24 ? 48 8B C3` |
| `is_demo_or_hltv` | `raw` | `0x7FFF01CAE9B0` | `0xEFE9B0` | `48 83 EC ? 48 8B 0D ? ? ? ? ? ? ? FF 90 ? ? ? ? 84 C0 75 ? 38 05` |
| `level_init_v2` | `raw` | `0x7FFF018AA990` | `0xAFA990` | `40 55 56 41 56 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 0D` |
| `level_shutdown` | `raw` | `0x7FFF018AAC10` | `0xAFAC10` | `48 83 EC ? 48 8B 0D ? ? ? ? 48 8D 15 ? ? ? ? 45 33 C9 45 33 C0 ? ? ? FF 50 ? 48 85 C0 74 ? 48 8B 0D ? ? ? ? 48 8B D0 ? ? ? 41 FF 50 ? 48 83 C4` |
| `local_controller` | `riprel` | `0x7FFF030BB5D0` | `0x230B5D0` | `48 8B 05 ? ? ? ? 41 89 BE` |
| `mark_interp_latch_flags_dirty` | `raw` | `0x7FFF00FC8070` | `0x218070` | `40 53 56 57 48 83 EC ? 80 3D ? ? ? ? 00` |
| `on_add_entity_v2` | `raw` | `0x7FFF01718BB0` | `0x968BB0` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 8B 81 ? ? ? ? 49 8B F0` |
| `override_view_short` | `raw` | `0x7FFF01A0F840` | `0xC5F840` | `40 57 48 83 EC ? 48 8B FA E8 ? ? ? ? BA` |
| `paintkit_prefab` | `stringref` | `0x7FFF01E0D3B0` | `0x105D3B0` | `"set item texture prefab"` |
| `paintkit_seed` | `stringref` | `0x7FFF01CA1330` | `0xEF1330` | `"set item texture seed"` |
| `paintkit_wear` | `stringref` | `0x7FFF01CA1330` | `0xEF1330` | `"set item texture wear"` |
| `planted_c4_ptr` | `riprel` | `0x7FFF03059D58` | `0x22A9D58` | `48 8B 15 ? ? ? ? 48 8B 5C 24 ? FF C0 89 05 ? ? ? ? 48 8B C6 ? ? ? ? 80 BE ? ? ? ? 00` |
| `remove_legs` | `raw` | `0x7FFF01EA0410` | `0x10F0410` | `40 55 53 56 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? F2 0F 10 42` |
| `statTrak_killEater` | `stringref` | `0x7FFF01CA1330` | `0xEF1330` | `"kill eater"` |
| `statTrak_scoreType` | `stringref` | `0x7FFF00ECB7F0` | `0x11B7F0` | `"kill eater score type"` |
| `unlock_inventory` | `raw` | `0x7FFF014B11C0` | `0x7011C0` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B E9 48 8B 0D ? ? ? ? ? ? ? FF 50` |
| `update_global_vars` | `raw` | `0x7FFF01894730` | `0xAE4730` | `48 8B 0D ? ? ? ? 4C 8D 05 ? ? ? ? 48 85 D2` |
| `update_post_processing_v2` | `raw` | `0x7FFF01CD64D6` | `0xF264D6` | `48 89 AC 24 ? ? ? ? 45 33 ED` |
| `view_matrix_ptr` | `riprel` | `0x7FFF030E1B30` | `0x2331B30` | `48 8D 0D ? ? ? ? 48 89 44 24 ? 48 89 4C 24 ? 4C 8D 0D` |

## `engine2.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `BuildNumber_addr` | `riprel` | `0x7FFF1BF1CC74` | `0x60CC74` | `89 05 ? ? ? ? 48 8D 0D ? ? ? ? FF 15 ? ? ? ? 48 8B 0D` |
| `CCommand_Tokenize` | `raw` | `0x7FFF1BD0D710` | `0x3FD710` | `48 89 6C 24 20 4C 89 44 24 18 56 57 41 54 41 56 41 57 48 83 EC 70 48 8B F2 49 8B E8 8B 51 08 4C` |
| `CGameClient_ClientCommand` | `raw` | `0x7FFF1B9B1240` | `0xA1240` | `48 8B C4 4C 89 40 18 4C 89 48 20 55 53 57 48 8D 68 A1 48 81 EC C0 00 00 00 33 FF 48 63 DA 48 39` |
| `CHLTVClient_ExecuteStringCommand` | `raw` | `0x7FFF1BA30D70` | `0x120D70` | `40 53 56 48 81 EC 48 07 00 00 48 8B F1 48 8B DA 48 8B 4A 48 48 83 E1 FC 48 83 79 18 0F 76 03 48` |
| `CSplitScreenSlot` | `stringref` | `0x7FFF1BB5A250` | `0x24A250` | `"CSplitScreenSlot"` |
| `Cvar_RegisterConCommand` | `raw` | `0x7FFF1BD0D270` | `0x3FD270` | `48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 60 44 8B 15 ? ? ? ? 48 8B D9 65 48` |
| `Cvar_RegisterConVar` | `raw` | `0x7FFF1BD0C080` | `0x3FC080` | `48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 48 89 7C 24 20 41 54 41 56 41 57 48 81 EC D0 00 00` |
| `Engine::GetScreenAspectRatio` | `raw` | `0x7FFF1B9869D0` | `0x769D0` | `48 89 5C 24 08 57 48 83 EC 20 8B FA 48 8D 0D` |
| `Engine::PVSManager_ptr` | `riprel` | `0x7FFF1BF233F0` | `0x6133F0` | `48 8D 0D ? ? ? ? 33 D2 FF 50` |
| `Engine::RunPrediction` | `raw` | `0x7FFF1B976490` | `0x66490` | `40 55 41 56 48 83 EC ? 80 B9` |
| `Engine_Disconnect_main` | `raw` | `0x7FFF1BAE1510` | `0x1D1510` | `48 89 5C 24 20 55 57 41 54 48 8B EC 48 83 EC 70 45 33 E4 48 C7 05` |
| `Engine_HLTVClient_ExecuteStringCommand` | `raw` | `0x7FFF1BA30D70` | `0x120D70` | `40 53 56 48 81 EC 48 07 00 00 48 8B F1 48 8B DA 48 8B 4A 48 48 83 E1 FC 48 83 79 18 0F 76 03 48` |
| `Engine_HostStateMgr_QueueNewRequest` | `raw` | `0x7FFF1BB2AFC0` | `0x21AFC0` | `48 89 6C 24 18 48 89 7C 24 20 41 56 48 83 EC 30 48 8B EA 48 8B F9 8B 0D ? ? ? ? BA 02 00 00` |
| `Engine_HostStateMgr_QueueNewRequest` | `raw` | `0x7FFF1BB2AFC0` | `0x21AFC0` | `48 89 6C 24 18 48 89 7C 24 20 41 56 48 83 EC 30 48 8B EA 48 8B F9 8B 0D ? ? ? ? BA 02 00 00` |
| `Engine_LoadGameInfo` | `raw` | `0x7FFF1BA9D760` | `0x18D760` | `40 55 56 41 56 48 8D 6C 24 F0 48 81 EC 10 01 00 00 4C 8B F1 C7 44 24 40 00 00 00 00 48 8B CA 48` |
| `Engine_MountAddon` | `raw` | `0x7FFF1BAA3440` | `0x193440` | `48 85 D2 0F 84 DA 0A 00 00 48 8B C4 44 88 40 18 55 57 41 54 41 57 48 8D A8 C8 FC FF FF 48 81 EC` |
| `Engine_NetTimeoutDisconnect` | `raw` | `0x7FFF1B979780` | `0x69780` | `40 53 55 56 57 41 56 48 81 EC 80 00 00 00 0F 29 74 24 70 49 8B F8` |
| `Engine_NetworkGameClient_Connect` | `raw` | `0x7FFF1B98F400` | `0x7F400` | `48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 40 44 89 81 3C 02 00 00 49 8B E9 44 8B` |
| `Engine_NetworkGameClient_SetSignonState` | `raw` | `0x7FFF1B970F80` | `0x60F80` | `44 89 44 24 18 89 54 24 10 55 53 56 57 41 55 41 56 41 57 48 8D 6C 24 D9 48 81 EC D0 00 00 00 8B` |
| `Engine_RegisterConCommand` | `raw` | `0x7FFF1BD0D270` | `0x3FD270` | `48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 60 44 8B 15` |
| `Engine_RegisterConVar` | `raw` | `0x7FFF1BD0C080` | `0x3FC080` | `48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 48 89 7C 24 20 41 54 41 56 41 57 48 81 EC D0 00 00` |
| `NetworkGameClient_ptr` | `riprel` | `0x7FFF1C21A0C0` | `0x90A0C0` | `48 89 3D ? ? ? ? FF 87` |
| `WindowHeight_addr` | `riprel` | `0x7FFF1C21E4EC` | `0x90E4EC` | `8B 05 ? ? ? ? 89 03` |
| `WindowWidth_addr` | `riprel` | `0x7FFF1C21E4E8` | `0x90E4E8` | `8B 05 ? ? ? ? 89 07` |

## `filesystem_stdio.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `FullFileSystem_ptr` | `riprel` | `0x7FFF1B8F57A0` | `0x2157A0` | `8B 41 28 C3 CC CC CC CC CC CC CC CC CC CC CC CC 48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 48 8D 05 ? ? ? ? C3` |

## `inputsystem.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `InputSystemSvc_ptr` | `riprel` | `0x7FFF90452B50` | `0x42B50` | `48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 40 53 48 83 EC 20 33 DB` |
| `InputSystem_ptr` | `riprel` | `0x7FFF90452B50` | `0x42B50` | `48 89 05 ? ? ? ? 33 C0` |

## `matchmaking.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `GameTypes_ptr` | `riprel` | `0x7FFF09740F80` | `0x1B0F80` | `48 8D 0D ? ? ? ? FF 90` |

## `materialsystem2.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `CMaterial2_CompileComboAndGetVariables_DynamicShaderCompile` | `stringref` | `0x7FFF53BE3FA0` | `0x13FA0` | `"CompileComboAndGetVariables_DynamicShaderCompile(), C:\buildworker\csgo_rel_win64\build\src\materialsystem2\material2.cpp:2786"` |
| `CMaterial2_GetMode` | `raw` | `0x7FFF53BDBD40` | `0xBD40` | `48 89 5C 24 18 57 48 83 EC 30 8B 02 48 8B D9 39 05 ? ? ? ? 48 8B 0D ? ? ? ? 48 89 74 24` |
| `CMaterial2_GetVertexShaderInputSignature` | `raw` | `0x7FFF53BDC8C0` | `0xC8C0` | `48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 48 89 7C 24 20 41 56 48 83 EC 30 F6 41 0B 01 4C 8B` |
| `CMaterial2_LoadShadersAndSetupModes` | `raw` | `0x7FFF53BE0040` | `0x10040` | `44 89 44 24 18 48 89 54 24 10 53 56 41 54 41 55 48 81 EC 88 00 00 00 4C 8B E9 48 C7 44 24 60` |
| `CMaterialLayer_ApplyMaterialVarsForBatch` | `raw` | `0x7FFF53BE8B80` | `0x18B80` | `4C 89 4C 24 20 4C 89 44 24 18 48 89 54 24 10 53 55 56 57 41 54 41 55 41 56 41 57 48 83 EC 78` |
| `CMaterialLayer_BuildPassCommandData` | `raw` | `0x7FFF53BE8F80` | `0x18F80` | `89 54 24 10 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 58 FE FF FF 48 81 EC A8 02 00 00` |
| `CMaterialLayer_ComputeWorkItemsToSetupStaticCombosForMode` | `stringref` | `0x7FFF53BE5F3C` | `0x15F3C` | `"CMaterialLayer::ComputeWorkItemsToSetupStaticCombosForMode(3154): Failed call to FindOrLoadStaticComboData()!\n"` |
| `CMaterialLayer_CreateCommandBuffer` | `stringref` | `0x7FFF53BE9820` | `0x19820` | `"\nCMaterialLayer::CreateCommandBuffer(4446): Find a graphics programmer! Trying to bind a "%s" shader that doesn't exist! for %s\n"` |
| `CMaterialSystem2_BindIdentityInstanceIDBufferAndSetRenderState` | `stringref` | `0x7FFF53C40000` | `0x70000` | `"BindIdentityInstanceIDBufferAndSetRenderState: GetMode == NULL? Can't Render\n"` |
| `CMaterialSystem2_DynamicShaderCompile_UnloadAllMaterials` | `stringref` | `0x7FFF53C09AA0` | `0x39AA0` | `"CMaterialSystem2::DynamicShaderCompile_UnloadAllMaterials(1084): ERROR!!! Shaders not freed before shader reload! (See spew above)\n\n"` |
| `CMaterialSystem2_FrameUpdate` | `raw` | `0x7FFF53C0BAC0` | `0x3BAC0` | `48 89 4C 24 08 55 53 56 57 41 54 41 56 48 8B EC 48 83 EC 68 48 8D 05 ? ? ? ? 48 C7 45 C0` |
| `CMaterialSystem2_GetErrorMaterial` | `stringref` | `0x7FFF53BE74D7` | `0x174D7` | `"CMaterialSystem2::GetErrorMaterial(529): GetErrorMaterial() called when m_pMaterialTypeManager == NULL!\n"` |
| `CMaterialSystem2_Init` | `stringref` | `0x7FFF53C06E40` | `0x36E40` | `"MaterialSystem2"` |
| `CMaterial_SetVariableAndRenderState` | `stringref` | `0x7FFF53BFF9B0` | `0x2F9B0` | `"SetRenderStateValueFromVariable(1172): Unsupported render state type in material "%s"!\n"` |
| `CVfxProgramData_FindOrCreateStaticComboDataInCache` | `stringref` | `0x7FFF53C7E0E0` | `0xAE0E0` | `"CVfxProgramData::FindOrCreateStaticComboDataInCache(4448): Error! Ref count !=0 for static combo data cache entry!\n"` |
| `FindParameter` | `raw` | `0x7FFF53BE1E30` | `0x11E30` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B 59 20 48` |
| `MatSys::PrepareSceneMaterial` | `raw` | `0x7FFF53BE1BE0` | `0x11BE0` | `48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 30 48 8B 59 ? 48 8B F2 48 63 79 ? 48 C1 E7 06` |
| `UpdateParameter` | `raw` | `0x7FFF53BE2370` | `0x12370` | `48 89 7C 24 ? 41 56 48 83 EC ? 8B 81` |

## `networksystem.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `CNetChan_ProcessMessages` | `raw` | `0x7FFF194FB280` | `0xBB280` | `48 8B C4 53 57 41 54 41 56 48 81 EC A8 00 00 00 48 89 70 D0 45 33 E4 4C 89 68 C8 48 8B D9 48 89` |
| `CNetChan_SendNetMessage` | `raw` | `0x7FFF194FD670` | `0xBD670` | `48 89 5C 24 10 48 89 6C 24 18 56 57 41 56 48 83 EC 40 41 0F B6 F0 48 8D 99 F8 73 00 00 4C 8B F2` |
| `CNetworkSystem_Init` | `raw` | `0x7FFF1952C0C0` | `0xEC0C0` | `40 55 53 57 41 54 41 55 41 57 48 8D AC 24 98 FC FF FF 48 81 EC 68 04 00 00 4C 8B E9` |
| `CNetworkSystem_RegisterNetMessageHandlerAbstract` | `raw` | `0x7FFF194FBC00` | `0xBBC00` | `48 89 5C 24 10 48 89 6C 24 18 57 41 56 41 57 48 83 EC 50 4C 8B B4 24 90 00 00 00 41 8B D9` |
| `NetSystem_CNetChan_ProcessMessages` | `raw` | `0x7FFF194FB280` | `0xBB280` | `48 8B C4 53 57 41 54 41 56 48 81 EC A8 00 00 00 48 89 70 D0 45 33 E4 4C 89 68 C8 48 8B D9 48 89` |
| `NetSystem_CNetChan_SendNetMessage` | `raw` | `0x7FFF194FD670` | `0xBD670` | `48 89 5C 24 10 48 89 6C 24 18 56 57 41 56 48 83 EC 40 41 0F B6 F0 48 8D 99 F8 73 00 00 4C 8B F2` |
| `NetworkSystem_ptr` | `riprel` | `0x7FFF196C6E50` | `0x286E50` | `48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 48 83 EC 28 BA FF FF FF` |

## `particles.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `Particles::DrawArray` | `raw` | `0x7FFF137320B0` | `0x220B0` | `40 55 53 56 57 48 8D 6C 24` |
| `Particles::FindKeyVar` | `raw` | `0x7FFF1374A650` | `0x3A650` | `48 89 5C 24 ? 57 48 81 EC ? ? ? ? 33 C0 8B DA` |
| `Particles::SetMaterialShaderType` | `raw` | `0x7FFF137AD8D0` | `0x9D8D0` | `48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 54 41 56 41 57 48 81 EC ? ? ? ? 4C 63 32` |

## `rendersystemdx11.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `CRenderDeviceBase_CreateConstantBuffer` | `stringref` | `0x7FFF1B23F500` | `0x2F500` | `"CRenderDeviceBase::CreateConstantBuffer(1571): "` |
| `CRenderDeviceDx11_BeginSubmittingDisplayLists` | `stringref` | `0x7FFF1B24C4E0` | `0x3C4E0` | `"CRenderDeviceDx11::BeginSubmittingDisplayLists(1162): "` |
| `CRenderDeviceDx11_CompileShaderSourceMain` | `stringref` | `0x7FFF1B24FAF0` | `0x3FAF0` | `"Shader compilation failed! Reported no errors.\n"` |
| `CSwapChainDx11_QueuePresentAndWait` | `raw` | `0x7FFF1B244650` | `0x34650` | `40 55 53 57 41 54 41 55 48 8D 6C 24 C9 48 81 EC C0 00 00 00 48 8D 05 ? ? ? ? 4C 89 B4 24` |
| `CSwapChainDx11_ResizeBuffers` | `raw` | `0x7FFF1B24DD20` | `0x3DD20` | `48 8B C4 55 53 56 57 41 54 48 8B EC 48 83 EC 70 4C 89 68 10 4D 8B E0 4C 89 70 18 4C 8B EA 4C 89` |
| `RenderDeviceMgr_ptr` | `riprel` | `0x7FFF1B63B530` | `0x42B530` | `8B 5C 24 38 48 83 C4 20 5E C3 CC CC CC CC CC CC 48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 48 8D 05 ? ? ? ? C3` |
| `RenderSystemDx11_QueuePresentAndWait` | `raw` | `0x7FFF1B244650` | `0x34650` | `40 55 53 57 41 54 41 55 48 8D 6C 24 C9 48 81 EC C0 00 00 00 48 8D 05 ? ? ? ? 4C 89 B4 24` |
| `RenderSystemDx11_SetHardwareGammaRamp` | `raw` | `0x7FFF1B24F790` | `0x3F790` | `48 89 5C 24 18 57 B8 B0 40 00 00 E8 ? ? ? ? 48 2B E0 0F 29 BC 24 90 40 00 00 0F 57 C9 0F 28` |
| `RenderSystemDx11_SetMode` | `raw` | `0x7FFF1B2499E0` | `0x399E0` | `44 89 4C 24 20 44 89 44 24 18 89 54 24 10 55 53 56 57 41 54 41 55 41 56 41 57 48 81 EC D8 02 00` |

## `resourcesystem.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `ResourceSystem_BlockingLoadResourceByName` | `raw` | `0x7FFF688E7360` | `0x17360` | `40 53 55 57 48 81 EC 80 00 00 00 48 8B 01 49 8B E8 48 8B FA 48 8B D9 FF 90 98 01 00 00 83 F8 03` |
| `ResourceSystem_FindOrRegisterResourceByName` | `raw` | `0x7FFF688E6D80` | `0x16D80` | `48 89 5C 24 18 48 89 74 24 20 57 48 81 EC A0 00 00 00 F7 02 FF FF FF 3F 41 0F B6 F8 48 8B DA 48` |
| `ResourceSystem_FrameUpdate` | `raw` | `0x7FFF688EC010` | `0x1C010` | `44 88 4C 24 20 44 89 44 24 18 89 54 24 10 55 56 41 54 41 55 41 56 48 8D 6C 24 A0 48 81 EC 60 01` |

## `scenesystem.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `CSceneAnimatableObject::GeneratePrimitives` | `raw` | `0x7FFF13F633F0` | `0x733F0` | `48 8B C4 48 89 58 08 48 89 50 10 55 56 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ?` |
| `CSceneSkyBoxObject_DrawSkyboxArray` | `raw` | `0x7FFF1403FA60` | `0x14FA60` | `45 85 C9 0F 8E ? ? ? ? 4C 8B DC 55 41 56 49 8D AB 58 FC FF FF 48 81 EC 98 04 00 00` |
| `CSceneSystem_CreateStaticShape` | `raw` | `0x7FFF13FA19C0` | `0xB19C0` | `48 8B C4 48 89 48 08 55 41 54 41 56 48 8D 68 D8 48 81 EC 10 01 00 00 4C 8B 65 50 48 8D 4D 80` |
| `CSceneSystem_InitGfxObjects` | `raw` | `0x7FFF13FA3D00` | `0xB3D00` | `40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 08 FE FF FF 48 81 EC F8 02 00 00` |
| `CSceneSystem_RenderViewLayer_Dispatch` | `raw` | `0x7FFF13FDDC50` | `0xEDC50` | `48 8B C4 48 89 48 08 55 53 56 57 41 54 41 55 41 56 41 57 48 8D A8 B8 FE FF FF 48 81 EC 08 02 00` |
| `CSceneSystem_Thread_CullView` | `stringref` | `0x7FFF13FD91C0` | `0xE91C0` | `"CSceneSystem::Thread_CullView(), C:\buildworker\csgo_rel_win64\build\src\scenesystem\scenesystem.cpp:3312"` |
| `DrawObject_legacy` | `raw` | `0x7FFF13F45AC0` | `0x55AC0` | `48 8B C4 53 57 41 54 48 81 EC D0 00 00 00 49 63 F9 49` |
| `DrawSkyboxArray` | `raw` | `0x7FFF1403FA60` | `0x14FA60` | `45 85 C9 0F 8E ? ? ? ? 4C 8B DC 55` |
| `SceneSystem::DrawAggeregateObject` | `raw` | `0x7FFF1401CE20` | `0x12CE20` | `48 8B C4 4C 89 48 20 4C 89 40 ? 48 89 50 ? 55 53 41 57 48 8D A8` |
| `SceneSystem::DrawArrayLight` | `raw` | `0x7FFF13F6A990` | `0x7A990` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 54 24` |
| `SceneSystem_Thread_RenderSceneDrawList` | `raw` | `0x7FFF13FDD900` | `0xED900` | `40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 E1 48 81 EC D8 00 00 00 4C 8B 71 28 48 8B D9` |
| `SceneSystem_ptr` | `riprel` | `0x7FFF147CB490` | `0x8DB490` | `48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 48 8D 0D ? ? ? ? E9` |

## `schemasystem.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `CSchemaSystem_InstallSchemaBindings` | `raw` | `0x7FFF688875D0` | `0x375D0` | `40 53 48 83 EC 20 48 8B DA 48 8B D1 48 8D 0D ? ? ? ? E8 ? ? ? ? 85 C0 74 08 32 C0` |
| `CSchemaSystem_RegisterModuleAndBuiltins` | `raw` | `0x7FFF688606F0` | `0x106F0` | `48 89 54 24 10 53 56 57 41 55 41 56 41 57 48 83 EC 48 45 33 ED 49 63 C0 33 FF 44 89 AC 24 90 00` |
| `CSchemaSystem_VerifySchemaBindingConsistency` | `raw` | `0x7FFF688558F0` | `0x58F0` | `88 54 24 10 55 53 57 41 54 41 55 48 8B EC 48 81 EC 80 00 00 00 65 48 8B 04 25 58 00 00 00` |
| `SchemaSystem_ptr` | `riprel` | `0x7FFF688C6800` | `0x76800` | `48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 48 89 5C 24 08 48 89 74` |

## `soundsystem.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `SoundSystem::PlayVSound` | `raw` | `0x7FFF18009840` | `0x349840` | `48 8B C4 48 89 58 08 57 48 81 EC ? ? ? ? 33 FF 48 8B D9` |
| `SoundSystem::SomeUtlSymbolFunc` | `raw` | `0x7FFF17D70740` | `0xB0740` | `48 89 74 24 18 57 48 83 EC 20 48 63 F2 48 8B F9 3B 71 30` |
| `SoundSystem_ptr` | `riprel` | `0x7FFF181D2360` | `0x512360` | `48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 48 89 15` |

## `tier0.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `CVar_ptr` | `riprel` | `0x7FFF53A493B0` | `0x3A93B0` | `48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC E9` |
| `LoadKV3` | `raw` | `0x7FFF537C9090` | `0x129090` | `48 89 5C 24 08 57 48 83 EC 70 4C 8B D1 48 C7 C0 FF FF FF FF 48 FF C0 41 80 3C 00 00 75 F6` |
| `Tier0::LoadKeyValues` | `rel32` | `0x7FFF537C9160` | `0x129160` | `E8 ? ? ? ? 8B 4C 24 34 0F B6 D8` |
| `Tier0::UtlBuffer` | `raw` | `0x7FFF536F3F10` | `0x53F10` | `48 89 5C 24 ? 57 48 83 EC ? 8B 41 ? 8D 7A` |

## `vphysics2.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `VPhysics2_Startup` | `raw` | `0x7FFF183BAF20` | `0x6AF20` | `48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 48 89 7C 24 20 41 54 41 56 41 57 48 83 EC 70 48 83 3D` |

