# CS2 Signatures

_This file is regenerated on every successful run of `cs2-sdk`._

**326/372 signatures resolved across 11 module(s).**

## `animationsystem.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `Animation::ShouldUpdateSequences` | `raw` | `0x7FFC0F0FF0A0` | `0x14F0A0` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 49 8B 40 48` |

## `client.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `AddNametagEntity` | `raw` | `0x7FFBFFE7AE20` | `0x78AE20` | `40 55 53 56 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B DA` |
| `AddStattrakEntity` | `raw` | `0x7FFC0013B030` | `0xA4B030` | `48 8B C4 48 89 58 08 48 89 70 10 57 48 83 EC 50 33 F6 8B FA 48 8B D1` |
| `AutowallInit` | `raw` | `0x7FFBFFFD0860` | `0x8E0860` | `40 53 48 83 EC ? 48 8B D9 48 81 C1 ? ? ? ? E8 ? ? ? ?` |
| `AutowallTraceData` | `raw` | `0x7FFC0007D260` | `0x98D260` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B 09` |
| `AutowallTracePos` | `raw` | `0x7FFBFFEF6BE0` | `0x806BE0` | `40 55 56 41 54 41 55 41 57 48 8B EC` |
| `BulkRegenIterator` | `raw` | `0x7FFBFFE7E321` | `0x78E321` | `57 48 83 EC 40 0F B6 F9 E8 ? ? ? ? 48 85 C0 0F 84` |
| `CAM_ThinkReturn` | `raw` | `0x7FFBFFA0A44F` | `0x31A44F` | `BA 04 00 00 00 FF 15 ? ? ? ? 84 C0 0F 84` |
| `CAttributeStringFill` | `rel32` | `0x7FFC0059C570` | `0xEAC570` | `E8 ? ? ? ? 41 83 CF 08` |
| `CAttributeStringInit` | `rel32` | `0x7FFBFFCE8600` | `0x5F8600` | `E8 ? ? ? ? 48 8D 05 ? ? ? ? 48 89 7D ? 48 89 45 ? 49 8D 4F` |
| `CBaseEntity_TakeDamageOld` | `raw` | `0x7FFBFF913C70` | `0x223C70` | `40 55 53 56 57 41 54 48 8D 6C 24 E0 48 81 EC 20 01 00 00 4D 8B E0 48 8B FA 48 8B F1 E8` |
| `CBaseModelEntity_SetBodygroup` | `raw` | `0x7FFBFFFC87F0` | `0x8D87F0` | `85 D2 0F 88 CB 01 00 00 55 53 56 41 56 48 8B EC 48 83 EC 78 45 8B F0 8B DA 48 8B F1 E8 ? ? ?` |
| `CBodyComponent` | `stringref` | `0x7FFBFF8AC0B0` | `0x1BC0B0` | `"CBodyComponent"` |
| `CBodyComponentSkeletonInstance` | `stringref` | `0x7FFBFF8B2F90` | `0x1C2F90` | `"CBodyComponentSkeletonInstance"` |
| `CBufferStringInit` | `raw` | `0x7FFC00ED0160` | `0x17E0160` | `48 89 5C 24 ? 57 48 83 EC ? 8B 41 ? 48 8D 79` |
| `CCSGOInput::CreateMove` | `raw` | `0x7FFC0034C2A0` | `0xC5C2A0` | `48 8B C4 4C 89 40 18 48 89 48 08 55 53 41 54 41 55` |
| `CCSGameRules` | `stringref` | `0x7FFBFF76E160` | `0x7E160` | `"CCSGameRules"` |
| `CCSGameRulesProxy` | `stringref` | `0x7FFBFFDD9450` | `0x6E9450` | `"CCSGameRulesProxy"` |
| `CCSInventoryManager::EquipItemInLoadout` | `raw` | `0x7FFBFFEB2090` | `0x7C2090` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 89 54 24 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 0F B7 FA` |
| `CCSPlayerController` | `stringref` | `0x7FFBFFED4FA0` | `0x7E4FA0` | `"CCSPlayerController"` |
| `CCSPlayerController` | `stringref` | `0x7FFBFFED4FA0` | `0x7E4FA0` | `"CCSPlayerController"` |
| `CCSPlayerController_ActionTrackingServices` | `stringref` | `0x7FFBFFED4FA0` | `0x7E4FA0` | `"CCSPlayerController_ActionTrackingServices"` |
| `CCSPlayerController_DamageServices` | `stringref` | `0x7FFBFFED4FA0` | `0x7E4FA0` | `"CCSPlayerController_DamageServices"` |
| `CCSPlayerController_InGameMoneyServices` | `stringref` | `0x7FFBFFED4FA0` | `0x7E4FA0` | `"CCSPlayerController_InGameMoneyServices"` |
| `CCSPlayerController_InventoryServices` | `stringref` | `0x7FFBFFED4FA0` | `0x7E4FA0` | `"CCSPlayerController_InventoryServices"` |
| `CCSPlayerInventory::GetItemInLoadout` | `raw` | `0x7FFBFFEB3CB0` | `0x7C3CB0` | `40 55 48 83 EC ? 49 63 E8` |
| `CCSPlayerPawn` | `stringref` | `0x7FFC0029F6E0` | `0xBAF6E0` | `"CCSPlayerPawn"` |
| `CCSPlayer_BulletServices` | `stringref` | `0x7FFBFFF02BC0` | `0x812BC0` | `"CCSPlayer_BulletServices"` |
| `CCSPlayer_BulletServices` | `stringref` | `0x7FFBFFF02BC0` | `0x812BC0` | `"CCSPlayer_BulletServices"` |
| `CCSPlayer_CameraServices` | `stringref` | `0x7FFBFFEFECD0` | `0x80ECD0` | `"CCSPlayer_CameraServices"` |
| `CCSPlayer_HostageServices` | `stringref` | `0x7FFBFFF02BC0` | `0x812BC0` | `"CCSPlayer_HostageServices"` |
| `CCSPlayer_ItemServices` | `stringref` | `0x7FFBFFF3F710` | `0x84F710` | `"CCSPlayer_ItemServices"` |
| `CCSPlayer_MovementServices` | `stringref` | `0x7FFBFFF2CAF0` | `0x83CAF0` | `"CCSPlayer_MovementServices"` |
| `CCSPlayer_MovementServices` | `stringref` | `0x7FFBFFF2CAF0` | `0x83CAF0` | `"CCSPlayer_MovementServices"` |
| `CCSPlayer_PingServices` | `stringref` | `0x7FFBFFF40920` | `0x850920` | `"CCSPlayer_PingServices"` |
| `CCSPlayer_RunCommand_Context` | `raw` | `0x7FFC000CA390` | `0x9DA390` | `48 8B C4 48 81 EC C8 00 00 00 48 89 58 10 48 89 68 18 48 8B EA 48 89 70 20 48 8B F1 48 89 78 F8` |
| `CCSPlayer_UseServices` | `stringref` | `0x7FFBFFF70B50` | `0x880B50` | `"CCSPlayer_UseServices"` |
| `CCSPlayer_WaterServices` | `stringref` | `0x7FFBFFF65DE0` | `0x875DE0` | `"CCSPlayer_WaterServices"` |
| `CCSPlayer_WeaponServices` | `stringref` | `0x7FFBFFF66190` | `0x876190` | `"CCSPlayer_WeaponServices"` |
| `CCSPlayer_WeaponServices` | `stringref` | `0x7FFBFFF66190` | `0x876190` | `"CCSPlayer_WeaponServices"` |
| `CCSWeaponBase` | `stringref` | `0x7FFBFFE6F260` | `0x77F260` | `"CCSWeaponBase"` |
| `CCSWeaponBaseGun` | `stringref` | `0x7FFBFFE6F300` | `0x77F300` | `"CCSWeaponBaseGun"` |
| `CCSWeaponBaseVData` | `stringref` | `0x7FFBFFE4A170` | `0x75A170` | `"CCSWeaponBaseVData"` |
| `CCollisionProperty` | `stringref` | `0x7FFBFF9D0EE0` | `0x2E0EE0` | `"CCollisionProperty"` |
| `CCompositeMaterialManager_AddPanoramaPanelRenderRequest_Caller` | `stringref` | `0x7FFC00AA9064` | `0x13B9064` | `"CCompositeMaterialManager::AddNewPanoramaPanelRenderRequest"` |
| `CDecoyProjectile` | `stringref` | `0x7FFBFFE3E0A0` | `0x74E0A0` | `"CDecoyProjectile"` |
| `CEconItemSchema::GetAttributeDefinitionByName` | `raw` | `0x7FFC0073A5E0` | `0x104A5E0` | `48 89 5C 24 10 48 89 6C 24 18 57 41 56 41 57 48 83 EC 60 48 8D 05` |
| `CEconItemView::GetCustomPaintKitIndex` | `raw` | `0x7FFBFFFA9C70` | `0x8B9C70` | `48 89 5C 24 ? 57 48 83 EC ? 8B 15 ? ? ? ? 48 8B F9 65 48 8B 04 25` |
| `CFlashbangProjectile` | `stringref` | `0x7FFC006CDB30` | `0xFDDB30` | `"CFlashbangProjectile"` |
| `CFogController` | `stringref` | `0x7FFBFF96EF20` | `0x27EF20` | `"CFogController"` |
| `CGameEntitySystem::OnAddEntity` | `raw` | `0x7FFC00056F30` | `0x966F30` | `48 89 74 24 ? 57 48 83 EC ? 41 B9 ? ? ? ? 41 8B C0 41 23 C1 48 8B F2 41 83 F8 ? 48 8B F9 44 0F 45 C8 41 81 F9 ? ? ? ? 73 ? FF 81` |
| `CGameEntitySystem::OnRemoveEntity` | `raw` | `0x7FFC00057790` | `0x967790` | `48 89 74 24 ? 57 48 83 EC ? 41 B9 ? ? ? ? 41 8B C0 41 23 C1 48 8B F2 41 83 F8 ? 48 8B F9 44 0F 45 C8 41 81 F9 ? ? ? ? 73 ? FF 89` |
| `CGameSceneNode` | `stringref` | `0x7FFBFF893840` | `0x1A3840` | `"CGameSceneNode"` |
| `CGameSceneNode_BuildBoneMergeWork` | `raw` | `0x7FFC0002E3C0` | `0x93E3C0` | `40 55 56 57 41 54 41 55 41 56 41 57 48 83 EC 50 48 8D 6C 24 50 80 A1 06 01 00 00 FB 4C 8B F9 80` |
| `CGameSceneNode_StartHierarchicalAttachment` | `raw` | `0x7FFC0007AE80` | `0x98AE80` | `48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20 57 41 54 41 55 41 56 41 57 48 83 EC 30 48 8B F9 8B` |
| `CGameTrace_TraceShape_Client` | `raw` | `0x7FFC0007D340` | `0x98D340` | `48 89 5C 24 20 48 89 4C 24 08 55 57 41 54 41 55 41 56 48 8D AC 24 10 E0 FF FF B8 F0 20 00 00` |
| `CGlowProperty` | `stringref` | `0x7FFBFF9D10F0` | `0x2E10F0` | `"CGlowProperty"` |
| `CGlowProperty_OnGlowTypeChanged` | `raw` | `0x7FFC001FB630` | `0xB0B630` | `48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 48 8B 05 ? ? ? ? 48 8B D9 F3 0F 10 41 4C` |
| `CHEGrenadeProjectile` | `stringref` | `0x7FFC006CDBD0` | `0xFDDBD0` | `"CHEGrenadeProjectile"` |
| `CInputPtrGlobal` | `riprel` | `0x7FFC017513C0` | `0x20613C0` | `4C 8B 05 ? ? ? ? 41 8B 80 50 0B 00 00 85 C0` |
| `CMolotovProjectile` | `stringref` | `0x7FFBFFE3E280` | `0x74E280` | `"CMolotovProjectile"` |
| `CPaintKitDefinitions_FindOrCreateByName` | `stringref` | `0x7FFC00747DD0` | `0x1057DD0` | `"Kit "[%s]" specified, but doesn't exist!! You're probably missing an entry in items_paintkits.txt or items_stickerkits.txt or need to run with -use_local_item_data\n"` |
| `CPaintKitDefinitions_LoadDefaultKit` | `stringref` | `0x7FFC00719EA0` | `0x1029EA0` | `"Unable to find "default" paint kit in "paint_kits_rarity""` |
| `CPostProcessingVolume` | `stringref` | `0x7FFBFF993CB0` | `0x2A3CB0` | `"CPostProcessingVolume"` |
| `CS2ItemEditor_BuildTemplateMaterialFromFile` | `raw` | `0x7FFC00AAA1E0` | `0x13BA1E0` | `48 89 54 24 10 55 53 41 55 41 57 48 8D AC 24 18 F9 FF FF 48 81 EC E8 07 00 00 4C 8B FA 48 85 D2` |
| `CSBaseGunFireData_fn` | `raw` | `0x7FFC00BD58D0` | `0x14E58D0` | `48 8B C4 55 53 56 57 41 54 41 55 41 56 41 57 48 8D 68 A8 48 81 EC ? ? ? ? 4C 8B 69` |
| `CSGOInput_ptr` | `riprel` | `0x7FFC017513C0` | `0x20613C0` | `48 8B 0D ? ? ? ? 4C 8B C6 8B 10 E8` |
| `CSGOInput_resolved` | `riprel` | `0x7FFC017513C7` | `0x20613C7` | `48 8B 0D ? ? ? ? 8B 10 E8 ? ? ? ? 45 32 FF` |
| `CSkeletonInstance` | `stringref` | `0x7FFBFF893970` | `0x1A3970` | `"CSkeletonInstance"` |
| `CSkeletonInstance::SetMeshGroupMask` | `raw` | `0x7FFC0011C3F0` | `0xA2C3F0` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8D 99` |
| `CSkeletonInstance_GetTransformsForHitboxList` | `raw` | `0x7FFC00108F60` | `0xA18F60` | `48 89 5C 24 18 55 56 57 41 55 41 57 48 81 EC A0 00 00 00 49 63 28 4D 8B F8 48 8B FA 48 8B D9 85` |
| `CSkeletonInstance_OnBodyGroupChoiceChanged` | `raw` | `0x7FFC00113BB0` | `0xA23BB0` | `48 89 5C 24 08 57 48 83 EC 20 49 63 D8 49 8B F9 45 85 C0 78 20 3B 99 18 02 00 00 7D 18` |
| `CSkeletonInstance_OnSkeletonModelChanged` | `raw` | `0x7FFC00113DC0` | `0xA23DC0` | `49 8B 00 48 89 81 B8 00 00 00 C6 81 B0 00 00 00 01 C3` |
| `CSkeletonInstance_PostDataUpdate` | `raw` | `0x7FFC00114D50` | `0xA24D50` | `48 8B C4 4C 89 40 18 89 50 10 55 57 48 8D A8 68 FE FF FF 48 81 EC 88 02 00 00 48 89 70 E0 48 8B` |
| `CSkeletonInstance_SetMaterialGroup` | `raw` | `0x7FFC0011B0D0` | `0xA2B0D0` | `3B 91 C4 03 00 00 74 24 89 91 C4 03 00 00 48 8B 81 28 02 00 00 48 85 C0 74 12` |
| `CSkeletonInstance_SetMeshGroupMask` | `raw` | `0x7FFC00113D20` | `0xA23D20` | `48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 49 8B 00 49 8B F8 48 8B F2 48 8B D9 48 39 81 C8 01` |
| `CSmokeGrenadeProjectile` | `stringref` | `0x7FFBFFE3E320` | `0x74E320` | `"CSmokeGrenadeProjectile"` |
| `CTonemapController2` | `stringref` | `0x7FFBFF947BE0` | `0x257BE0` | `"CTonemapController2"` |
| `CUtlVector_CompositeMaterialInput_AddToTail` | `raw` | `0x7FFBFFE79A00` | `0x789A52` | `41 B9 88 02 00 00 8B 57 14 81 E2 FF FF FF 3F 8D 71 01 44 8B C6 FF 15` |
| `C_AttributeContainer` | `stringref` | `0x7FFC003065E0` | `0xC165E0` | `"C_AttributeContainer"` |
| `C_BaseEntity` | `stringref` | `0x7FFBFF73E260` | `0x4E260` | `"C_BaseEntity"` |
| `C_BaseModelEntity` | `stringref` | `0x7FFBFF847F60` | `0x157F60` | `"C_BaseModelEntity"` |
| `C_BasePlayerPawn` | `stringref` | `0x7FFBFF75DA20` | `0x6DA20` | `"C_BasePlayerPawn"` |
| `C_C4` | `stringref` | `0x7FFBFF78A370` | `0x9A370` | `"C_C4"` |
| `C_CSPlayerPawn` | `stringref` | `0x7FFBFFDB2380` | `0x6C2380` | `"C_CSPlayerPawn"` |
| `C_CSPlayerPawnBase` | `stringref` | `0x7FFC002C57C0` | `0xBD57C0` | `"C_CSPlayerPawnBase"` |
| `C_CSWeaponBase` | `stringref` | `0x7FFBFFE32030` | `0x742030` | `"C_CSWeaponBase"` |
| `C_EconEntity_BuildLegacyGloveSkinMaterial` | `stringref` | `0x7FFC002AFB00` | `0xBBFB00` | `"MapPlayerPreview gloves"` |
| `C_EconEntity_BuildLegacyWeaponSkinMaterial` | `stringref` | `0x7FFBFFE7C050` | `0x78C050` | `"workshop preview weapon"` |
| `C_EconEntity_BuildModernWeaponSkinMaterial` | `raw` | `0x7FFC004728E0` | `0xD828E0` | `48 85 C9 0F 84 ? ? 00 00 48 8B C4 48 89 50 10 48 89 48 08 55 41 54 41 56 41 57 48 8D A8 B8 FA` |
| `C_EconEntity_BuildNametagOverlayMaterial` | `stringref` | `0x7FFBFFE7AE20` | `0x78AE20` | `"low-res nametag"` |
| `C_EconItemView` | `stringref` | `0x7FFBFFDFB400` | `0x70B400` | `"C_EconItemView"` |
| `C_EconWearable_OnNewCustomMaterials` | `stringref` | `0x7FFC007A67D0` | `0x10B67D0` | `"Invalid EconItemView -- Can't create custom materials for wearable, debug this.\n"` |
| `C_Hostage` | `stringref` | `0x7FFBFF7D73D0` | `0xE73D0` | `"C_Hostage"` |
| `C_Inferno` | `stringref` | `0x7FFBFF7E7390` | `0xF7390` | `"C_Inferno"` |
| `C_PlantedC4` | `stringref` | `0x7FFBFF7E06F0` | `0xF06F0` | `"C_PlantedC4"` |
| `C_SmokeGrenadeProjectile` | `stringref` | `0x7FFBFF785960` | `0x95960` | `"C_SmokeGrenadeProjectile"` |
| `CacheParticleEffect` | `raw` | `0x7FFBFF8F7B10` | `0x207B10` | `4C 8B DC 53 48 81 EC ? ? ? ? F2 0F 10 05` |
| `CalcSpread` | `raw` | `0x7FFC0036C6F0` | `0xC7C6F0` | `48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 4C 63 EA` |
| `CalcViewmodel` | `raw` | `0x7FFBFFF3E040` | `0x84E040` | `40 55 53 56 41 56 41 57 48 8B EC` |
| `CalcViewmodelTransform_v2` | `raw` | `0x7FFBFFE92460` | `0x7A2460` | `48 89 5C 24 20 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 80 48 81 EC 80 01 00 00 48 8B FA` |
| `CalcViewmodelView` | `raw` | `0x7FFC003599D0` | `0xC699D0` | `40 53 48 83 EC 60 48 8B 41 08 49 8B D8 8B 48 30 48 C1 E9 0C F6 C1 01 0F 85 48 01 00 00 41 B8 07` |
| `CalculateInterpolation` | `rel32` | `0x7FFC00BB5600` | `0x14C5600` | `E8 ? ? ? ? 8B 45 ? 3B 45 60 75 04 32 D2 EB 09 BA 01 00 00 00 41 0F 4C D5 C0 EA 07 84 D2 0F 85 87` |
| `CalculateWorldSpaceBones` | `raw` | `0x7FFC000F9910` | `0xA09910` | `48 89 4C 24 ? 55 53 56 57 41 54 41 55 41 56 41 57 B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 48 8D 6C 24 ? 48 8B 81` |
| `ClearHUDWeaponIcon` | `rel32` | `0x7FFC004DB720` | `0xDEB720` | `E8 ? ? ? ? 8B F8 C6 84 24 ? ? ? ? ?` |
| `ClientModeCSNormal_OnEvent` | `raw` | `0x7FFC0034A110` | `0xC5A110` | `40 53 57 48 81 EC 78 02 00 00 48 8B CA 48 8B FA` |
| `ClientMode_ptr` | `riprel` | `0x7FFC01A2BA60` | `0x233BA60` | `48 8D 0D ? ? ? ? 48 69 C0 ? ? ? ? 48 03 C1 C3 CC CC` |
| `Client_DispatchSpawn` | `raw` | `0x7FFC00BC32A0` | `0x14D32A0` | `4C 8B DC 55 56 48 83 EC 78 49 8B 68 08 48 8B F1 48 85 ED 0F 84 72 01 00 00 49 89 5B 08 49 8D 4B` |
| `CompositeMaterialPanoramaPanel_Init` | `stringref` | `0x7FFC0027FB00` | `0xB8FB00` | `"CompositeMaterialPanoramaPanel_t::Init"` |
| `ConCommand_firstperson` | `raw` | `0x7FFC001B8B50` | `0xAC8B50` | `48 83 EC 28 48 8B 0D ? ? ? ? 48 8D 54 24 ? 48 8B 01 FF 90 08 03 00 00 83 7C 24 ? 00 75 ? 48 8B 05 ? ? ? ? C6 80 29 02 00 00 00 C7 80 A8 06 00 00 00` |
| `ConCommand_thirdperson` | `raw` | `0x7FFC001B8C30` | `0xAC8C30` | `48 83 EC 38 48 8B 0D ? ? ? ? 48 8D 54 24 ? 48 8B 01 FF 90 08 03 00 00 83 7C 24 ? 00 0F 85 ? ? ? ? 4C 8B 05 ? ? ? ? 41 8B 80 50 0B 00 00` |
| `ConvarGet` | `raw` | `0x7FFBFFFADF82` | `0x8BDF82` | `8B D0 48 8D 0D ? ? ? ? E8 ? ? ? ? 0F 10 45 ? 83 F0 74` |
| `CreateBaseTypeCache` | `raw` | `0x7FFC00BFE630` | `0x150E630` | `40 53 48 83 EC ? 4C 8B 49 ? 44 8B D2` |
| `CreateEntityByClassName` | `raw` | `0x7FFC00CF23D6` | `0x16023D6` | `4C 8D 05 ? ? ? ? 4C 8B CF BA 03 00 00 00 FF 15 ? ? ? ? EB ? 0F B7 C8 48` |
| `CreateNewSubtickMoveStep` | `rel32` | `0x7FFBFFBA1CD0` | `0x4B1CD0` | `E8 ? ? ? ? 48 8B D0 48 8B CE E8 ? ? ? ? 48 8B C8` |
| `CreateParticleEffect` | `raw` | `0x7FFC000758C0` | `0x9858C0` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? F3 0F 10 1D ? ? ? ? 41 8B F8 8B DA 4C 8D 05` |
| `CreateSOSubclassEconItem` | `raw` | `0x7FFC006E4EB0` | `0xFF4EB0` | `48 83 EC 28 B9 48 00 00 00 E8 ? ? ? ? 48 85` |
| `DestroyParticle` | `raw` | `0x7FFC00034DB0` | `0x944DB0` | `83 FA ? 0F 84 ? ? ? ? 41 54` |
| `DispatchEffect` | `raw` | `0x7FFBFFA4A4C0` | `0x35A4C0` | `48 89 5C 24 ? 57 48 83 EC ? 48 8B F9 48 8B DA 48 8D 4C 24` |
| `DispatchSpawn_caller` | `raw` | `0x7FFC00BC32A0` | `0x14D32A0` | `4C 8B DC 55 56 48 83 EC 78 49 8B 68 08 48 8B F1 48 85 ED 0F 84 72 01 00 00` |
| `DrawCrosshair` | `raw` | `0x7FFBFFEA0B60` | `0x7B0B60` | `48 89 5C 24 08 57 48 83 EC 20 48 8B D9 E8 ? ? ? ? 48 85` |
| `DrawOverHead` | `raw` | `0x7FFC00155590` | `0xA65590` | `40 53 48 83 EC ? 48 8B D9 83 FA ? 75 ? 48 8B 0D ? ? ? ? 48 8D 54 24 ? 48 8B 01 FF 90 ? ? ? ? 8B 10` |
| `DrawScopeOverlay` | `raw` | `0x7FFBFFF4BFF0` | `0x85BFF0` | `48 8B C4 53 57 48 83 EC ? 48 8B FA` |
| `DrawSmokeVertex` | `raw` | `0x7FFC00368D90` | `0xC78D90` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 48 8B 9C 24 ? ? ? ? 4D 8B F8` |
| `FX_FireBullets` | `raw` | `0x7FFC0036BE80` | `0xC7BE80` | `48 8B C4 4C 89 48 20 48 89 50 10 55 53 57 41 54 41 55 48 8D A8 58 FB FF FF 48 81 EC A0 05` |
| `FX_FireBullets` | `raw` | `0x7FFC0036BE80` | `0xC7BE80` | `48 8B C4 4C 89 48 20 48 89 50 10 55 53 57 41 54 41 55 48 8D A8 58 FB FF FF 48 81 EC A0 05 00 00` |
| `FindHudElement` | `raw` | `0x7FFC004AF7E8` | `0xDBF7E8` | `48 8D 15 ? ? ? ? 45 33 C0 B9 ? ? ? ? FF 15 ? ? ? ? EB ? 48 8B 15` |
| `FindHudElement_panorama` | `raw` | `0x7FFC004B17C0` | `0xDC17C0` | `4C 8B DC 53 48 83 EC 50 48 8B 05` |
| `FindSOCache` | `raw` | `0x7FFC00F0C810` | `0x181C810` | `48 89 5C 24 08 57 48 83 EC 30 4C 8B 52 08 48 8B D9 8B 0A` |
| `FirstPersonLegs` | `raw` | `0x7FFC007DDAA0` | `0x10EDAA0` | `40 55 53 56 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? F2 0F 10 42` |
| `FlashOverlay` | `raw` | `0x7FFC00498C10` | `0xDA8C10` | `85 D2 0F 88 ? ? ? ? 48 89 4C 24` |
| `ForceButtonsDown` | `raw` | `0x7FFC000BE9D0` | `0x9CE9D0` | `40 53 57 41 56 48 81 EC ? ? ? ? 48 83 79` |
| `GameEntitySystemPtr` | `riprel` | `0x7FFC01BBED50` | `0x24CED50` | `48 8B 1D ? ? ? ? 48 89 1D ? ? ? ?` |
| `GameEventManager_AddListener` | `raw` | `0x7FFC00028970` | `0x938970` | `48 89 5C 24 10 48 89 6C 24 18 56 57 41 56 48 83 EC 50 41 0F B6 E9 48 8D 99 E0 00 00 00 49 8B F0` |
| `GameEventManager_UnserializeEvent` | `raw` | `0x7FFC000811A0` | `0x9911A0` | `48 8B C4 48 89 50 10 55 41 54 41 55 41 56 48 8D 68 D8 48 81 EC 08 01 00 00 48 89 58 D8 4C 8D B1` |
| `GameRules_ptr` | `riprel` | `0x7FFC01A18F38` | `0x2328F38` | `48 8B 1D ? ? ? ? 48 8D 54 24 ? 0F 28 D0 48 8D 4C 24 ?` |
| `GetBBox_ptr` | `riprel` | `0x7FFC01A18F38` | `0x2328F38` | `48 8B 0D ? ? ? ? 48 85 C9 74 ? ? ? ? 48 FF A0 ? ? ? ? 48 8D 05` |
| `GetBaseEntity` | `raw` | `0x7FFC00055EF0` | `0x965EF0` | `4C 8D 49 ? 81 FA` |
| `GetBonePositionByName` | `raw` | `0x7FFBFFFB6B60` | `0x8C6B60` | `40 53 48 83 EC ? 48 8B 89 ? ? ? ? 48 8B DA 48 8B 01 FF 50 ? 48 8B C8` |
| `GetChatObject` | `rel32` | `0x7FFC007B0DB0` | `0x10C0DB0` | `E8 ? ? ? ? 48 8B E8 48 85 C0 0F 84 ? ? ? ? 4C 8D 05` |
| `GetClientSystem` | `rel32` | `0x7FFC00723CB0` | `0x1033CB0` | `E8 ? ? ? ? 48 8B C8 E8 ? ? ? ? 8B D8 85 C0 74 33` |
| `GetControllerCmd` | `raw` | `0x7FFBFFFAC580` | `0x8BC580` | `40 53 48 83 EC 20 8B DA E8 ? ? ? ? 4C` |
| `GetEconItemSystem` | `raw` | `0x7FFBFFA69780` | `0x379780` | `48 83 EC 28 48 8B 05 ? ? ? ? 48 85 C0 0F 85 ? ? ? ? 48 89 5C 24` |
| `GetEntityHandle` | `raw` | `0x7FFC0003D1C0` | `0x94D1C0` | `48 85 C9 74 32 48 8B 49 10 48 85 C9 74 29 44 8B 41 10 BA` |
| `GetGlowColor` | `raw` | `0x7FFC001F9460` | `0xB09460` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B F2 48 8B F9 48 8B 54 24` |
| `GetInstanceS` | `riprel` | `0x7FFC019A75E0` | `0x22B75E0` | `48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 8B 91 ? ? ? ? B8` |
| `GetInt2_Event` | `raw` | `0x7FFBFFB9AA90` | `0x4AAA90` | `48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC 20 48 63 FA 41` |
| `GetInventoryManager` | `rel32` | `0x7FFBFFEB6370` | `0x7C6370` | `E8 ? ? ? ? 48 8B D3 48 8B C8 4C 8B 00 41 FF 90 00 02` |
| `GetLocalControllerById` | `raw` | `0x7FFBFFFCF9F0` | `0x8DF9F0` | `48 83 EC 28 83 F9 FF 75 ? 48 8B 0D ? ? ? ? 48 8D 54 24 ? 48 8B 01 FF 90 ? ? ? ? 8B 08 48 63 C1 4C 8D 05` |
| `GetLocalPlayer_dispatcher` | `raw` | `0x7FFBFFA69150` | `0x379150` | `48 83 EC 38 48 8B 05 ? ? ? ? 48 85 C0 0F 85 14 06 00 00 48 89 5C 24 40 B9 50 00 00 00 48 89` |
| `GetMatrixForView` | `raw` | `0x7FFBFF859BA0` | `0x169BA0` | `40 53 48 83 EC 60 0F 29 74 24 50 0F 57 DB F3 0F 10 ? ? ? ? ? 49 8B D8` |
| `GetPlayerByIndex_export` | `raw` | `0x7FFC005EE180` | `0xEFE180` | `48 83 EC 28 4C 8D 05 ? ? ? ? 48 8D 15 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 4C 8D` |
| `GetPlayerInterp` | `raw` | `0x7FFBFFFA7DE0` | `0x8B7DE0` | `40 53 48 83 EC ? 48 8B D9 48 8B 0D ? ? ? ? 48 83 C1` |
| `GetRemovedAimPunch_E8` | `rel32` | `0x7FFBFFF3C2F0` | `0x84C2F0` | `E8 ? ? ? ? 4C 8B C0 48 8D 55 ? 48 8B CB E8 ? ? ? ? 48 8D 0D` |
| `GetRemovedAimpunch` | `raw` | `0x7FFBFF802897` | `0x112897` | `F2 0F 10 44 24 ? F2 0F 11 84 24 ? ? ? ? FF 15` |
| `GetSurfaceData` | `rel32` | `0x7FFC00041E30` | `0x951E30` | `E8 ? ? ? ? 80 78 18 00` |
| `GetTickBase` | `rel32` | `0x7FFBFFFAC380` | `0x8BC380` | `E8 ? ? ? ? EB ? 48 8B 05 ? ? ? ? 8B 40` |
| `GetTraceInfo` | `raw` | `0x7FFBFFEF63B0` | `0x8063B0` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B E9 0F 29 74 24 ? 48 8B CA` |
| `GetUserCmdManager` | `raw` | `0x7FFBFFFAC610` | `0x8BC610` | `41 56 41 57 48 83 EC ? 48 8D 54 24` |
| `GetViewAngles` | `raw` | `0x7FFC001C4540` | `0xAD4540` | `4C 8B C1 85 D2 74 08 48 8D 05 ? ? ? ? C3` |
| `GetWeaponInAccuracyRecoveryTime` | `rel32` | `0x7FFBFFE86570` | `0x796570` | `E8 ? ? ? ? F3 0F 10 B7 ? ? ? ? F3 0F 5E F8` |
| `GetWorldFovResolver` | `raw` | `0x7FFBFFEFBE50` | `0x80BE50` | `40 53 48 83 EC 50 48 8B D9 E8 ? ? ? ? 48 85 C0 74 ? 48 8B C8 48 83 C4 50 5B E9` |
| `GlobalVariables_ptr` | `riprel` | `0x7FFC017396A0` | `0x20496A0` | `48 89 15 ? ? ? ? 48 89 42` |
| `GloveApply_PerTick` | `raw` | `0x7FFC002AFB00` | `0xBBFB00` | `40 55 56 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B B9 A0 00 00 00` |
| `GlowObjectManager_GetInstance` | `raw` | `0x7FFC001F9570` | `0xB09570` | `48 8B 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 8B 41 38 C3` |
| `HandleBulletPenetration` | `raw` | `0x7FFBFFF10210` | `0x820210` | `48 8B C4 44 89 48 ? 48 89 50 ? 48 89 48 ? 55` |
| `HandleEntityList` | `rel32` | `0x7FFBFF8B3650` | `0x1C3650` | `E8 ? ? ? ? 84 C0 74 ? 48 63 03` |
| `HandleTeamIntro` | `raw` | `0x7FFBFFDF3E00` | `0x703E00` | `48 83 EC ? ? ? ? ? 44 38 89` |
| `HudChatPrintf` | `rel32` | `0x7FFC007AE830` | `0x10BE830` | `E8 ? ? ? ? 49 8B 4E 20 BA ? ? ? ?` |
| `InfoForResourceTypeCCompositeMaterialKit_TypeManager` | `stringref` | `0x7FFC00AC6840` | `0x13D6840` | `"InfoForResourceTypeCCompositeMaterialKit"` |
| `InfoForResourceTypeCCompositeMaterial_TypeManager` | `raw` | `0x7FFC00AC6D90` | `0x13D6D90` | `40 55 41 56 48 83 EC 68 48 8B EA 83 F9 06 0F 87 B4 02 00 00` |
| `InitFilter` | `raw` | `0x7FFBFFA1BB40` | `0x32BB40` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 0F B6 41 ? 33 FF 24 C9 C7 41 ?` |
| `InitPlayerMovementTraceFilter` | `raw` | `0x7FFBFFF2F270` | `0x83F270` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 0F B6 41 ? 33 FF C7 41 ?` |
| `InitTraceInfo` | `raw` | `0x7FFC00CE9A30` | `0x15F9A30` | `40 55 41 55 41 57 48 83 EC` |
| `IsGlowing` | `rel32` | `0x7FFC001FABA0` | `0xB0ABA0` | `E8 ? ? ? ? 33 DB 84 C0 0F 84 ? ? ? ? 48 8B 4F` |
| `LevelInit` | `raw` | `0x7FFBFFFBEA80` | `0x8CEA80` | `40 55 56 41 56 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48` |
| `LoadFileForMe` | `raw` | `0x7FFC0000A8C0` | `0x91A8C0` | `40 55 57 41 56 48 83 EC 20 4C` |
| `LoadPath` | `rel32` | `0x7FFBFFDAB150` | `0x6BB150` | `E8 ? ? ? ? 8B 44 24 2C` |
| `LookupBone` | `rel32` | `0x7FFBFFFB6B60` | `0x8C6B60` | `E8 ? ? ? ? 48 8B 8D ? ? ? ? B3` |
| `ModulationUpdate` | `raw` | `0x7FFC000C8CF0` | `0x9D8CF0` | `48 89 5C 24 08 57 48 83 EC 20 8B FA 48 8B D9 E8 ? ? ? ? 84 C0 0F 84` |
| `NoClipOnChange` | `raw` | `0x7FFBFF856B50` | `0x166B50` | `48 89 5C 24 10 48 89 74 24 18 48 89 7C 24 20 55 48 8B EC 48 83 EC 30 48 8D 05` |
| `NoSpread1` | `raw` | `0x7FFC0036BDD0` | `0xC7BDD0` | `48 89 5C 24 08 57 48 81 EC F0 00` |
| `ParticleCollection` | `raw` | `0x7FFBFF8E4CE0` | `0x1F4CE0` | `48 89 5C 24 ? 57 48 83 EC ? 0F 28 05` |
| `ParticleManager_ptr` | `riprel` | `0x7FFC0171DAA8` | `0x202DAA8` | `48 8B 0D ? ? ? ? 41 B8 ? ? ? ? F3 0F 11 74 24 ? 48 C7 44 24 ? ? ? ? ?` |
| `PhysicsRunThink_Ctrl` | `raw` | `0x7FFBFFFC5C90` | `0x8D5C90` | `48 89 5C 24 ? 57 48 81 EC ? ? ? ? ? ? ? 48 8B F9 FF 90` |
| `PhysicsRunThink_Pawn` | `raw` | `0x7FFC001FD5F0` | `0xB0D5F0` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 8B 81 ? ? ? ? 48 8B F9` |
| `PlayVSound_client` | `raw` | `0x7FFC00BFC490` | `0x150C490` | `48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 48 8D 6C 24 ? 48 81 EC ? ? ? ? 33 FF` |
| `ProcessImpacts` | `raw` | `0x7FFC000BD2F0` | `0x9CD2F0` | `48 8B C4 53 56 41 55` |
| `ProcessMovement` | `rel32` | `0x7FFC000C82D0` | `0x9D82D0` | `E8 ? ? ? ? 48 8B 06 48 8B CE FF 90 ? ? ? ? 48 85 DB` |
| `RegenerateWeaponSkin` | `raw` | `0x7FFBFFE7C050` | `0x78C050` | `40 55 53 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 44 0F B6 FA 48 8B D9 BA ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ?` |
| `RegenerateWeaponSkin_v2` | `raw` | `0x7FFBFFE7C050` | `0x78C050` | `40 55 53 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 44 0F B6 FA 48 8B D9 BA ? ? ? ? 48 8D 0D ? ? ? ? E8` |
| `RegenerateWeaponSkins` | `raw` | `0x7FFBFFEA0CB0` | `0x7B0CB0` | `48 83 EC ? E8 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? 48 8B 10` |
| `ReportHit` | `rel32` | `0x7FFBFFCF21E0` | `0x6021E0` | `E8 ? ? ? ? 48 8B AC 24 D8 00 00 00 48 81 C4` |
| `RunCommand` | `raw` | `0x7FFC000CA390` | `0x9DA390` | `48 8B C4 48 81 EC ? ? ? ? 48 89 58 10` |
| `RunCommand_processor` | `raw` | `0x7FFC000CA390` | `0x9DA390` | `48 8B C4 48 81 EC C8 00 00 00 48 89 58 10 48 89 68 18 48 8B EA 48 89 70 20 48 8B F1 48 89 78 F8` |
| `Scope_callsite` | `rel32` | `0x7FFBFFF4BFF0` | `0x85BFF0` | `E8 ? ? ? ? 80 7C 24 34 ? 74 ?` |
| `SendChatMessage` | `rel32` | `0x7FFC007AE830` | `0x10BE830` | `E8 ? ? ? ? 49 8B 4E 20 BA ? ? ? ?` |
| `SetAbsOrigin_Pawn` | `raw` | `0x7FFBFF90EEA0` | `0x21EEA0` | `48 89 5C 24 ? 57 48 83 EC ? ? ? ? 48 8B FA 48 8B D9 FF 90 ? ? ? ? 84 C0 0F 85` |
| `SetBodyGroup_inv` | `raw` | `0x7FFC00484BF0` | `0xD94BF0` | `85 D2 0F 88 ? ? ? ? 53 55` |
| `SetCollisionBounds` | `raw` | `0x7FFBFFEF3540` | `0x803540` | `48 83 EC ? F2 0F 10 02 8B 42 08` |
| `SetDynamicAttributeValue` | `raw` | `0x7FFC006F26A0` | `0x10026A0` | `48 89 6C 24 ? 57 41 56 41 57 48 81 EC ? ? ? ? 48 8B FA C7 44 24 ? ? ? ? ? 4D 8B F8` |
| `SetDynamicAttributeValue_raw` | `raw` | `0x7FFC006F26A0` | `0x10026A0` | `48 89 6C 24 ? 57 41 56 41 57 48 81 EC ? ? ? ? 48 8B FA C7 44 24` |
| `SetMeshGroupMask` | `raw` | `0x7FFC0011C3F0` | `0xA2C3F0` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8D 99 ? ? ? ? 48 8B 71` |
| `SetModel` | `raw` | `0x7FFBFFFC9B40` | `0x8D9B40` | `40 53 48 83 EC ? 48 8B D9 4C 8B C2 48 8B 0D ? ? ? ? 48 8D 54 24` |
| `SetPlayerReady` | `raw` | `0x7FFC0060B510` | `0xF1B510` | `40 53 48 83 EC 20 48 8B DA 48 8D 15 ? ? ? ? 48 8B CB FF 15 ? ? ? ? 85 C0 75 14 BA` |
| `SetTraceData` | `rel32` | `0x7FFBFFEC4750` | `0x7D4750` | `E8 ? ? ? ? 8B 85 ? ? ? ? 48 8D 54 24 ? F2 0F 10 45` |
| `SetTypeKV3` | `raw` | `0x7FFC00F08640` | `0x1818640` | `40 53 48 83 EC 30 4C 8B 11 41 B9 ? ? ? ? 49 83 CA 01 0F B6 C2 80 FA 06 48 8B D9 44 0F 45 C8` |
| `SetViewAngle` | `raw` | `0x7FFC001D3580` | `0xAE3580` | `85 D2 75 3D 48 63 81 ? ? ? ?` |
| `SetupCmd` | `raw` | `0x7FFBFFFA98A0` | `0x8B98A0` | `48 83 EC 28 E8 ? ? ? ? 8B 80` |
| `SetupMove` | `raw` | `0x7FFC0040AA50` | `0xD1AA50` | `48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 83 EC ? 48 8B EA 4C 8B F1 E8 ? ? ? ? 48 8D 15` |
| `SetupMovementMoves` | `raw` | `0x7FFC0087441F` | `0x118441F` | `48 8B ? E8 ? ? ? ? 48 8B 5C 24 ? 48 8B 6C 24 ? 48 83 C4 30` |
| `SomeTimingFromPawn` | `raw` | `0x7FFC00145B50` | `0xA55B50` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 49 63 D8 48 8B F1` |
| `Spawner_PerTickOrchestrator` | `raw` | `0x7FFC002B2680` | `0xBC2680` | `48 8B C4 55 53 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 80 B9 B1 13 00 00 00` |
| `SpectatorInput` | `raw` | `0x7FFBFFEC9130` | `0x7D9130` | `48 89 5C 24 10 55 56 57 41 56 41 57 48 8B EC 48 83 EC 60 48 8B 01 41 8B F8 48 8B DA 48 8B F1 FF` |
| `TestSurfaces` | `raw` | `0x7FFBFFEF6290` | `0x806290` | `40 53 57 41 56 48 83 EC 50 8B` |
| `TracePlayerBBox` | `raw` | `0x7FFC0025F6D0` | `0xB6F6D0` | `48 89 5C 24 ? 55 57 41 54 41 55 41 56` |
| `TraceShape` | `raw` | `0x7FFC0007D340` | `0x98D340` | `48 89 5C 24 ? 48 89 4C 24 ? 55 57` |
| `TraceShape_Client` | `raw` | `0x7FFC0007D340` | `0x98D340` | `48 89 5C 24 20 48 89 4C 24 08 55 57 41 54 41 55 41 56 48 8D AC 24 10 E0 FF FF B8 F0 20 00 00` |
| `TraceToExit` | `raw` | `0x7FFBFFEF4400` | `0x804400` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? F2 0F 10 02` |
| `UpdatePostProcessing` | `raw` | `0x7FFC0060F6A0` | `0xF1F6A0` | `48 85 D2 0F 84 ? ? ? ? 48 89 5C 24 08 57 48 83 EC 60 80` |
| `UpdateSubClass` | `raw` | `0x7FFBFF8EA88B` | `0x1FA88B` | `48 8B 41 10 48 8B D9 8B 50 30` |
| `UpdateTurningInAccuracy` | `rel32` | `0x7FFBFFE9FD10` | `0x7AFD10` | `E8 ? ? ? ? F3 0F 10 87 ? ? ? ? 44 0F 2F C8` |
| `VPhys2World_ptr` | `riprel` | `0x7FFC0171D718` | `0x202D718` | `4C 8B 25 ? ? ? ? 24` |
| `ViewModelHideZoomed` | `raw` | `0x7FFBFFE903D0` | `0x7A03D0` | `48 89 5C 24 20 55 56 57 41 54 41 56 48 8B EC 48 83 EC 50 48 8D 05` |
| `ViewRender_ptr` | `riprel` | `0x7FFC01A1DCB8` | `0x232DCB8` | `48 89 05 ? ? ? ? 48 8B C8 48 85 C0` |
| `WriteSubtickFromEntry` | `raw` | `0x7FFC00343E10` | `0xC53E10` | `48 89 5C 24 ? 55 57 41 56 48 8D 6C 24 ? 48 81 EC B0 00 00 00 8B 01 48 8B F9 81 4A 10 00 02` |
| `create_move_v2` | `raw` | `0x7FFC001BA9C0` | `0xACA9C0` | `85 D2 0F 85 ? ? ? ? 48 8B C4 44 88 40` |
| `draw_smoke_array` | `raw` | `0x7FFC00368E80` | `0xC78E80` | `40 55 41 54 41 55 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 4C 8B E2` |
| `draw_view_punch_v2` | `raw` | `0x7FFBFFEF3CC0` | `0x803CC0` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 49 8B E9 49 8B F8` |
| `entity_list_ptr` | `riprel` | `0x7FFC01BBEE58` | `0x24CEE58` | `48 8B 1D ? ? ? ? 48 8D 46` |
| `frame_stage_notify` | `raw` | `0x7FFC001C15D1` | `0xAD15D1` | `4C 8B 0D ? ? ? ? 48 8D 15 ? ? ? ? 48 8B 8F ? ? ? ? F3 41 0F 10 51 ? 45 8B 49 ? 0F 5A D2 66 49 0F 7E D0 FF 15 ? ? ? ? 48 8B 97 ? ? ? ? 48 8B 0D ? ? ? ? E8 ? ? ? ? E9` |
| `get_fov` | `raw` | `0x7FFBFFEF3CC0` | `0x803CC0` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 49 8B E9 49 8B F8` |
| `get_map_name` | `raw` | `0x7FFC005CADF0` | `0xEDADF0` | `48 83 EC ? 48 8B 0D ? ? ? ? ? ? ? FF 90 ? ? ? ? 48 8B C8 48 83 C4` |
| `get_view_angles_v2` | `raw` | `0x7FFC001C2EA0` | `0xAD2EA0` | `4D 85 C0 74 ? 85 D2 74` |
| `get_view_model` | `raw` | `0x7FFBFFF3E040` | `0x84E040` | `40 55 53 56 41 56 41 57 48 8B EC` |
| `global_vars_v2` | `riprel` | `0x7FFC01A18F38` | `0x2328F38` | `48 89 1D ? ? ? ? FF 15 ? ? ? ? 84 C0 74 ? 8B 0D ? ? ? ? 4C 8D 0D ? ? ? ? 4C 8D 05 ? ? ? ? BA ? ? ? ? FF 15 ? ? ? ? 48 8B 74 24 ? 48 8B C3` |
| `is_demo_or_hltv` | `raw` | `0x7FFC005EC220` | `0xEFC220` | `48 83 EC ? 48 8B 0D ? ? ? ? ? ? ? FF 90 ? ? ? ? 84 C0 75 ? 38 05` |
| `level_init_v2` | `raw` | `0x7FFC001E9230` | `0xAF9230` | `40 55 56 41 56 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 0D` |
| `level_shutdown` | `raw` | `0x7FFC001E94B0` | `0xAF94B0` | `48 83 EC ? 48 8B 0D ? ? ? ? 48 8D 15 ? ? ? ? 45 33 C9 45 33 C0 ? ? ? FF 50 ? 48 85 C0 74 ? 48 8B 0D ? ? ? ? 48 8B D0 ? ? ? 41 FF 50 ? 48 83 C4` |
| `local_controller` | `riprel` | `0x7FFC019F8520` | `0x2308520` | `48 8B 05 ? ? ? ? 41 89 BE` |
| `mark_interp_latch_flags_dirty` | `raw` | `0x7FFBFF907FC0` | `0x217FC0` | `40 53 56 57 48 83 EC ? 80 3D ? ? ? ? 00` |
| `on_add_entity_v2` | `raw` | `0x7FFC000574A0` | `0x9674A0` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 8B 81 ? ? ? ? 49 8B F0` |
| `override_view_short` | `raw` | `0x7FFC0034D2F0` | `0xC5D2F0` | `40 57 48 83 EC ? 48 8B FA E8 ? ? ? ? BA` |
| `paintkit_prefab` | `stringref` | `0x7FFC0074AAF0` | `0x105AAF0` | `"set item texture prefab"` |
| `paintkit_seed` | `stringref` | `0x7FFC005DEC20` | `0xEEEC20` | `"set item texture seed"` |
| `paintkit_wear` | `stringref` | `0x7FFC005DEC20` | `0xEEEC20` | `"set item texture wear"` |
| `planted_c4_ptr` | `riprel` | `0x7FFC01996CB8` | `0x22A6CB8` | `48 8B 15 ? ? ? ? 48 8B 5C 24 ? FF C0 89 05 ? ? ? ? 48 8B C6 ? ? ? ? 80 BE ? ? ? ? 00` |
| `remove_legs` | `raw` | `0x7FFC007DDAA0` | `0x10EDAA0` | `40 55 53 56 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? F2 0F 10 42` |
| `statTrak_killEater` | `stringref` | `0x7FFC005DEC20` | `0xEEEC20` | `"kill eater"` |
| `statTrak_scoreType` | `stringref` | `0x7FFBFF80B740` | `0x11B740` | `"kill eater score type"` |
| `unlock_inventory` | `raw` | `0x7FFBFFDF1110` | `0x701110` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B E9 48 8B 0D ? ? ? ? ? ? ? FF 50` |
| `update_global_vars` | `raw` | `0x7FFC001D2FD0` | `0xAE2FD0` | `48 8B 0D ? ? ? ? 4C 8D 05 ? ? ? ? 48 85 D2` |
| `update_post_processing_v2` | `raw` | `0x7FFC00613C56` | `0xF23C56` | `48 89 AC 24 ? ? ? ? 45 33 ED` |
| `view_matrix_ptr` | `riprel` | `0x7FFC01A1EAC0` | `0x232EAC0` | `48 8D 0D ? ? ? ? 48 89 44 24 ? 48 89 4C 24 ? 4C 8D 0D` |

## `engine2.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `CCommand_Tokenize` | `raw` | `0x7FFC1306D710` | `0x3FD710` | `48 89 6C 24 20 4C 89 44 24 18 56 57 41 54 41 56 41 57 48 83 EC 70 48 8B F2 49 8B E8 8B 51 08 4C` |
| `CGameClient_ClientCommand` | `raw` | `0x7FFC12D11240` | `0xA1240` | `48 8B C4 4C 89 40 18 4C 89 48 20 55 53 57 48 8D 68 A1 48 81 EC C0 00 00 00 33 FF 48 63 DA 48 39` |
| `CHLTVClient_ExecuteStringCommand` | `raw` | `0x7FFC12D90D70` | `0x120D70` | `40 53 56 48 81 EC 48 07 00 00 48 8B F1 48 8B DA 48 8B 4A 48 48 83 E1 FC 48 83 79 18 0F 76 03 48` |
| `CSplitScreenSlot` | `stringref` | `0x7FFC12EBA250` | `0x24A250` | `"CSplitScreenSlot"` |
| `Cvar_RegisterConCommand` | `raw` | `0x7FFC1306D270` | `0x3FD270` | `48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 60 44 8B 15 ? ? ? ? 48 8B D9 65 48` |
| `Cvar_RegisterConVar` | `raw` | `0x7FFC1306C080` | `0x3FC080` | `48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 48 89 7C 24 20 41 54 41 56 41 57 48 81 EC D0 00 00` |
| `Engine::GetScreenAspectRatio` | `raw` | `0x7FFC12CE69D0` | `0x769D0` | `48 89 5C 24 08 57 48 83 EC 20 8B FA 48 8D 0D` |
| `Engine::PVSManager_ptr` | `riprel` | `0x7FFC132833F0` | `0x6133F0` | `48 8D 0D ? ? ? ? 33 D2 FF 50` |
| `Engine::RunPrediction` | `raw` | `0x7FFC12CD6490` | `0x66490` | `40 55 41 56 48 83 EC ? 80 B9` |
| `Engine_Disconnect_main` | `raw` | `0x7FFC12E41510` | `0x1D1510` | `48 89 5C 24 20 55 57 41 54 48 8B EC 48 83 EC 70 45 33 E4 48 C7 05` |
| `Engine_HLTVClient_ExecuteStringCommand` | `raw` | `0x7FFC12D90D70` | `0x120D70` | `40 53 56 48 81 EC 48 07 00 00 48 8B F1 48 8B DA 48 8B 4A 48 48 83 E1 FC 48 83 79 18 0F 76 03 48` |
| `Engine_HostStateMgr_QueueNewRequest` | `raw` | `0x7FFC12E8AFC0` | `0x21AFC0` | `48 89 6C 24 18 48 89 7C 24 20 41 56 48 83 EC 30 48 8B EA 48 8B F9 8B 0D ? ? ? ? BA 02 00 00` |
| `Engine_HostStateMgr_QueueNewRequest` | `raw` | `0x7FFC12E8AFC0` | `0x21AFC0` | `48 89 6C 24 18 48 89 7C 24 20 41 56 48 83 EC 30 48 8B EA 48 8B F9 8B 0D ? ? ? ? BA 02 00 00` |
| `Engine_LoadGameInfo` | `raw` | `0x7FFC12DFD760` | `0x18D760` | `40 55 56 41 56 48 8D 6C 24 F0 48 81 EC 10 01 00 00 4C 8B F1 C7 44 24 40 00 00 00 00 48 8B CA 48` |
| `Engine_MountAddon` | `raw` | `0x7FFC12E03440` | `0x193440` | `48 85 D2 0F 84 DA 0A 00 00 48 8B C4 44 88 40 18 55 57 41 54 41 57 48 8D A8 C8 FC FF FF 48 81 EC` |
| `Engine_NetTimeoutDisconnect` | `raw` | `0x7FFC12CD9780` | `0x69780` | `40 53 55 56 57 41 56 48 81 EC 80 00 00 00 0F 29 74 24 70 49 8B F8` |
| `Engine_NetworkGameClient_Connect` | `raw` | `0x7FFC12CEF400` | `0x7F400` | `48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 40 44 89 81 3C 02 00 00 49 8B E9 44 8B` |
| `Engine_NetworkGameClient_SetSignonState` | `raw` | `0x7FFC12CD0F80` | `0x60F80` | `44 89 44 24 18 89 54 24 10 55 53 56 57 41 55 41 56 41 57 48 8D 6C 24 D9 48 81 EC D0 00 00 00 8B` |
| `Engine_RegisterConCommand` | `raw` | `0x7FFC1306D270` | `0x3FD270` | `48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 60 44 8B 15` |
| `Engine_RegisterConVar` | `raw` | `0x7FFC1306C080` | `0x3FC080` | `48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 48 89 7C 24 20 41 54 41 56 41 57 48 81 EC D0 00 00` |

