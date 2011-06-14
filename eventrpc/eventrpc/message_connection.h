/*
 * Copyright(C) lichuang
 */
#ifndef __EVENTRPC_MESSAGE_CONNECTION_H__
#define __EVENTRPC_MESSAGE_CONNECTION_H__
#include <arpa/inet.h>
#include <google/protobuf/message.h>
#include "eventrpc/buffer.h"
#include "eventrpc/net_address.h"
#include "eventrpc/message_utility.h"
#include "eventrpc/message_header.h"
#include "eventrpc/message_handler.h"
#include "eventrpc/message_connection_manager.h"
namespace eventrpc {
class Dispatcher;
class EventHandler;
class MessageConnection {
 public:
  MessageConnection(MessageConnectionManager *connection_manager);

  ~MessageConnection();

  void set_fd(int fd);

  void set_client_address(const NetAddress &address);

  void set_dispacher(Dispatcher *dispatcher);

  void set_message_handler(MessageHandler *handler);

  EventHandler* event_handler();

  void SendPacket(uint32 opcode, const ::google::protobuf::Message *message);

  void Close();

  struct Impl;
 private:
  struct Impl *impl_;
};
};
#endif  // __EVENTRPC_MESSAGE_CONNECTION_H__
