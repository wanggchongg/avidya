/*
 * Copyright(C) lichuang
 */
#ifndef __EVENTRPC_MESSAGE_SERVER_H__
#define __EVENTRPC_MESSAGE_SERVER_H__
#include <string>
#include "eventrpc/base.h"
#include "eventrpc/buffer.h"
#include "eventrpc/dispatcher.h"
#include "eventrpc/message_connection.h"
#include "eventrpc/message_header.h"
#include "eventrpc/message_handler.h"
#include "eventrpc/message_utility.h"
namespace eventrpc {
class MessageServer {
 public:
  MessageServer(const string &host, int port);

  ~MessageServer();

  void Start();

  void Stop();

  void set_dispatcher(Dispatcher *dispatcher);

  void set_message_handler_factory(ServerMessageHandlerFactory *factory);
  struct Impl;
 private:
  Impl *impl_;
};
};
#endif  //  __EVENTRPC_MESSAGE_SERVER_H__
