/*
 * Copyright (C) Lichuang
 */
#include <stdio.h>
#include "quorum_peer_manager.h"
using namespace global;
int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("usage: %s -f configfile\n", argv[0]);
    return -1;
  }
  QuorumPeerManager peer_manager;
  if (peer_manager.ParseConfigFile(argv[1])) {
    printf("parse config file %s error\n", argv[1]);
    return -1;
  }

  return 0;
}
