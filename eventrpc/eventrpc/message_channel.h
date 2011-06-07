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
class RpcMsgChannel {
 public:
  RpcMsgChannel(const string &host, int port, Dispatcher *dispatcher);

  ~RpcMsgChannel();

  bool Connect();

  void Close();

  void SendMessage(const ::google::protobuf::Message *message);

  struct Impl;
 private:
  Impl *impl_;
};
};
#endif // __EVENTRPC_MESSAGE_CHANNEL_H__
