/*
 * Copyright (C) Lichuang
 */
#ifndef __GLOBAL_QUORUM_PEER_SERVER_H__
#define __GLOBAL_QUORUM_PEER_SERVER_H__
#include <eventrpc/dispatcher.h>
namespace global {
class QuorumPeerManager;
class DataTree;
class QuorumPeerServer {
 public:
  QuorumPeerServer();
  ~QuorumPeerServer();

  void set_quorumpeer_manager(QuorumPeerManager *manager);
  void set_data_tree(DataTree *data_tree);
  void set_dispatcher(eventrpc::Dispatcher *dispatcher);

  void Start();

  struct Impl;
 private:
  Impl *impl_;
};
};
#endif  // __GLOBAL_QUORUM_PEER_SERVER_H__
