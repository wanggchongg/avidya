/*
 * Copyright(C) lichuang
 */
#ifndef __EVENTRPC_MESSAGE_SERVER_H__
#define __EVENTRPC_MESSAGE_SERVER_H__

#include <string>
using std::string;
namespace eventrpc {
class MessageHandler;
class MessageServer {
 public:
  MessageServer();

  ~MessageServer();

  void Start();

  void Stop();

  void set_host_and_port(const string &host, uint32 port);

  void set_dispatcher(Dispatcher *dispatcher);

  struct Impl;
 private:
  Impl *impl_;
};
};
#endif  //  __EVENTRPC_MESSAGE_SERVER_H__
