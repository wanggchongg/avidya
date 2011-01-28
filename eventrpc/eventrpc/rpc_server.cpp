
#include <signal.h>
#include "utility.h"
#include "rpc_server.h"
#include "event.h"
#include "log.h"
#include "net_utility.h"
#include "rpc_connection.h"

namespace eventrpc {
RpcServer::RpcServer()
  : host_(""),
    port_(0),
    listen_fd_(0) {
}

RpcServer::~RpcServer() {
  dispatcher_.DeleteEvent(event_);
  delete event_;
}

void RpcServer::Start() {
  dispatcher_.Start();
  listen_fd_ = NetUtility::Listen(host_.c_str(), port_);
  ASSERT_TRUE(listen_fd_ > 0);
  event_ = new RpcServer::RpcServerEvent(listen_fd_,
                                         EVENT_READ, this);
  dispatcher_.AddEvent(event_);
  ASSERT_NE(static_cast<RpcServerEvent*>(NULL), event_);
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
}

void RpcServer::Stop() {
  dispatcher_.Stop();
}

int RpcServer::HandleAccept() {
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
    connection->set_dispacher(&dispatcher_);
    dispatcher_.AddEvent(connection->event());
  }
}

}; // namespace eventrpc

