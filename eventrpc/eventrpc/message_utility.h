/*
 * Copyright(C) lichuang
 */
#ifndef __EVENTRPC_MESSAGE_UTILITY_H_
#define __EVENTRPC_MESSAGE_UTILITY_H_
#include <google/protobuf/message.h>
#include <eventrpc/message_header.h>
namespace eventrpc {
bool EncodeMessage(const google::protobuf::Message& message,
                   string *output);
bool DecodeMessageHeader(const string &input,
                         MessageHeader *message_header);
};
#endif  // __EVENTRPC_MESSAGE_UTILITY_H_
