
#include <eventrpc/net/dispatcher.h>
#include <eventrpc/net/rpcclientevent.h>
#include <eventrpc/net/rpcserverevent.h>
#include "common/fast_leader_electioner.h"
#include "proto/fast_leader_election.pb.h"

GLOBAL_NAMESPACE_BEGIN

FastLeaderElectioner::FastLeaderElectioner(
    string host,
    uint32 port,
    list<ServerAddrPair> addr_pair_list)
  : host_(host)
  , port_(port)
  , addr_pair_list_(addr_pair_list) {
}

void FastLeaderElectioner::Start() {
}

void FastLeaderElectioner::Stop() {
}

GLOBAL_NAMESPACE_END
