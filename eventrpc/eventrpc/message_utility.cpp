/*
 * Copyright(C) lichuang
 */
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
  output->SerializeFromUint32(opcode);
  output->SerializeFromUint32(message->ByteSize());
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
  message_header->opcode = input->DeserializeToUint32();
  message_header->length = input->DeserializeToUint32();
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
