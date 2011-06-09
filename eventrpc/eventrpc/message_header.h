/*
 * Copyright(C) lichuang
 */
#ifndef __EVENTRPC_MESSAGE_HEADER_H_
#define __EVENTRPC_MESSAGE_HEADER_H_
#include <string>
#include "eventrpc/base.h"
using std::string;
namespace eventrpc {
struct MessageHeader {
  uint32 opcode;
  uint32 message_length;
}__attribute__((packed));

struct MessageHandler {
  virtual ~MessageHandler() {}
  virtual bool HandlePacket(uint32 opcode, string *packet) = 0;
 private:
  MessageHandler() {}
};
};
#endif  // __EVENTRPC_MESSAGE_HEADER_H_
