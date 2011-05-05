/*
 * Copyright (C) Lichuang
 *
 */
#ifndef __GLOBAL_FAST_ELECTION_H__
#define __GLOBAL_FAST_ELECTION_H__
#include <eventrpc/base.h>
#include <string>
using namespace std;
namespace global {
class QuorumPeerServer;
class FastLeaderElection {
 public:
  explicit FastLeaderElection(QuorumPeerServer *server);
  ~FastLeaderElection();

 private:
  QuorumPeerServer *quorum_peer_server_;
};
};
#endif  // __GLOBAL_FAST_ELECTION_H__
