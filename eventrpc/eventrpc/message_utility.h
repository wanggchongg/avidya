/*
 * Copyright(C) lichuang
 */
#ifndef __EVENTRPC_MESSAGE_UTILITY_H_
#define __EVENTRPC_MESSAGE_UTILITY_H_
#include <google/protobuf/message.h>
#include <eventrpc/message_header.h>
namespace eventrpc {
class Buffer;
bool EncodeMessage(const google::protobuf::Message *message,
                   Buffer *output);
bool DecodeMessageHeader(Buffer *input,
                         MessageHeader *message_header);
};
#endif  // __EVENTRPC_MESSAGE_UTILITY_H_
