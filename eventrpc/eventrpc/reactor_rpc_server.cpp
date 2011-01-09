
#include "reactor_rpc_server.h"
#include "log.h"
#include "net_utility.h"

namespace eventrpc {
ReactorRpcServer::ReactorRpcServer()
  : RpcServer() {
}

ReactorRpcServer::~ReactorRpcServer() {
  dispatcher_.DelEvent(event_);
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
  char buf[100];
  int len;
  for (int i = 0; i < 200; ++i) {
    fd = NetUtility::Accept(listen_fd_);
    if (fd == -1) {
      break;
    }
    if (!NetUtility::Recv(fd, buf, 100, &len)) {
      continue;
    }
    printf("buf: %s\n", buf);
  }
}

}; // namespace eventrpc
