#ifndef __EVENTRPC_META_H__
#define __EVENTRPC_META_H__

#include <google/protobuf/message.h>
#include <eventrpc/base.h>
#include <stdint.h>
#include <string>

using std::string;

namespace eventrpc {
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

  uint32_t message_len() {
    return len_;
  }

  uint32_t method_id() {
    return id_;
  }

 private:
  char buffer_[META_LEN];
  uint32_t id_;
  uint32_t len_;

  struct Impl;
  Impl *impl_;
};

};
#endif  //  __EVENTRPC_META_H__
