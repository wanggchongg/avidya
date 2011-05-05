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
// for quorum peer server
struct QuorumPeerServerRunnable : public eventrpc::Runnable {
  QuorumPeerServerRunnable(QuorumPeerServer::Impl *peer_server)
    : quorum_peer_server_(peer_server) {
    }

  virtual ~QuorumPeerServerRunnable() {
  }

  void Run();

  QuorumPeerServer::Impl *quorum_peer_server_;
};

class QuorumPeerServer::Impl {
 public:
  Impl(QuorumPeerServer *server);
  ~Impl();

  void set_quorumpeer_manager(QuorumPeerManager *manager);

  void Start();

 private:

  void MainLoop();

 private:
  friend struct QuorumPeerServerRunnable;
  QuorumPeerServer *quorum_peer_server_;
  uint64 server_id_;
  QuorumPeer *quorum_peer_;
  QuorumPeerManager *quorum_peer_manager_;
  QuorumPeerServerRunnable runnable_;
  eventrpc::Thread thread_;
  FastLeaderElection fast_leader_election_;
};

QuorumPeerServer::Impl::Impl(QuorumPeerServer *server)
  : quorum_peer_server_(server),
    server_id_(0),
    quorum_peer_(NULL),
    quorum_peer_manager_(NULL),
    runnable_(this),
    thread_(&runnable_),
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
  thread_.Start();
}

void QuorumPeerServer::Impl::MainLoop() {
  while (true) {
    switch (quorum_peer_->state_) {
    }
  }
}

void QuorumPeerServerRunnable::Run() {
  quorum_peer_server_->MainLoop();
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
