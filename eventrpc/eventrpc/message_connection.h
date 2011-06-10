/*
 * Copyright(C) lichuang
 */
#ifndef __EVENTRPC_MESSAGE_CONNECTION_H__
#define __EVENTRPC_MESSAGE_CONNECTION_H__
#include <arpa/inet.h>
#include <google/protobuf/message.h>
#include "eventrpc/buffer.h"
#include "eventrpc/message_utility.h"
#include "eventrpc/message_header.h"
#include "eventrpc/message_handler.h"
#include "eventrpc/message_connection_manager.h"
namespace eventrpc {
class Dispatcher;
class Event;
class MessageConnection {
 public:
  MessageConnection(MessageConnectionManager *connection_manager);

  ~MessageConnection();

  void set_fd(int fd);

  void set_client_address(struct sockaddr_in address);

  void set_dispacher(Dispatcher *dispatcher);

  void set_message_handler(MessageHandler *handler);

  Event* event();

  void SendMessage(const ::google::protobuf::Message *message);

  void Close();

  struct Impl;
 private:
  struct Impl *impl_;
};
};
#endif  // __EVENTRPC_MESSAGE_CONNECTION_H__
