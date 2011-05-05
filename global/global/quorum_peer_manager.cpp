/*
 * Copyright (C) Lichuang
 */
#include <google/protobuf/text_format.h>
#include <eventrpc/log.h>
#include <eventrpc/thread.h>
#include <eventrpc/file_utility.h>
#include <eventrpc/rpc_server.h>
#include "quorum_peer_manager.h"
#include "protocol/leader_election.pb.h"
namespace global {
struct ListenerThreadRunnable : public eventrpc::Runnable {
  ListenerThreadRunnable(QuorumPeerManager::Impl *impl)
    : impl_(impl) {
  }
  void Run();
  QuorumPeerManager::Impl *impl_;
};

class LeaderElectionServiceImpl : public global::LeaderElection {
 public:
  LeaderElectionServiceImpl(QuorumPeerManager::Impl *impl)
    : impl_(impl) {
  }

  virtual ~LeaderElectionServiceImpl() {
  };

  virtual void LeaderProposal(
      ::google::protobuf::RpcController* controller,
      const ::global::Notification* request,
      ::global::Dummy* response,
      ::google::protobuf::Closure* done);
 private:
  QuorumPeerManager::Impl *impl_;
};

struct QuorumPeerManager::Impl {
 public:
  Impl();
  ~Impl();

  uint64 server_id() const;

  bool ParseConfigFile(const string &config_file);

  QuorumPeer* FindQuorumPeerById(uint64 server_id);

  void ListenerMainLoop();
 private:
  friend struct ListenerThreadRunnable;
  map<uint64, QuorumPeer*> quorum_peer_map_;
  global::ServerConfig server_config_;
  ListenerThreadRunnable listener_thread_runnable_;
  eventrpc::Thread listener_thread_;
};

QuorumPeerManager::Impl::Impl()
  : listener_thread_runnable_(this),
    listener_thread_(&listener_thread_runnable_) {
}

QuorumPeerManager::Impl::~Impl() {
  map<uint64, QuorumPeer*>::iterator iter;
  for (iter = quorum_peer_map_.begin();
       iter != quorum_peer_map_.end(); ) {
      QuorumPeer *peer = iter->second;
      ++iter;
      delete peer;
  }
  quorum_peer_map_.clear();
}

QuorumPeer* QuorumPeerManager::Impl::FindQuorumPeerById(uint64 server_id) {
  map<uint64, QuorumPeer*>::iterator iter;
  iter = quorum_peer_map_.find(server_id);
  if (iter == quorum_peer_map_.end()) {
    return NULL;
  }
  return iter->second;
}

uint64 QuorumPeerManager::Impl::server_id() const {
  return server_config_.server_id();
}

bool QuorumPeerManager::Impl::ParseConfigFile(const string &config_file) {
  string content;
  if (!eventrpc::FileUtility::ReadFileContents(config_file,
                                               &content)) {
    LOG_ERROR() << "cannot config file " << config_file;
    return false;
  }
  if (google::protobuf::TextFormat::ParseFromString(
      content, &server_config_)) {
    LOG_ERROR() << "parse from file " << config_file << " error";
    return false;
  }
  uint64 server_id = 0;
  for (int i = 0; i < server_config_.server_info_size(); ++i) {
    const global::ServerInfo &server_info = server_config_.server_info(i);
    server_id = server_info.server_id();
    QuorumPeer *peer = new QuorumPeer(
        server_id,
        server_info.server_host(),
        server_info.leader_port(),
        server_info.election_port());
    ASSERT(peer);
    quorum_peer_map_[server_id] = peer;
  }
}

void QuorumPeerManager::Impl::ListenerMainLoop() {
  eventrpc::RpcServer rpc_server;
  eventrpc::gpb::Service *service = new LeaderElectionServiceImpl(this);
  QuorumPeer *peer = quorum_peer_map_[server_config_.server_id()];
  rpc_server.rpc_method_manager()->RegisterService(service);
  rpc_server.set_host_and_port(peer->server_host_,
                               peer->election_port_);
  rpc_server.Start();
}

void ListenerThreadRunnable::Run() {
  impl_->ListenerMainLoop();
}

void LeaderElectionServiceImpl::LeaderProposal(
    ::google::protobuf::RpcController* controller,
    const ::global::Notification* request,
    ::global::Dummy* response,
    ::google::protobuf::Closure* done) {
}

QuorumPeerManager::QuorumPeerManager()
  : impl_(new Impl) {
}

QuorumPeerManager::~QuorumPeerManager() {
  delete impl_;
}


QuorumPeer* QuorumPeerManager::FindQuorumPeerById(uint64 server_id) {
  return impl_->FindQuorumPeerById(server_id);
}

uint64 QuorumPeerManager::server_id() const {
  return impl_->server_id();
}

bool QuorumPeerManager::ParseConfigFile(const string &config_file) {
  impl_->ParseConfigFile(config_file);
}
};
