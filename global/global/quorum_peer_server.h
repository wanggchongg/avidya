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

 private:
  // for quorum peer server
  struct QuorumPeerServerRunnable : public eventrpc::Runnable {
    QuorumPeerServerRunnable(QuorumPeerServer *peer_server)
      : quorum_peer_server_(peer_server) {
    }

    virtual ~QuorumPeerServerRunnable() {
    }

    void Run();

    QuorumPeerServer *quorum_peer_server_;
  };

  void MainLoop();

 private:
  friend struct QuorumPeerServerRunnable;
  uint64 server_id_;
  QuorumPeerManager *quorum_peer_manager_;
  QuorumPeerServerRunnable runnable_;
  eventrpc::Thread thread_;
};
};
#endif  // __GLOBAL_QUORUM_PEER_SERVER_H__