## `materialsystem2.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `CMaterial2_CompileComboAndGetVariables_DynamicShaderCompile` | `stringref` | `0x7FFC11943FA0` | `0x13FA0` | `"CompileComboAndGetVariables_DynamicShaderCompile(), C:\buildworker\csgo_rel_win64\build\src\materialsystem2\material2.cpp:2786"` |
| `CMaterial2_GetVertexShaderInputSignature` | `stringref` | `0x7FFC1193C8C0` | `0xC8C0` | `"CMaterial2::GetVertexShaderInputSignature(767): Error! Material "%s" doesn't have any valid layers to get the CVsInputSignatureVector from!\n"` |
| `CMaterial2_LoadShadersAndSetupModes` | `raw` | `0x7FFC11940040` | `0x10040` | `44 89 44 24 18 48 89 54 24 10 53 56 41 54 41 55 48 81 EC 88 00 00 00 4C 8B E9 48 C7 44 24 60` |
| `CMaterialLayer_ApplyMaterialVarsForBatch` | `raw` | `0x7FFC11948B80` | `0x18B80` | `4C 89 4C 24 20 4C 89 44 24 18 48 89 54 24 10 53 55 56 57 41 54 41 55 41 56 41 57 48 83 EC 78` |
| `CMaterialLayer_BuildPassCommandData` | `raw` | `0x7FFC11948F80` | `0x18F80` | `89 54 24 10 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 58 FE FF FF 48 81 EC A8 02 00 00` |
| `CMaterialLayer_ComputeWorkItemsToSetupStaticCombosForMode` | `stringref` | `0x7FFC11945F3C` | `0x15F3C` | `"CMaterialLayer::ComputeWorkItemsToSetupStaticCombosForMode(3154): Failed call to FindOrLoadStaticComboData()!\n"` |
| `CMaterialLayer_CreateCommandBuffer` | `stringref` | `0x7FFC11949820` | `0x19820` | `"\nCMaterialLayer::CreateCommandBuffer(4446): Find a graphics programmer! Trying to bind a "%s" shader that doesn't exist! for %s\n"` |
| `CMaterialSystem2_BindIdentityInstanceIDBufferAndSetRenderState` | `stringref` | `0x7FFC119A0000` | `0x70000` | `"BindIdentityInstanceIDBufferAndSetRenderState: GetMode == NULL? Can't Render\n"` |
| `CMaterialSystem2_DynamicShaderCompile_ProcessQueue` | `stringref` | `0x7FFC1196A5E0` | `0x3A5E0` | `"Compiling %i shaders:"` |
| `CMaterialSystem2_DynamicShaderCompile_ReloadAndSync` | `raw` | `0x7FFC119655C1` | `0x355C2` | `48 83 EC 20 48 8B 35 ? ? ? ? 48 8B CE E8 ? ? ? ? 48 8B CE E8 ? ? ? ? 80 BE A0 03 00 00 00 74 ?` |
| `CMaterialSystem2_DynamicShaderCompile_UnloadAllMaterials` | `stringref` | `0x7FFC11969AA0` | `0x39AA0` | `"CMaterialSystem2::DynamicShaderCompile_UnloadAllMaterials(1084): ERROR!!! Shaders not freed before shader reload! (See spew above)\n\n"` |
| `CMaterialSystem2_FrameUpdate` | `raw` | `0x7FFC1196BAC0` | `0x3BAC0` | `48 89 4C 24 08 55 53 56 57 41 54 41 56 48 8B EC 48 83 EC 68 48 8D 05 ? ? ? ? 48 C7 45 C0` |
| `CMaterialSystem2_GetErrorMaterial` | `stringref` | `0x7FFC119474D7` | `0x174D7` | `"CMaterialSystem2::GetErrorMaterial(529): GetErrorMaterial() called when m_pMaterialTypeManager == NULL!\n"` |
| `CMaterialSystem2_Init` | `stringref` | `0x7FFC11966E40` | `0x36E40` | `"MaterialSystem2"` |
| `CMaterial_SetVariableAndRenderState` | `stringref` | `0x7FFC1195F9B0` | `0x2F9B0` | `"SetRenderStateValueFromVariable(1172): Unsupported render state type in material "%s"!\n"` |
| `CVfxProgramData_FindOrCreateStaticComboDataInCache` | `stringref` | `0x7FFC119DE0E0` | `0xAE0E0` | `"CVfxProgramData::FindOrCreateStaticComboDataInCache(4448): Error! Ref count !=0 for static combo data cache entry!\n"` |
| `CVfxProgramData_FindOrCreateStaticComboData_CacheGate` | `raw` | `0x7FFC119DE950` | `0xAE950` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 57 48 83 EC 60 80 39 00 45 8B D9` |
| `CVfxProgramData_FindOrLoadStaticComboData` | `stringref` | `0x7FFC119EDAE0` | `0xBDAE0` | `"Shader %s attribute "%s" has inconsistent value or type across multiple shaders of a feature combo! ["` |
| `FindParameter` | `raw` | `0x7FFC11941E30` | `0x11E30` | `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B 59 20 48` |
| `MatSys::PrepareSceneMaterial` | `raw` | `0x7FFC11941BE0` | `0x11BE0` | `48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 30 48 8B 59 ? 48 8B F2 48 63 79 ? 48 C1 E7 06` |
| `UpdateParameter` | `raw` | `0x7FFC11942370` | `0x12370` | `48 89 7C 24 ? 41 56 48 83 EC ? 8B 81` |

## `networksystem.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `CNetChan_ProcessMessages` | `raw` | `0x7FFC0FC1B280` | `0xBB280` | `48 8B C4 53 57 41 54 41 56 48 81 EC A8 00 00 00 48 89 70 D0 45 33 E4 4C 89 68 C8 48 8B D9 48 89` |
| `CNetChan_SendNetMessage` | `raw` | `0x7FFC0FC1D670` | `0xBD670` | `48 89 5C 24 10 48 89 6C 24 18 56 57 41 56 48 83 EC 40 41 0F B6 F0 48 8D 99 F8 73 00 00 4C 8B F2` |
| `NetSystem_CNetChan_ProcessMessages` | `raw` | `0x7FFC0FC1B280` | `0xBB280` | `48 8B C4 53 57 41 54 41 56 48 81 EC A8 00 00 00 48 89 70 D0 45 33 E4 4C 89 68 C8 48 8B D9 48 89` |
| `NetSystem_CNetChan_SendNetMessage` | `raw` | `0x7FFC0FC1D670` | `0xBD670` | `48 89 5C 24 10 48 89 6C 24 18 56 57 41 56 48 83 EC 40 41 0F B6 F0 48 8D 99 F8 73 00 00 4C 8B F2` |

