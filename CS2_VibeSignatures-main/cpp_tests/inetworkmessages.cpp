#include <tier0/platform.h>
#undef RESTRICT
#define RESTRICT

// Pre-define include guards to avoid heavy transitive includes
// eiface.h -> edict.h -> cmodel.h -> gametrace.h -> variant.h -> vector.h (missing)
// inetchannel.h -> eiface.h (same chain) and needs protobuf-generated ENetworkDisconnectionReason
#define EIFACE_H
#define INETCHANNEL_H
enum NetChannelBufType_t : int8 {};

#include <networksystem/inetworkmessages.h>

INetworkMessages * networkmessages();

int main() {

    networkmessages()->GetLoggingChannel();

    return 0;
}
