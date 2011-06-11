/*
 * Copyright(C) lichuang
 */
#ifndef __EVENTRPC_RPC_METHOD_MANAGER_H__
#define __EVENTRPC_RPC_METHOD_MANAGER_H__
#include <google/protobuf/service.h>
#include "eventrpc/base.h"
#include "eventrpc/buffer.h"
#include "eventrpc/message_header.h"
#include "eventrpc/message_connection.h"
namespace eventrpc {
class RpcMethodManager {
 public:
  RpcMethodManager();

  ~RpcMethodManager();

  void RegisterService(gpb::Service *service);

  bool HandlePacket(const MessageHeader &header,
                    Buffer* buffer,
                    MessageConnection *connection);
  struct Impl;
 private:
  Impl *impl_;
};
};
#endif  //  __EVENTRPC_RPC_METHOD_MANAGER_H__
