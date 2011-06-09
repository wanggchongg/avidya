/*
 * Copyright(C) lichuang
 */
#include <arpa/inet.h>  // htonl, ntohl
#include "utility.h"
#include "string_utility.h"
#include "buffer.h"
#include "message_utility.h"
namespace eventrpc {
bool EncodeMessage(const google::protobuf::Message *message,
                   Buffer *output) {
  if (output == NULL) {
    return false;
  }
  output->Clear();
  uint32 opcode = hash_string(message->GetTypeName());
  opcode = ::htonl(opcode);
  output->SerializeFromUint32(opcode);
  uint32 message_length = ::htonl(message->ByteSize());
  output->SerializeFromUint32(message_length);
  string buffer;
  message->SerializeToString(&buffer);
  output->AppendString(buffer);
  return true;
}

bool DecodeMessageHeader(Buffer *input,
                         MessageHeader *message_header) {
  if (message_header == NULL) {
    return false;
  }
  uint32 opcode = input->DeserializeToUint32();
  message_header->opcode = ::ntohl(opcode);
  uint32 message_length = input->DeserializeToUint32();
  message_header->message_length = ::ntohl(message_length);
  return true;
}
};
