/*
 * Copyright(C) lichuang
 */
#include "message_connection_manager.h"
#include "message_connection.h"
namespace eventrpc {
MessageConnectionManager::MessageConnectionManager() {
}

MessageConnectionManager::~MessageConnectionManager() {
  for (ConnectionList::iterator iter = connection_list_.begin();
       iter != connection_list_.end(); ++iter) {
    delete (*iter);
  }
  connection_list_.clear();
}

MessageConnection* MessageConnectionManager::GetConnection() {
  if (connection_list_.empty()) {
    return new MessageConnection(this);
  }
  MessageConnection *connection = connection_list_.back();
  connection_list_.pop_back();
  return connection;
}

void MessageConnectionManager::PutConnection(MessageConnection *connection) {
  connection_list_.push_back(connection);
}
};
