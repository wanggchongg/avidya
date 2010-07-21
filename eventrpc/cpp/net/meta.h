#ifndef __EVENTRPC_META_H__
#define __EVENTRPC_META_H__

#include <google/protobuf/message.h>
#include <string>
#include "base/base.h"

using std::string;

EVENTRPC_NAMESPACE_BEGIN

enum Request_State {
  INIT,
  SEND_REQUEST,
  READ_META,
  READ_MESSAGE,
};

// one for method id, the other for message length
#define META_LEN (sizeof(uint32_t) + sizeof(uint32_t))

// meta infomation for proto message
class Meta {
 public:
  Meta();

  ~Meta();

  void Encode(const char *buffer);

  void EncodeWithMessage(const string &method_fullname,
                         const gpb::Message *message,
                         string *buffer);

  const char* Decode();

  void set_message_len(int len);

  void set_method_id(const string &method_fullname);

  uint32_t message_len();

  uint32_t method_id();

 private:
  struct Impl;
  Impl *impl_;
};

EVENTRPC_NAMESPACE_END
#endif  //  __EVENTRPC_META_H__
