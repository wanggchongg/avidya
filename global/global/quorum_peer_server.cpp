/*
 * Copyright (C) Lichuang
 */
#include <eventrpc/base.h>
#include <eventrpc/log.h>
#include "quorum_peer_server.h"
#include "quorum_peer_manager.h"
namespace global {
QuorumPeerServer::QuorumPeerServer()
  : server_id_(0),
    quorum_peer_manager_(NULL),
    runnable_(this),
    thread_(&runnable_) {
}

QuorumPeerServer::~QuorumPeerServer() {
}

void QuorumPeerServer::set_quorumpeer_manager(QuorumPeerManager *manager) {
  quorum_peer_manager_ = manager;
  server_id_ = manager->server_id();
}

void QuorumPeerServer::Start() {
  thread_.Start();
}

void QuorumPeerServer::MainLoop() {
}

void QuorumPeerServer::QuorumPeerServerRunnable::Run() {
  quorum_peer_server_->MainLoop();
}
};
