/*
 * Copyright (C) Lichuang
 */
#ifndef __GLOBAL_QUORUM_PEER_SERVER_H__
#define __GLOBAL_QUORUM_PEER_SERVER_H__
#include <eventrpc/thread.h>
namespace global {
class QuorumPeerManager;
class QuorumPeerServer {
 public:
  QuorumPeerServer();
  ~QuorumPeerServer();

  void set_quorumpeer_manager(QuorumPeerManager *manager);

  void Start();

  struct Impl;
 private:
  Impl *impl_;
};
};
#endif  // __GLOBAL_QUORUM_PEER_SERVER_H__
