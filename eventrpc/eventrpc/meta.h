#ifndef __EVENTRPC_META_H__
#define __EVENTRPC_META_H__

#include <google/protobuf/message.h>
#include <string>
#include "base.h"

using std::string;

namespace eventrpc {
// one for method id, second for message length, third for request id
#define META_LEN (sizeof(uint32) + sizeof(uint32) + sizeof(uint32))

// meta infomation for proto message
class Meta {
 public:
  Meta();

  ~Meta();

  void EncodeWithBuffer(const char *buffer);

  void EncodeWithMessage(const string &method_fullname,
                         const gpb::Message *message,
                         string *buffer);

  const char* Decode();

  void set_message_length(uint32 message_length);

  void set_method_id(const string &method_fullname);

  uint32 message_length() const;

  uint32 method_id() const;

  uint64 request_id() const;

 private:
  char buffer_[META_LEN];
  uint32 method_id_;
  uint32 message_length_;
  uint32 request_id_;
};
};
#endif  //  __EVENTRPC_META_H__
