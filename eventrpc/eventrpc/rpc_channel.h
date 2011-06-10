/*
 * Copyright(C) lichuang
 */
#ifndef __EVENTRPC_RPC_CHANNEL_H__
#define __EVENTRPC_RPC_CHANNEL_H__
#include <string>
#include <google/protobuf/service.h>
#include "eventrpc/message_channel.h"
namespace eventrpc {
class Dispatcher;
class RpcChannel : public gpb::RpcChannel,
                   public MessageChannel {
 public:
  RpcChannel(const string &host, int port);

  virtual ~RpcChannel();

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
