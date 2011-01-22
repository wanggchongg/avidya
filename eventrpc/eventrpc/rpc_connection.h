#ifndef __EVENTRPC_RPC_CONNECTION_H__
#define __EVENTRPC_RPC_CONNECTION_H__

#include <string>
#include "event.h"
#include "meta.h"

using std::string;

namespace eventrpc {
class RpcMethodManager;
class RpcConnectionManager;
class RpcConnection {
 public:
  RpcConnection()
    : event_(-1, this),
    state_(READ_META),
    message_(""),
    expect_recv_count_(META_LEN),
    rpc_method_manager_(NULL) {
  }

  void set_fd(int fd) {
    event_.fd_ = fd;
  }

  void set_rpc_method_manager(RpcMethodManager *rpc_method_manager) {
    rpc_method_manager_ = rpc_method_manager;
  }

  void set_rpc_connection_manager(
      RpcConnectionManager *rpc_connection_manager) {
    rpc_connection_manager_ = rpc_connection_manager;
  }

  Event* event() {
    return &event_;
  }

 private:
  int StateMachine();
  struct RpcConnectionEvent : public Event {
    RpcConnectionEvent(int fd, RpcConnection *connection)
      : Event(fd, EVENT_READ),
        connection_(connection) {
    }

    virtual ~RpcConnectionEvent() {
    }

    int HandleRead();

    int HandleWrite();

    RpcConnection *connection_;
  };
 private:
  int HandleRead();

  int HandleWrite();
  friend class RpcConnectionEvent;
  RpcConnectionEvent event_;
  RequestState state_;
  static const int BUFFER_LENGTH = 100;
  char buffer_[BUFFER_LENGTH];
  string message_;
  ssize_t expect_recv_count_;
  RpcMethodManager *rpc_method_manager_;
  RpcConnectionManager *rpc_connection_manager_;
  Meta meta_;
};
};
#endif  // __EVENTRPC_RPC_CONNECTION_H__
