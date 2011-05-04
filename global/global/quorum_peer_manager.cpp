/*
 * Copyright (C) Lichuang
 */
#include "quorum_peer_manager.h"
namespace global {
QuorumPeerManager::QuorumPeerManager() {
}

QuorumPeerManager::~QuorumPeerManager() {
}

QuorumPeer* QuorumPeerManager::get_quorum_peer_by_id(uint64 server_id) {
  return quorum_peer_map_[server_id];
}

void QuorumPeerManager::set_quorum_peer_by_id(
    uint64 server_id,
    QuorumPeer *quorum_peer) {
  quorum_peer_map_[server_id] = quorum_peer;
}
};
