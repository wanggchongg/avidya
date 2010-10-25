
#include <map>
#include <eventrpc/net/dispatcher.h>
#include <eventrpc/net/rpcclientevent.h>
#include <eventrpc/net/rpcserverevent.h>
#include "server/fast_leader_electioner.h"
#include "proto/fast_leader_election.pb.h"
#include "log/log.h"

using std::map;
using std::pair;

GLOBAL_NAMESPACE_BEGIN

class FastLeaderElectioner::Impl
  : public fast_leader_election::FastLeaderElectionService {
 public:
  Impl(uint32 server_id,
       const list<server_config::ServerInfo> &server_info_list);

  virtual ~Impl();

  void Start();

  void Stop();

  virtual void Election(
      ::google::protobuf::RpcController* controller,
      const ::fast_leader_election::Request* request,
      ::fast_leader_election::Response* response,
      ::google::protobuf::Closure* done);

 private:
  void Init();
  void SendProposal();
  void SendProposalToServer(uint32 server_id,
                            const string &host,
                            uint32 election_port);

 private:
  uint32 server_id_;
  string host_;
  uint32 election_port_;
  uint32 internal_port_;
  list<server_config::ServerInfo> server_info_list_;
  eventrpc::Dispatcher dispatcher_;
  eventrpc::RpcServerEvent *server_event_;
  fast_leader_election::ServerState state_;
  uint64 leader_id_;
  uint64 global_id_;
  uint64 epoch_;

  typedef pair<eventrpc::RpcClientEvent*,
              fast_leader_election::FastLeaderElectionService::Stub*>
                EventStubPair;
  map<uint32, EventStubPair> map_serverid_to_stub_;
};

FastLeaderElectioner::Impl::Impl(
    uint32 server_id,
    const list<server_config::ServerInfo> &server_info_list)
  : server_id_(server_id),
    server_info_list_(server_info_list),
    state_(fast_leader_election::LOOKING),
    leader_id_(server_id),
    global_id_(0),
    epoch_(0) {
}

FastLeaderElectioner::Impl::~Impl() {
}

void FastLeaderElectioner::Impl::Start() {
  Init();
  SendProposal();
}

void FastLeaderElectioner::Impl::Stop() {
}

void FastLeaderElectioner::Impl::Init() {
  list<server_config::ServerInfo>::iterator iter;
  for (iter = server_info_list_.begin(); iter != server_info_list_.end(); ++iter) {
    if (iter->server_id() == server_id_) {
      host_ = iter->host();
      election_port_ = iter->election_port();
      internal_port_ = iter->internal_port();
      break;
    } else {
      eventrpc::RpcClientEvent *event = new eventrpc::RpcClientEvent(
          iter->host().c_str(), iter->election_port());

      fast_leader_election::FastLeaderElectionService::Stub *stub =
        new fast_leader_election::FastLeaderElectionService::Stub(event);

      map_serverid_to_stub_[iter->server_id()] = std::make_pair(
          event, stub);
      dispatcher_.AddEvent(event);
    }
  }

  ASSERT_NE(iter, server_info_list_.end());
  LOG_INFO() << "server host: " << host_
    << ", server id: " << server_id_
    << ", election port: " << election_port_
    << ", internal port: " << internal_port_;
  server_event_ = new eventrpc::RpcServerEvent(host_.c_str(), (int32)election_port_);
  server_event_->RegisterService(this);
  dispatcher_.AddEvent(server_event_);
  dispatcher_.Loop();
}

void FastLeaderElectioner::Impl::SendProposal() {
  map<uint32, EventStubPair>::iterator iter;
  for (iter = map_serverid_to_stub_.begin();
       iter != map_serverid_to_stub_.end();
       ++iter) {
    eventrpc::RpcClientEvent *event = iter->second.first;
    fast_leader_election::FastLeaderElectionService::Stub *stub =
      iter->second.second;

    fast_leader_election::Request request;
    request.set_leader(server_id_);
    request.set_global_id(global_id_);
    request.set_epoch(epoch_);
    request.set_server_id(server_id_);
    request.set_state(state_);
    stub->Election(NULL, &request, NULL, NULL);
  }
}

void FastLeaderElectioner::Impl::Election(
    ::google::protobuf::RpcController* controller,
    const ::fast_leader_election::Request* request,
    ::fast_leader_election::Response* response,
    ::google::protobuf::Closure* done) {
}

void FastLeaderElectioner::Impl::SendProposalToServer(
    uint32 server_id, const string &host, uint32 election_port) {
}

FastLeaderElectioner::FastLeaderElectioner(
    uint32 server_id,
    const list<server_config::ServerInfo> &server_info_list) {
  impl_ = new Impl(server_id, server_info_list);
}

FastLeaderElectioner::~FastLeaderElectioner() {
  delete impl_;
}

void FastLeaderElectioner::Start() {
  impl_->Start();
}

void FastLeaderElectioner::Stop() {
  impl_->Stop();
}

GLOBAL_NAMESPACE_END
