/*
 * Copyright(C) lichuang
 */
#ifndef __EVENTRPC_MESSAGE_CONNECTION_MANAGER_H__
#define __EVENTRPC_MESSAGE_CONNECTION_MANAGER_H__
#include <list>
using std::list;
namespace eventrpc {
class MessageConnection;
class ServerMessageHandlerFactory;
class MessageConnectionManager {
 public:
  MessageConnectionManager();
  ~MessageConnectionManager();

  void set_message_handler_factory(ServerMessageHandlerFactory *factory);
  MessageConnection* GetConnection();
  void PutConnection(MessageConnection *connection);

 private:
  typedef list<MessageConnection*> ConnectionList;
  ConnectionList connection_list_;
  ServerMessageHandlerFactory *factory_;
};
};
#endif  //  __EVENTRPC_MESSAGE_CONNECTION_MANAGER_H__
