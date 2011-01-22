
#include "reactor_rpc_server.h"
#include "log.h"
#include "net_utility.h"
#include "rpc_connection.h"

namespace eventrpc {
ReactorRpcServer::ReactorRpcServer()
  : RpcServer() {
}

ReactorRpcServer::~ReactorRpcServer() {
  dispatcher_.DeleteEvent(event_);
  delete event_;
}

void ReactorRpcServer::Run() {
  listen_fd_ = NetUtility::Listen(host_.c_str(), port_);
  ASSERT_TRUE(listen_fd_ > 0);
  event_ = new RpcServer::RpcServerEvent(listen_fd_,
                                         EVENT_READ, this);
  dispatcher_.AddEvent(event_);
  ASSERT_NE(static_cast<RpcServerEvent*>(NULL), event_);
  while (dispatcher_.Poll() == 0) {
    ;
  }
}

int ReactorRpcServer::HandleAccept() {
  int fd;
  for (int i = 0; i < 200; ++i) {
    fd = NetUtility::Accept(listen_fd_);
    if (fd == -1) {
      break;
    }
    RpcConnection* connection = rpc_connection_manager_.GetConnection();
    connection->set_fd(fd);
    connection->set_rpc_method_manager(&rpc_method_manager_);
    connection->set_rpc_connection_manager(&rpc_connection_manager_);
    dispatcher_.AddEvent(connection->event());
  }
}

}; // namespace eventrpc
