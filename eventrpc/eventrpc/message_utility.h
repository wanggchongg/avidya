/*
 * Copyright(C) lichuang
 */
#ifndef __EVENTRPC_MESSAGE_UTILITY_H_
#define __EVENTRPC_MESSAGE_UTILITY_H_
#include <google/protobuf/message.h>
#include "eventrpc/message_header.h"
namespace eventrpc {
class Buffer;
bool EncodePacket(uint32 opcode,
                  const google::protobuf::Message *message,
                  Buffer *output);

bool DecodeMessageHeader(Buffer *input,
                         MessageHeader *message_header);

uint32 ReadMessageStateMachine(Buffer *input,
                               MessageHeader *header,
                               ReadMessageState *state);

uint32 WriteMessage(Buffer *output, int fd);
};
#endif  // __EVENTRPC_MESSAGE_UTILITY_H_
