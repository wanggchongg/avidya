/*
 * Copyright(C) lichuang
 */
#ifndef __EVENTRPC_RPC_CONNECTION_MANAGER_H__
#define __EVENTRPC_RPC_CONNECTION_MANAGER_H__

#include <list>

using std::list;

namespace eventrpc {
class RpcConnection;

class RpcConnectionManager {
 public:
  RpcConnectionManager();
  ~RpcConnectionManager();

  RpcConnection* GetConnection();
  void PutConnection(RpcConnection *connection);

 private:
  typedef list<RpcConnection*> ConnectionList;
  ConnectionList connection_list_;
};
};
#endif  //  __EVENTRPC_RPC_CONNECTION_MANAGER_H__
