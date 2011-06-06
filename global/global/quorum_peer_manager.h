/*
 * Copyright (C) Lichuang
 */
#ifndef __GLOBAL_QUORUM_PEER_MANAGER_H__
#define __GLOBAL_QUORUM_PEER_MANAGER_H__
#include <eventrpc/base.h>
#include <eventrpc/dispatcher.h>
#include <map>
#include "protocol/server_config.pb.h"
#include "quorum_peer.h"
using std::map;
namespace global {
class QuorumPeer;
class QuorumPeerManager {
 public:
  explicit QuorumPeerManager();
  ~QuorumPeerManager();

  uint64 my_server_id() const;

  bool ParseConfigFile(const string &config_file);

  QuorumPeer* FindQuorumPeerById(uint64 server_id);

  void set_dispatcher(eventrpc::Dispatcher *dispatcher);

  struct Impl;
 private:
  Impl *impl_;
};
};
#endif // __GLOBAL_QUORUM_PEER_MANAGER_H__