## `particles.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `Particles::DrawArray` | `raw` | `0x7FFC09E520B0` | `0x220B0` | `40 55 53 56 57 48 8D 6C 24` |
| `Particles::FindKeyVar` | `raw` | `0x7FFC09E6A650` | `0x3A650` | `48 89 5C 24 ? 57 48 81 EC ? ? ? ? 33 C0 8B DA` |
| `Particles::SetMaterialShaderType` | `raw` | `0x7FFC09ECD8D0` | `0x9D8D0` | `48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 54 41 56 41 57 48 81 EC ? ? ? ? 4C 63 32` |

## `rendersystemdx11.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `CRenderDeviceBase_CreateConstantBuffer` | `stringref` | `0x7FFC1259F500` | `0x2F500` | `"CRenderDeviceBase::CreateConstantBuffer(1571): "` |
| `CRenderDeviceDx11_BeginSubmittingDisplayLists` | `stringref` | `0x7FFC125AC4E0` | `0x3C4E0` | `"CRenderDeviceDx11::BeginSubmittingDisplayLists(1162): "` |
| `CRenderDeviceDx11_CompileShaderSourceMain` | `stringref` | `0x7FFC125AFAF0` | `0x3FAF0` | `"Shader compilation failed! Reported no errors.\n"` |
| `CSwapChainDx11_QueuePresentAndWait` | `raw` | `0x7FFC125A4650` | `0x34650` | `40 55 53 57 41 54 41 55 48 8D 6C 24 C9 48 81 EC C0 00 00 00 48 8D 05 ? ? ? ? 4C 89 B4 24` |
| `CSwapChainDx11_ResizeBuffers` | `raw` | `0x7FFC125ADD20` | `0x3DD20` | `48 8B C4 55 53 56 57 41 54 48 8B EC 48 83 EC 70 4C 89 68 10 4D 8B E0 4C 89 70 18 4C 8B EA 4C 89` |
| `RenderSystemDx11_QueuePresentAndWait` | `raw` | `0x7FFC125A4650` | `0x34650` | `40 55 53 57 41 54 41 55 48 8D 6C 24 C9 48 81 EC C0 00 00 00 48 8D 05 ? ? ? ? 4C 89 B4 24` |
| `RenderSystemDx11_SetHardwareGammaRamp` | `raw` | `0x7FFC125AF790` | `0x3F790` | `48 89 5C 24 18 57 B8 B0 40 00 00 E8 ? ? ? ? 48 2B E0 0F 29 BC 24 90 40 00 00 0F 57 C9 0F 28` |
| `RenderSystemDx11_SetMode` | `raw` | `0x7FFC125A99E0` | `0x399E0` | `44 89 4C 24 20 44 89 44 24 18 89 54 24 10 55 53 56 57 41 54 41 55 41 56 41 57 48 81 EC D8 02 00` |

