/*
 * Copyright(C) lichuang
 */
#ifndef __EVENTRPC_RPC_CHANNEL_H__
#define __EVENTRPC_RPC_CHANNEL_H__
#include <google/protobuf/service.h>
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
  struct Impl;
 private:
  Impl *impl_;
};
};
#endif // __EVENTRPC_RPC_CHANNEL_H__
