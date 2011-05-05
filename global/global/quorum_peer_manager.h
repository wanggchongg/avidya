/*
 * Copyright (C) Lichuang
 */
#ifndef __GLOBAL_QUORUM_PEER_MANAGER_H__
#define __GLOBAL_QUORUM_PEER_MANAGER_H__
#include <eventrpc/base.h>
#include <map>
#include "protocol/server_config.pb.h"
#include "quorum_peer.h"
using std::map;
namespace global {
class QuorumPeer;
class QuorumPeerManager {
 public:
  QuorumPeerManager();
  ~QuorumPeerManager();

  uint64 server_id() const;
  bool ParseConfigFile(const string &config_file);
  QuorumPeer* FindQuorumPeerById(uint64 server_id);

 private:
  map<uint64, QuorumPeer*> quorum_peer_map_;
  global::ServerConfig server_config_;
};
};
#endif // __GLOBAL_QUORUM_PEER_MANAGER_H__
