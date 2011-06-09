/*
 * Copyright(C) lichuang
 */
#include "rpc_connection.h"
#include "rpc_connection_manager.h"

namespace eventrpc {
RpcConnectionManager::RpcConnectionManager() {
}

RpcConnectionManager::~RpcConnectionManager() {
  for (ConnectionList::iterator iter = connection_list_.begin();
       iter != connection_list_.end(); ++iter) {
    delete (*iter);
  }
  connection_list_.clear();
}

RpcConnection* RpcConnectionManager::GetConnection() {
  if (connection_list_.empty()) {
    return new RpcConnection();
  }
  RpcConnection *connection = connection_list_.back();
  connection_list_.pop_back();
  return connection;
}

void RpcConnectionManager::PutConnection(RpcConnection *connection) {
  connection_list_.push_back(connection);
}
};
