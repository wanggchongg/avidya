/*
 * Copyright (C) Lichuang
 */
#include <eventrpc/file_utility.h>
#include <eventrpc/log.h>
#include <google/protobuf/text_format.h>
#include "quorum_peer_server.h"
#include "protocol/server_config.pb.h"
namespace global {
QuorumPeerServer::QuorumPeerServer()
  : server_id_(0) {
}

QuorumPeerServer::~QuorumPeerServer() {
}

bool QuorumPeerServer::ParseConfigFile(const string &config_file) {
  string content;
  global::ServerConfig server_config;
  if (!eventrpc::FileUtility::ReadFileContents(config_file,
                                               &content)) {
    LOG_ERROR() << "cannot config file " << config_file;
    return false;
  }
  if (google::protobuf::TextFormat::ParseFromString(
      content, &server_config)) {
    LOG_ERROR() << "parse from file " << config_file << " error";
    return false;
  }
  server_id_ = server_config.server_id();
  uint64 server_id = 0;
  for (int i = 0; i < server_config.server_info_size(); ++i) {
    const global::ServerInfo &server_info = server_config.server_info(i);
    server_id = server_info.server_id();
    QuorumPeer *peer = new QuorumPeer(
        server_id,
        server_info.server_address(),
        server_info.leader_port(),
        server_info.election_port());
    ASSERT(peer);
    quorum_peer_manager_.set_quorum_peer_by_id(server_id, peer);
  }
}
};
