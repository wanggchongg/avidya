/*
 * Copyright (C) Lichuang
 */
#include <eventrpc/base.h>
#include <eventrpc/log.h>
#include "fast_leader_election.h"
#include "quorum_peer.h"
#include "quorum_peer_server.h"
#include "quorum_peer_manager.h"
namespace global {
class QuorumPeerServer::Impl {
 public:
  Impl(QuorumPeerServer *server);
  ~Impl();

  void set_quorumpeer_manager(QuorumPeerManager *manager);

  void Start();

 private:
  QuorumPeerServer *quorum_peer_server_;
  uint64 server_id_;
  QuorumPeer *quorum_peer_;
  QuorumPeerManager *quorum_peer_manager_;
  FastLeaderElection fast_leader_election_;
};

QuorumPeerServer::Impl::Impl(QuorumPeerServer *server)
  : quorum_peer_server_(server),
    server_id_(0),
    quorum_peer_(NULL),
    quorum_peer_manager_(NULL),
    fast_leader_election_(server) {
}

QuorumPeerServer::Impl::~Impl() {
}

void QuorumPeerServer::Impl::set_quorumpeer_manager(QuorumPeerManager *manager) {
  quorum_peer_manager_ = manager;
  server_id_ = manager->server_id();
  quorum_peer_ = manager->FindQuorumPeerById(server_id_);
}

void QuorumPeerServer::Impl::Start() {
}

QuorumPeerServer::QuorumPeerServer()
  : impl_(new Impl(this)) {
}

QuorumPeerServer::~QuorumPeerServer() {
  delete impl_;
}

void QuorumPeerServer::set_quorumpeer_manager(QuorumPeerManager *manager) {
  impl_->set_quorumpeer_manager(manager);
}

void QuorumPeerServer::Start() {
  impl_->Start();
}
};
