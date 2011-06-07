/*
 * Copyright(C) lichuang
 */
#include <arpa/inet.h>  // htonl, ntohl
#include "utility.h"
#include "string_utility.h"
#include "message_utility.h"
namespace eventrpc {
bool EncodeMessage(const google::protobuf::Message& message,
                   string *output) {
  if (output == NULL) {
    return false;
  }
  uint32 opcode = hash_string(message.GetTypeName());
  opcode = ::htonl(opcode);
  output->append(reinterpret_cast<char*>(&opcode), sizeof(opcode));
  uint32 message_length = ::htonl(message.ByteSize());
  output->append(reinterpret_cast<char*>(&message_length),
                 sizeof(message_length));
  string buffer;
  message.SerializeToString(&buffer);
  output->append(buffer);
  return true;
}

bool DecodeMessageHeader(const string &input,
                         MessageHeader *message_header) {
  if (message_header == NULL) {
    return false;
  }
  uint32 pos = 0;
  uint32 opcode = StringUtility::DeserializeStringToUint32(input.substr(pos, sizeof(opcode)));
  message_header->opcode = ::ntohl(opcode);
  pos += sizeof(opcode);
  uint32 message_length = StringUtility::DeserializeStringToUint32(
      input.substr(pos, sizeof(message_length)));
  message_header->message_length = ::ntohl(message_length);
  return true;
}
};
