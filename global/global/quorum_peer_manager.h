/*
 * Copyright (C) Lichuang
 */
#ifndef __GLOBAL_QUORUM_PEER_MANAGER_H__
#define __GLOBAL_QUORUM_PEER_MANAGER_H__
#include <eventrpc/base.h>
#include <map>
#include "quorum_peer.h"
using std::map;
namespace global {
class QuorumPeer;
class QuorumPeerManager {
 public:
  QuorumPeerManager();
  ~QuorumPeerManager();

  QuorumPeer* get_quorum_peer_by_id(uint64 server_id);
  void set_quorum_peer_by_id(uint64 server_id,
                             QuorumPeer *quorum_peer);

 private:
  map<uint64, QuorumPeer*> quorum_peer_map_;
};
};
#endif // __GLOBAL_QUORUM_PEER_MANAGER_H__
