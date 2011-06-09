/*
 * Copyright(C) lichuang
 */
#ifndef __EVENTRPC_MESSAGE_CHANNEL_H__
#define __EVENTRPC_MESSAGE_CHANNEL_H__
#include <string>
#include <google/protobuf/service.h>
using std::string;
namespace eventrpc {
class Dispatcher;
class MessageHandler;
class MessageChannel {
 public:
  MessageChannel(const string &host, int port);

  ~MessageChannel();

  bool Connect();

  void Close();

  void SendMessage(const ::google::protobuf::Message *message);

  void set_message_handler(MessageHandler *handler);

  void set_dispatcher(Dispatcher *dispatcher);

  struct Impl;
 private:
  Impl *impl_;
};
};
#endif // __EVENTRPC_MESSAGE_CHANNEL_H__
