/*
 * Copyright (C) Lichuang
 *
 */
#include <eventrpc/log.h>
#include "quorum_peer_server.h"
#include "fast_leader_election.h"

namespace global {
FastLeaderElection::FastLeaderElection(QuorumPeerServer *server)
  : logical_lock_(0),
    quorum_peer_server_(server) {
}

FastLeaderElection::~FastLeaderElection() {
}

void FastLeaderElection::LookForLeader() {
}
};
