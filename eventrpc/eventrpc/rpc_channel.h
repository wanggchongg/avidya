#ifndef __EVENTRPC_RPC_CHANNEL_H__
#define __EVENTRPC_RPC_CHANNEL_H__
#include <google/protobuf/service.h>
#include "base.h"
#include "event.h"
#include "meta.h"
namespace eventrpc {
class Dispatcher;
class RpcChannel : public gpb::RpcChannel {
 public:
  RpcChannel(const char *host, int port, Dispatcher *dispatcher);

  virtual ~RpcChannel();

  bool Connect();

  void Close();

  virtual void CallMethod(const gpb::MethodDescriptor* method,
                          gpb::RpcController* controller,
                          const gpb::Message* request,
                          gpb::Message* response,
                          gpb::Closure* done);
 private:
  int HandleRead();

  int HandleWrite();

  struct RpcChannelEvent : public Event {
    RpcChannelEvent(int fd, RpcChannel *channel)
      : Event(fd, EVENT_WRITE | EVENT_READ),
        channel_(channel) {
    }

    virtual ~RpcChannelEvent() {
    }

    int HandleRead();

    int HandleWrite();

    RpcChannel *channel_;
  };

  friend class RpcChannelEvent;
  RpcChannelEvent event_;
  const char *host_;
  int port_;
  Dispatcher *dispatcher_;
  Meta meta_;
  gpb::Closure *done_;
  gpb::Message *response_;
  RequestState state_;
  static const int BUFFER_LENGTH = 100;
  char buf_[BUFFER_LENGTH];
  ssize_t count_;
  ssize_t sent_count_;
  uint32 method_id_;
  string message_;
};
};
#endif // __EVENTRPC_RPC_CHANNEL_H__