## `resourcesystem.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `ResourceSystem_BlockingLoadResourceByName` | `raw` | `0x7FFC5B0A7360` | `0x17360` | `40 53 55 57 48 81 EC 80 00 00 00 48 8B 01 49 8B E8 48 8B FA 48 8B D9 FF 90 98 01 00 00 83 F8 03` |
| `ResourceSystem_FindOrRegisterResourceByName` | `raw` | `0x7FFC5B0A6D80` | `0x16D80` | `48 89 5C 24 18 48 89 74 24 20 57 48 81 EC A0 00 00 00 F7 02 FF FF FF 3F 41 0F B6 F8 48 8B DA 48` |
| `ResourceSystem_FrameUpdate` | `raw` | `0x7FFC5B0AC010` | `0x1C010` | `44 88 4C 24 20 44 89 44 24 18 89 54 24 10 55 56 41 54 41 55 41 56 48 8D 6C 24 A0 48 81 EC 60 01` |

## `scenesystem.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `CSceneAnimatableObject::GeneratePrimitives` | `raw` | `0x7FFC0A683520` | `0x73520` | `48 8B C4 48 89 58 08 48 89 50 10 55 56 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ?` |
| `CSceneSkyBoxObject_DrawSkyboxArray` | `raw` | `0x7FFC0A75FB90` | `0x14FB90` | `45 85 C9 0F 8E ? ? ? ? 4C 8B DC 55 41 56 49 8D AB 58 FC FF FF 48 81 EC 98 04 00 00` |
| `CSceneSystem_RenderViewLayer_Dispatch` | `raw` | `0x7FFC0A6FDD80` | `0xEDD80` | `48 8B C4 48 89 48 08 55 53 56 57 41 54 41 55 41 56 41 57 48 8D A8 B8 FE FF FF 48 81 EC 08 02 00` |
| `CSceneSystem_Thread_CullView` | `stringref` | `0x7FFC0A6F92F0` | `0xE92F0` | `"CSceneSystem::Thread_CullView(), C:\buildworker\csgo_rel_win64\build\src\scenesystem\scenesystem.cpp:3312"` |
| `DrawObject_legacy` | `raw` | `0x7FFC0A665BC0` | `0x55BC0` | `48 8B C4 53 57 41 54 48 81 EC D0 00 00 00 49 63 F9 49` |
| `DrawSkyboxArray` | `raw` | `0x7FFC0A75FB90` | `0x14FB90` | `45 85 C9 0F 8E ? ? ? ? 4C 8B DC 55` |
| `SceneSystem::DrawAggeregateObject` | `raw` | `0x7FFC0A73CF50` | `0x12CF50` | `48 8B C4 4C 89 48 20 4C 89 40 ? 48 89 50 ? 55 53 41 57 48 8D A8` |
| `SceneSystem::DrawArrayLight` | `raw` | `0x7FFC0A68AAC0` | `0x7AAC0` | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 54 24` |
| `SceneSystem_Thread_RenderSceneDrawList` | `raw` | `0x7FFC0A6FDA30` | `0xEDA30` | `40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 E1 48 81 EC D8 00 00 00 4C 8B 71 28 48 8B D9` |

## `soundsystem.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `SoundSystem::PlayVSound` | `raw` | `0x7FFC0E729840` | `0x349840` | `48 8B C4 48 89 58 08 57 48 81 EC ? ? ? ? 33 FF 48 8B D9` |
| `SoundSystem::SomeUtlSymbolFunc` | `raw` | `0x7FFC0E490740` | `0xB0740` | `48 89 74 24 18 57 48 83 EC 20 48 63 F2 48 8B F9 3B 71 30` |

## `tier0.dll`

| Name | Resolve | VA | RVA | Pattern |
| --- | --- | --- | --- | --- |
| `Tier0::LoadKeyValues` | `rel32` | `0x7FFC5CF49160` | `0x129160` | `E8 ? ? ? ? 8B 4C 24 34 0F B6 D8` |
| `Tier0::UtlBuffer` | `raw` | `0x7FFC5CE73F10` | `0x53F10` | `48 89 5C 24 ? 57 48 83 EC ? 8B 41 ? 8D 7A` |

