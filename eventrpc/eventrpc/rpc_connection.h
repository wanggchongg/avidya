/*
 * Copyright(C) lichuang
 */
#ifndef __EVENTRPC_RPC_CONNECTION_H__
#define __EVENTRPC_RPC_CONNECTION_H__
#include <arpa/inet.h>
namespace eventrpc {
class RpcMethodManager;
class RpcConnectionManager;
class Dispatcher;
class Event;
class RpcConnection {
 public:
  RpcConnection();

  ~RpcConnection();

  void set_fd(int fd);

  void set_client_address(struct sockaddr_in address);

  void set_rpc_method_manager(RpcMethodManager *rpc_method_manager);

  void set_rpc_connection_manager(
      RpcConnectionManager *rpc_connection_manager);

  void set_dispacher(Dispatcher *dispatcher);

  Event* event();

  void Close();

  struct Impl;
 private:
  struct Impl *impl_;
};
};
#endif  // __EVENTRPC_RPC_CONNECTION_H__
