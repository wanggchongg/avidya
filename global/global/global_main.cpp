/*
 * Copyright (C) Lichuang
 */
#include <signal.h>
#include <stdio.h>
#include <eventrpc/log.h>
#include "quorum_peer_manager.h"
#include "quorum_peer_server.h"
using namespace global;
void Usage(char *argv[]) {
  LOG_ERROR() << "usage: " << argv[0] << "-f configfile";
}

int main(int argc, char *argv[]) {
  eventrpc::SetProgramName(argv[0]);
  if (argc != 3) {
    Usage(argv);
    exit(-1);
  }
  QuorumPeerManager peer_manager;
  if (!peer_manager.ParseConfigFile(argv[2])) {
    LOG_ERROR() << "parse config file " << argv[2] << " error";
    exit(-1);
  }

  QuorumPeerServer server;
  server.set_quorumpeer_manager(&peer_manager);
  server.Start();

  sigset_t new_mask;
  sigfillset(&new_mask);
  sigset_t old_mask;
  sigset_t wait_mask;
  sigemptyset(&wait_mask);
  sigaddset(&wait_mask, SIGINT);
  sigaddset(&wait_mask, SIGQUIT);
  sigaddset(&wait_mask, SIGTERM);
  pthread_sigmask(SIG_BLOCK, &new_mask, &old_mask);
  pthread_sigmask(SIG_SETMASK, &old_mask, 0);
  pthread_sigmask(SIG_BLOCK, &wait_mask, 0);
  int sig = 0;
  sigwait(&wait_mask, &sig);
  return 0;
}
