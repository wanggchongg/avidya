
#include <string.h>
#include <arpa/inet.h>
#include "utility.h"
#include "meta.h"

namespace eventrpc {

Meta::Meta()
  : id_(0),
    len_(0) {
}

Meta::~Meta() {
}

void Meta::set_message_len(int len) {
  len_ = len;
}

void Meta::set_method_id(const string &method_fullname) {
  id_ = hash_string(method_fullname);
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

void Meta::Encode(const char *buffer) {
  uint32_t tmp;
  memcpy(buffer_, buffer, META_LEN);
  memcpy(reinterpret_cast<char*>(&tmp),
         buffer, sizeof(tmp));
  id_ = ntohl(tmp);
  memcpy(reinterpret_cast<char*>(&tmp),
         buffer + sizeof(tmp), sizeof(tmp));
  len_ = ntohl(tmp);
}

void Meta::EncodeWithMessage(const string &method_fullname,
                                   const gpb::Message *message,
                                   string *buffer) {
  set_method_id(method_fullname);
  set_message_len(message->ByteSize());
  buffer->append(Decode(), META_LEN);
  message->AppendToString(buffer);
}

const char* Meta::Decode() {
  uint32_t tmp = htonl(id_);
  memcpy(buffer_, reinterpret_cast<char*>(&tmp), sizeof(tmp));
  tmp = htonl(len_);
  memcpy(buffer_ + sizeof(tmp),
         reinterpret_cast<char*>(&tmp), sizeof(tmp));
  return buffer_;
}

};
