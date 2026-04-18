#include <tier0/platform.h>
#undef RESTRICT
#define RESTRICT

#include <tier1/convar.h>
#include <tier1/utlstring.h>
#include <entity2/entityidentity.h>
#include <entityhandle.h>
#include <igamesystem.h>

IGameSystem * gamesystem();

/*
clang++ -std=c++20 -c cpp_tests/igamesystem.cpp ` --target=x86_64-pc-windows-msvc
  -Ihl2sdk_cs2/game/shared -Ihl2sdk_cs2/public `
  -Ihl2sdk_cs2/public/tier0 -Ihl2sdk_cs2/public/tier1 `
  -DCOMPILER_MSVC=1 -DCOMPILER_MSVC64=1 `
  -Xclang -fdump-vtable-layouts -fms-extensions -fms-compatibility

VFTable indices for 'IGameSystem' (62 entries).
   0 | bool IGameSystem::Init()
   1 | void IGameSystem::PostInit()
   2 | void IGameSystem::Shutdown()
   3 | void IGameSystem::GameInit(const EventGameInit_t *const)
   4 | void IGameSystem::GameShutdown(const EventGameShutdown_t *const)
   5 | void IGameSystem::GamePostInit(const EventGamePostInit_t *const)
   6 | void IGameSystem::GamePreShutdown(const EventGamePreShutdown_t *const)
   7 | void IGameSystem::BuildGameSessionManifest(const EventBuildGameSessionManifest_t *const)
   8 | void IGameSystem::GameActivate(const EventGameActivate_t *const)
   9 | void IGameSystem::ClientFullySignedOn(const EventClientFullySignedOn_t *const)
  10 | void IGameSystem::Disconnect(const EventDisconnect_t *const)
  11 | void IGameSystem::unk_001(const void *const)
  12 | void IGameSystem::GameDeactivate(const EventGameDeactivate_t *const)
  13 | void IGameSystem::SpawnGroupPrecache(const EventSpawnGroupPrecache_t *const)
  14 | void IGameSystem::SpawnGroupUncache(const EventSpawnGroupUncache_t *const)
  15 | void IGameSystem::PreSpawnGroupLoad(const EventPreSpawnGroupLoad_t *const)
  16 | void IGameSystem::PostSpawnGroupLoad(const EventPostSpawnGroupLoad_t *const)
  17 | void IGameSystem::PreSpawnGroupUnload(const EventPreSpawnGroupUnload_t *const)
  18 | void IGameSystem::PostSpawnGroupUnload(const EventPostSpawnGroupUnload_t *const)
  19 | void IGameSystem::ActiveSpawnGroupChanged(const EventActiveSpawnGroupChanged_t *const)
  20 | void IGameSystem::ClientPostDataUpdate(const EventClientPostDataUpdate_t *const)
  21 | void IGameSystem::ClientPreRender(const EventClientPreRender_t *const)
  22 | void IGameSystem::ClientPreEntityThink(const EventClientPreEntityThink_t *const)
  23 | void IGameSystem::unk_101(const void *const)
  24 | void IGameSystem::unk_102(const void *const)
  25 | void IGameSystem::unk_103(const void *const)
  26 | void IGameSystem::ClientPollNetworking(const EventClientPollNetworking_t *const)
  27 | void IGameSystem::unk_201(const void *const)
  28 | void IGameSystem::ClientUpdate(const EventClientUpdate_t *const)
  29 | void IGameSystem::unk_301(const void *const)
  30 | void IGameSystem::ClientPostRender(const EventClientPostRender_t *const)
  31 | void IGameSystem::ServerPreEntityThink(const EventServerPreEntityThink_t *const)
  32 | void IGameSystem::ServerPostEntityThink(const EventServerPostEntityThink_t *const)
  33 | void IGameSystem::unk_401(const void *const)
  34 | void IGameSystem::ServerPreClientUpdate(const EventServerPreClientUpdate_t *const)
  35 | void IGameSystem::ServerAdvanceTick(const EventServerAdvanceTick_t *const)
  36 | void IGameSystem::ClientAdvanceTick(const EventClientAdvanceTick_t *const)
  37 | void IGameSystem::ServerGamePostSimulate(const EventServerGamePostSimulate_t *const)
  38 | void IGameSystem::ClientGamePostSimulate(const EventClientGamePostSimulate_t *const)
  39 | void IGameSystem::ServerPostAdvanceTick(const EventServerPostAdvanceTick_t *const)
  40 | void IGameSystem::ClientPostAdvanceTick(const EventClientPostAdvanceTick_t *const)
  41 | void IGameSystem::ServerBeginAsyncPostTickWork(const EventServerBeginAsyncPostTickWork_t *const)
  42 | void IGameSystem::unk_501(const void *const)
  43 | void IGameSystem::ServerEndAsyncPostTickWork(const EventServerEndAsyncPostTickWork_t *const)
  44 | void IGameSystem::ClientFrameSimulate(const EventClientFrameSimulate_t *const)
  45 | void IGameSystem::ClientPauseSimulate(const EventClientPauseSimulate_t *const)
  46 | void IGameSystem::ClientAdvanceNonRenderedFrame(const EventClientAdvanceNonRenderedFrame_t *const)
  47 | void IGameSystem::GameFrameBoundary(const EventGameFrameBoundary_t *const)
  48 | void IGameSystem::OutOfGameFrameBoundary(const EventOutOfGameFrameBoundary_t *const)
  49 | void IGameSystem::SaveGame(const EventSaveGame_t *const)
  50 | void IGameSystem::RestoreGame(const EventRestoreGame_t *const)
  51 | void IGameSystem::unk_601(const void *const)
  52 | void IGameSystem::unk_602(const void *const)
  53 | void IGameSystem::unk_603(const void *const)
  54 | void IGameSystem::unk_604(const void *const)
  55 | void IGameSystem::unk_605(const void *const)
  56 | void IGameSystem::unk_606(const void *const)
  57 | const char *IGameSystem::GetName() const
  58 | void IGameSystem::SetGameSystemGlobalPtrs(void *)
  59 | void IGameSystem::SetName(const char *)
  60 | bool IGameSystem::DoesGameSystemReallocate()
  61 | IGameSystem::~IGameSystem() [scalar deleting]
*/

int main() {

    gamesystem()->Init();

    return 0;
}
