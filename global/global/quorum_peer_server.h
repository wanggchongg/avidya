/*
 * Copyright (C) Lichuang
 */
#ifndef __GLOBAL_QUORUM_PEER_SERVER_H__
#define __GLOBAL_QUORUM_PEER_SERVER_H__
#include "quorum_peer_manager.h"
namespace global {
class QuorumPeerServer {
 public:
  QuorumPeerServer();
  ~QuorumPeerServer();

  bool ParseConfigFile(const string &config_file);

 private:
  uint64 server_id_;
  QuorumPeerManager quorum_peer_manager_;
};
};
#endif  // __GLOBAL_QUORUM_PEER_SERVER_H__
