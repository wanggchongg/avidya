/*
 * Copyright(C) lichuang
 */
#ifndef __EVENTRPC_RPC_SERVER_H__
#define __EVENTRPC_RPC_SERVER_H__
#include <google/protobuf/service.h>
#include "eventrpc/base.h"
#include "eventrpc/message_server.h"
namespace eventrpc {
class RpcServer : public MessageServer {
 public:
  RpcServer(const string &host, int port);

  virtual ~RpcServer();

  void RegisterService(gpb::Service *service);

  struct Impl;
 private:
  Impl *impl_;
};
};
#endif  //  __EVENTRPC_RPC_SERVER_H__
