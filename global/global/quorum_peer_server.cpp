/*
 * Copyright (C) Lichuang
 */
#include <list>
#include <eventrpc/base.h>
#include <eventrpc/log.h>
#include <eventrpc/rpc_server.h>
#include "fast_leader_election.h"
#include "data_tree.h"
#include "quorum_peer.h"
#include "quorum_peer_server.h"
#include "quorum_peer_manager.h"
#include "protocol/leader_election.pb.h"
using namespace eventrpc;
using std::list;
namespace global {
class LeaderElectionServiceImpl : public LeaderElection {
 public:
  virtual void LeaderProposal(::google::protobuf::RpcController* controller,
                       const ::global::Notification* request,
                       ::global::Dummy* response,
                       ::google::protobuf::Closure* done);
};

class QuorumPeerServer::Impl {
 public:
  Impl(QuorumPeerServer *server);
  ~Impl();

  void set_quorumpeer_manager(QuorumPeerManager *manager);

  void set_data_tree(DataTree *data_tree);

  void set_dispatcher(Dispatcher *dispatcher);

  void Start();

  void SendProposal();

 private:
  QuorumPeerServer *quorum_peer_server_;
  uint64 my_server_id_;
  QuorumPeer *quorum_peer_;
  list<QuorumPeer*> other_quorum_peers_;
  QuorumPeerManager *quorum_peer_manager_;
  DataTree *data_tree_;
  FastLeaderElection fast_leader_election_;
  RpcServer rpc_server_;
  Dispatcher *dispatcher_;
  uint64 logical_clock_;
  uint64 proposed_leader_;
  uint64 proposed_zxid_;
};

QuorumPeerServer::Impl::Impl(QuorumPeerServer *server)
  : quorum_peer_server_(server),
    my_server_id_(0),
    quorum_peer_(NULL),
    quorum_peer_manager_(NULL),
    data_tree_(NULL),
    fast_leader_election_(server),
    dispatcher_(NULL),
    logical_clock_(0),
    proposed_leader_(0),
    proposed_zxid_(0) {
}

QuorumPeerServer::Impl::~Impl() {
}

void QuorumPeerServer::Impl::set_quorumpeer_manager(QuorumPeerManager *manager) {
  quorum_peer_manager_ = manager;
  my_server_id_ = manager->my_server_id();
  quorum_peer_ = manager->FindQuorumPeerById(my_server_id_);
  manager->GetOtherQuorumPeers(&other_quorum_peers_);
}

void QuorumPeerServer::Impl::set_data_tree(DataTree *data_tree) {
  data_tree_ = data_tree;
}

void QuorumPeerServer::Impl::set_dispatcher(Dispatcher *dispatcher) {
  dispatcher_ = dispatcher;
}

void QuorumPeerServer::Impl::Start() {
  proposed_leader_ = my_server_id_;
  rpc_server_.set_dispatcher(dispatcher_);
  gpb::Service *service = new LeaderElectionServiceImpl();
  rpc_server_.rpc_method_manager()->RegisterService(service);
  rpc_server_.set_host_and_port(quorum_peer_->server_host_,
                                quorum_peer_->election_port_);
  rpc_server_.Start();
}

void QuorumPeerServer::Impl::SendProposal() {
  list<QuorumPeer*>::iterator iter;
  for (iter = other_quorum_peers_.begin();
       iter != other_quorum_peers_.end(); ++iter) {
  }
}

void LeaderElectionServiceImpl::LeaderProposal(
    ::google::protobuf::RpcController* controller,
    const ::global::Notification* request,
    ::global::Dummy* response,
    ::google::protobuf::Closure* done) {
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

void QuorumPeerServer::set_data_tree(DataTree *data_tree) {
  impl_->set_data_tree(data_tree);
}

void QuorumPeerServer::set_dispatcher(Dispatcher *dispatcher) {
  impl_->set_dispatcher(dispatcher);
}

void QuorumPeerServer::Start() {
  impl_->Start();
}
};
