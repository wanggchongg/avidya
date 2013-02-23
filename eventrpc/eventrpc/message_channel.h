// Copyright (C) 2013, avidya
// Created by lichuang1982@gmail.com
// Last modified: anqin.qin@gmail.com

#ifndef __EVENTRPC_MESSAGE_CHANNEL_H__
#define __EVENTRPC_MESSAGE_CHANNEL_H__
#include <string>
#include <google/protobuf/service.h>
#include <google/protobuf/message.h>
#include "eventrpc/base.h"
#include "eventrpc/buffer.h"
#include "eventrpc/message_header.h"
#include "eventrpc/message_handler.h"
#include "eventrpc/message_utility.h"
#include "eventrpc/dispatcher.h"
namespace eventrpc {
class MessageChannel {
 public:
  MessageChannel(const string &host, int port);

  ~MessageChannel();

  bool Connect();

  void Close();

  void SendPacket(uint32 opcode, const ::google::protobuf::Message *message);

  void set_message_handler(ChannelMessageHandler *handler);

  void set_dispatcher(Dispatcher *dispatcher);

  Dispatcher * dispatcher();

  bool IsConnected();

  struct Impl;
 private:
  Impl *impl_;
};
};
#endif // __EVENTRPC_MESSAGE_CHANNEL_H__
