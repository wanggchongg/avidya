
#include <string.h>
#include <arpa/inet.h>
#include "log.h"
#include "meta.h"
#include "utility.h"

namespace eventrpc {
Meta::Meta()
  : method_id_(0),
    message_length_(0),
    request_id_(0) {
}

Meta::~Meta() {
}

void Meta::set_message_length(uint32 message_length) {
  message_length_ = message_length;
}

void Meta::set_method_id(const string &method_fullname) {
  method_id_ = hash_string(method_fullname);
}

uint32 Meta::message_length() const{
  return message_length_;
}

uint32 Meta::method_id() const{
  return method_id_;
}

uint64 Meta::request_id() const{
  return request_id_;
}

void Meta::set_request_id(uint32 request_id) {
  request_id_ = request_id;
}
#if 0
static void DumpBuffer(const char *buffer) {
  printf("=====DumpBuffer:=====\n");
  for (uint32_t i = 0; i < META_LEN; ++i) {
    printf("%x", buffer[i]);
  }
  printf("\n=====================\n");
}
#endif

void Meta::EncodeWithBuffer(const char *buffer) {
  uint32 tmp = 0;
  memcpy(buffer_, buffer, META_LEN);
  memcpy(reinterpret_cast<char*>(&tmp),
         buffer, sizeof(tmp));
  method_id_ = ntohl(tmp);
  buffer += sizeof(tmp);

  tmp = 0;
  memcpy(reinterpret_cast<char*>(&tmp),
         buffer, sizeof(tmp));
  message_length_ = ntohl(tmp);
  buffer += sizeof(tmp);

  memcpy(reinterpret_cast<char*>(&request_id_),
         buffer, sizeof(request_id_));
  request_id_ = ntohl(request_id_);
}

void Meta::EncodeWithMessage(const string &method_fullname,
                             const gpb::Message *message,
                             string *buffer) {
  *buffer = "";
  set_method_id(method_fullname);
  set_message_length(message->ByteSize());
  buffer->append(Decode(), META_LEN);
  message->AppendToString(buffer);
}

const char* Meta::Decode() {
  uint32 tmp = htonl(method_id_);
  memcpy(buffer_, reinterpret_cast<char*>(&tmp), sizeof(tmp));

  tmp = htonl(message_length_);
  memcpy(buffer_ + sizeof(tmp),
         reinterpret_cast<char*>(&tmp), sizeof(tmp));

  tmp = htonl(request_id_);
  memcpy(buffer_ + sizeof(request_id_) + sizeof(tmp),
         reinterpret_cast<char*>(&tmp), sizeof(tmp));
  return buffer_;
}
};
