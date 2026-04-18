#include <tier0/platform.h>
#undef RESTRICT
#define RESTRICT

// Pre-define include guards to avoid heavy transitive includes (eiface.h -> protobuf)
#define INETCHANNEL_H
#define BITBUF_H
class INetChannel;
enum ENetworkDisconnectionReason {};
struct netadr_t { int type; unsigned short port; unsigned int ip; };
class CSteamID {};

#include <networksystem/inetworksystem.h>

INetworkSystem * networksystem();

/*
clang++ -std=c++20 -c cpp_tests/inetworksystem.cpp `
  --target=x86_64-pc-windows-msvc `
  -Ihl2sdk_cs2/game/shared -Ihl2sdk_cs2/public `
  -Ihl2sdk_cs2/public/tier0 -Ihl2sdk_cs2/public/tier1 `
  -DCOMPILER_MSVC=1 -DCOMPILER_MSVC64=1 `
  -Xclang -fdump-vtable-layouts -fms-extensions -fms-compatibility

VFTable indices for 'INetworkSystem' (55 entries).
   0 | bool IAppSystem::Connect(CreateInterfaceFn)
   1 | void IAppSystem::Disconnect()
   2 | void *IAppSystem::QueryInterface(const char *)
   3 | InitReturnVal_t IAppSystem::Init()
   4 | void IAppSystem::Shutdown()
   5 | void IAppSystem::PreShutdown()
   6 | const AppSystemInfo_t *IAppSystem::GetDependencies()
   7 | AppSystemTier_t IAppSystem::GetTier()
   8 | void IAppSystem::Reconnect(CreateInterfaceFn, const char *)
   9 | bool IAppSystem::IsSingleton()
  10 | BuildType_t IAppSystem::GetBuildType()
  11 | void INetworkSystem::InitGameServer()
  12 | void INetworkSystem::ShutdownGameServer()
  13 | int INetworkSystem::CreateSocket(int, int, int, int, int, const char *)
  14 | void INetworkSystem::OpenSocket(int)
  15 | void INetworkSystem::ConnectSocket(int, const netadr_t &)
  16 | bool INetworkSystem::IsSocketOpen(int)
  17 | void INetworkSystem::CloseSocket(int)
  18 | void INetworkSystem::EnableLoopbackBetweenSockets(int, int)
  19 | void INetworkSystem::SetDefaultBroadcastPort(int)
  20 | void INetworkSystem::PollSocket(int, IConnectionlessPacketHandler *)
  21 | void INetworkSystem::unk001()
  22 | INetChannel *INetworkSystem::CreateNetChannel(int, const ns_address *, uint32, const char *, uint32, uint32)
  23 | void INetworkSystem::RemoveNetChannel(INetChannel *, bool)
  24 | bool INetworkSystem::RemoveNetChannelByAddress(int, const ns_address *)
  25 | void INetworkSystem::PrintNetworkStats()
  26 | void INetworkSystem::unk101()
  27 | void INetworkSystem::unk102()
  28 | const char *INetworkSystem::DescribeSocket(int)
  29 | bool INetworkSystem::IsValidSocket(int)
  30 | void INetworkSystem::BufferToBufferCompress(uint8 *, int &, uint8 *, unsigned int)
  31 | void INetworkSystem::BufferToBufferDecompress(uint8 *, int &, uint8 *, unsigned int)
  32 | netadr_t &INetworkSystem::GetPublicAdr()
  33 | netadr_t &INetworkSystem::GetLocalAdr()
  34 | float INetworkSystem::GetFakeLag(int)
  35 | uint16 INetworkSystem::GetUDPPort(int)
  36 | void INetworkSystem::unk201()
  37 | void INetworkSystem::unk202()
  38 | void INetworkSystem::CloseAllSockets()
  39 | NetScratchBuffer_t *INetworkSystem::GetScratchBuffer()
  40 | void INetworkSystem::PutScratchBuffer(NetScratchBuffer_t *)
  41 | void *INetworkSystem::GetSteamNetworkUtils()
  42 | void *INetworkSystem::GetSteamUserNetworkingSockets()
  43 | void *INetworkSystem::GetSteamGameServerNetworkingSockets()
  44 | void *INetworkSystem::GetSteamNetworkingSockets()
  45 | void *INetworkSystem::GetSteamNetworkingMessages()
  46 | void INetworkSystem::unk301()
  47 | void INetworkSystem::unk302()
  48 | void INetworkSystem::RejectConnection(uint32, ENetworkDisconnectionReason, void *)
  49 | void INetworkSystem::unk401()
  50 | void INetworkSystem::unk402()
  51 | void INetworkSystem::InitNetworkSystem()
  52 | void INetworkSystem::unk501()
  53 | void INetworkSystem::unk502()
  54 | INetworkSystem::~INetworkSystem() [scalar deleting]
*/

int main() {

    networksystem()->InitGameServer();
    
    return 0;
}
