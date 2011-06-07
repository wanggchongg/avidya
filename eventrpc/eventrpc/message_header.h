/*
 * Copyright(C) lichuang
 */
#ifndef __EVENTRPC_MESSAGE_HEADER_H_
#define __EVENTRPC_MESSAGE_HEADER_H_
#include <string>
#include <eventrpc/base.h>
namespace eventrpc {
struct MessageHeader {
  uint32 opcode;
  uint32 message_length;
};

struct Message {
  MessageHeader header;
  string content;
};

struct MessageHandler {
  virtual ~MessageHandler() {}
  virtual bool Handle() = 0;
 private:
  MessageHandler() {}
};
};
#endif  // __EVENTRPC_MESSAGE_HEADER_H_
