/*
 * Copyright(C) lichuang
 */
#include <arpa/inet.h>  // htonl, ntohl
#include "eventrpc/utility.h"
#include "eventrpc/log.h"
#include "eventrpc/string_utility.h"
#include "eventrpc/buffer.h"
#include "eventrpc/message_utility.h"
#include "eventrpc/error_code.h"
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
  message_header->length = ::ntohl(message_length);
  return true;
}

uint32 ReadMessageStateMachine(Buffer *input,
                               MessageHeader *header,
                               ReadMessageState *state) {
  while (true) {
    if (*state == READ_HEADER) {
      if (input->size() < sizeof(MessageHeader)) {
        return kRecvMessageNotCompleted;
      }
      DecodeMessageHeader(input, header);
      *state = READ_MESSAGE;
    }
    if (*state == READ_MESSAGE) {
      if (input->size() < header->length) {
        return kRecvMessageNotCompleted;
      }
      return kSuccess;
    }
  }
  LOG_FATAL() << "should not reach here";
  return kSuccess;
}

uint32 WriteMessage(Buffer *output, int fd) {
  if (output->Write(fd) == -1) {
    return kSendMessageError;
  }
  if (output->is_read_complete() == true) {
    output->Clear();
    return kSuccess;
  }
  return kSendMessageNotCompleted;
}
};
