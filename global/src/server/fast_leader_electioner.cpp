
#include <eventrpc/net/dispatcher.h>
#include <eventrpc/net/rpcclientevent.h>
#include <eventrpc/net/rpcserverevent.h>
#include "server/fast_leader_electioner.h"
#include "proto/fast_leader_election.pb.h"
#include "log/log.h"

GLOBAL_NAMESPACE_BEGIN

class FastLeaderElectioner::Impl
  : public fast_leader_election::FastLeaderElectionService {
 public:
  Impl(uint32 server_id,
       const list<server_config::ServerInfo> &addr_list);

  virtual ~Impl();

  void Start();

  void Stop();

  virtual void Election(
      ::google::protobuf::RpcController* controller,
      const ::fast_leader_election::Request* request,
      ::fast_leader_election::Response* response,
      ::google::protobuf::Closure* done) {
  }

 private:
  void Init();

 private:
  uint32 server_id_;
  string host_;
  uint32 election_port_;
  uint32 internal_port_;
  list<server_config::ServerInfo> addr_list_;
  eventrpc::Dispatcher dispatcher_;
};

FastLeaderElectioner::Impl::Impl(
    uint32 server_id,
    const list<server_config::ServerInfo> &addr_list)
  : server_id_(server_id)
  , addr_list_(addr_list) {
}

FastLeaderElectioner::Impl::~Impl() {
}

void FastLeaderElectioner::Impl::Start() {
  Init();
}

void FastLeaderElectioner::Impl::Stop() {
}

void FastLeaderElectioner::Impl::Init() {
  list<server_config::ServerInfo>::iterator iter;
  for (iter = addr_list_.begin(); iter != addr_list_.end(); ++iter) {
    if (iter->server_id() == server_id_) {
      host_ = iter->host();
      election_port_ = iter->election_port();
      internal_port_ = iter->internal_port();
      break;
    }
  }

  ASSERT_NE(iter, addr_list_.end());
  LOG_INFO() << "server host: " << host_
    << ", server id: " << server_id_
    << ", election port: " << election_port_
    << ", internal port: " << internal_port_;
}

FastLeaderElectioner::FastLeaderElectioner(
    uint32 server_id,
    const list<server_config::ServerInfo> &addr_list) {
  impl_ = new Impl(server_id, addr_list);
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
