// Dumped by SnipeHype200(frenchbulldogs) [++Fortnite+Release-40.10-CL-52157884]
#pragma once
#include <cstdint>

namespace Offsets
{
    constexpr uintptr_t UWorld_Levels = 0x1f0;
    constexpr uintptr_t UWorld_PersistentLevel = 0x40;
    constexpr uintptr_t UWorld_GameState = 0x1d8;
    constexpr uintptr_t UWorld_OwningGameInstance = 0x250;
    constexpr uintptr_t UGameInstance_LocalPlayers = 0x38;
    constexpr uintptr_t Uplayer_PlayerController = 0x30;
    constexpr uintptr_t APlayerController_AcknowledgedPawn = 0x358;
    constexpr uintptr_t FortPlayerController_TargetedFortPawn = 0x1840;
    constexpr uintptr_t UGameStateBase_PlayerArray = 0x2c8;
    constexpr uintptr_t APawn_PlayerState = 0x2d0;
    constexpr uintptr_t UPlayerState_PawnPrivate = 0x328;
    constexpr uintptr_t AActor_RootComponent = 0x1b0;
    constexpr uintptr_t ACharacter_Mesh = 0x330;
    constexpr uintptr_t AFortPlayerPawn_CurrentVehicle = 0x2d10;
    constexpr uintptr_t ASceneComponent_RelativeLocation = 0x140;
    constexpr uintptr_t ASceneComponent_RelativeRotation = 0x158;
    constexpr uintptr_t ASceneComponent_ComponentVelocity = 0x188;
    constexpr uintptr_t UPlayerState_PlayerNamePrivate = 0x348;
    constexpr uintptr_t UPlayerState_bIsABot = 0x2ba;
    constexpr uintptr_t UFortPlayerState_HabaneroComponent = 0x948;
    constexpr uintptr_t UFortPlayerStateComponent_Habanero_RankedProgress = 0xd8;
    constexpr uintptr_t UFortPlayerState_Platform = 0x440;
    constexpr uintptr_t UFortPlayerStateAthena_KillScore = 0x11d8;
    constexpr uintptr_t UFortPlayerStateAthena_SeasonLevelUIDisplay = 0x11dc;
    constexpr uintptr_t UFortPlayerStateAthena_InitialSquadSize = 0x19b0;
    constexpr uintptr_t UFortPlayerStateAthena_TeamIndex = 0x11c1;
    constexpr uintptr_t UFortPlayerStateAthena_bIsAnAthenaGameParticipant = 0x18cf;
    constexpr uintptr_t AFortPawn_bIsDying = 0x728;
    constexpr uintptr_t AFortPawn_bIsDBNO = 0x841;
    constexpr uintptr_t AFortPawn_bIsInvulnerable = 0x72a;
    constexpr uintptr_t AFortPawn_CurrentWeapon = 0x990;
    constexpr uintptr_t AFortWeapon_WeaponData = 0x688;
    constexpr uintptr_t AFortWeapon_AmmoCount = 0x1194;
    constexpr uintptr_t AFortWeapon_bIsReloadingWeapon = 0x3b9;
    constexpr uintptr_t AFortWeapon_WeaponCoreAnimation = 0x16d8;
    constexpr uintptr_t AFortPickup_DefaultFlyTime = 0x654;
    constexpr uintptr_t AFortPickup_PrimaryPickupItemEntry = 0x3a8;
    constexpr uintptr_t FItemEntry_ItemDefinition = 0x10;
    constexpr uintptr_t UFortItemDefinition_ItemType = 0xa0;
    constexpr uintptr_t UBuildingContainer_bAlreadySearched = 0xd0a;
    constexpr uintptr_t UFortPlayerStateZone_Spectators = 0xab0;
    constexpr uintptr_t FFortSpectatorZoneArray_SpectatorArray = 0x108;
    constexpr uintptr_t FFortSpectatorZoneItem_PlayerState = 0x10;

    namespace BitFields
    {
        constexpr int bIsABot = 3;
        constexpr int bIsAnAthenaGameParticipant = 0;
        constexpr int bIsDying = 5;
        constexpr int bIsDBNO = 7;
        constexpr int bIsInvulnerable = 2;
        constexpr int bIsReloadingWeapon = 0;
        constexpr int bAlreadySearched = 3;
    }
}
