/*
 * Copyright (C) Lichuang
 */
#ifndef __GLOBAL_QUORUM_PEER_H__
#define __GLOBAL_QUORUM_PEER_H__
#include <string>
#include <eventrpc/base.h>
using std::string;
namespace global {
enum QuorumPeerState {
  LOOKING       = 1,
  FOLLOWING     = 2,
  LEADING       = 3,
  OBSERVING     = 4
};

struct QuorumPeer {
 public:
  QuorumPeer(uint64 server_id, const string &server_host,
             uint32 leader_port, uint32 election_port)
    : state_(LOOKING),
      server_id_(server_id),
      server_host_(server_host),
      leader_port_(leader_port),
      election_port_(election_port) {
  }

  ~QuorumPeer() {
  }

 public:
  QuorumPeerState state_;
  uint64 server_id_;
  string server_host_;
  uint32 leader_port_;
  uint32 election_port_;
};
};
#endif  //  __GLOBAL_QUORUM_PEER_H__
