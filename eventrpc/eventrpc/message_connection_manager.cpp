/*
 * Copyright(C) lichuang
 */
#include "message_connection_manager.h"
#include "message_connection.h"
namespace eventrpc {
MessageConnectionManager::MessageConnectionManager()
  : factory_(NULL) {
}

MessageConnectionManager::~MessageConnectionManager() {
  for (ConnectionList::iterator iter = connection_list_.begin();
       iter != connection_list_.end(); ++iter) {
    delete (*iter);
  }
  connection_list_.clear();
}

void MessageConnectionManager::set_message_handler_factory(
    ServerMessageHandlerFactory *factory) {
  factory_ = factory;
}

MessageConnection* MessageConnectionManager::GetConnection() {
  if (connection_list_.empty()) {
    MessageConnection *connection = new MessageConnection(this);
    ServerMessageHandler *handler = factory_->CreateHandler(connection);
    connection->set_message_handler(handler);
    return connection;
  }
  MessageConnection *connection = connection_list_.back();
  connection_list_.pop_back();
  return connection;
}

void MessageConnectionManager::PutConnection(MessageConnection *connection) {
  connection_list_.push_back(connection);
}
};
