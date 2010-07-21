
#include <string.h>
#include <arpa/inet.h>
#include "util/utility.h"
#include "net/meta.h"

EVENTRPC_NAMESPACE_BEGIN

struct Meta::Impl {
 public:
  Impl()
    : id_(0)
    , len_(0) {
  }

  ~Impl() {
  }

  void Encode(const char *buffer);

  void EncodeWithMessage(const string &method_fullname,
                         const gpb::Message *message,
                         string *buffer);

  const char* Decode();

  void set_message_len(int len) {
    len_ = len;
  }

  void set_method_id(const string &method_fullname) {
    id_ = hash_string(method_fullname);
  }

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
};

#if 0
static void DumpBuffer(const char *buffer) {
  printf("=====DumpBuffer:=====\n");
  for (uint32_t i = 0; i < META_LEN; ++i) {
    printf("%x", buffer[i]);
  }
  printf("\n=====================\n");
}
#endif

void Meta::Impl::Encode(const char *buffer) {
  uint32_t tmp;
  memcpy(buffer_, buffer, META_LEN);
  memcpy(reinterpret_cast<char*>(&tmp),
         buffer, sizeof(tmp));
  id_ = ntohl(tmp);
  memcpy(reinterpret_cast<char*>(&tmp),
         buffer + sizeof(tmp), sizeof(tmp));
  len_ = ntohl(tmp);
}

void Meta::Impl::EncodeWithMessage(const string &method_fullname,
                                   const gpb::Message *message,
                                   string *buffer) {
  set_method_id(method_fullname);
  set_message_len(message->ByteSize());
  buffer->append(Decode(), META_LEN);
  message->AppendToString(buffer);
}

const char* Meta::Impl::Decode() {
  uint32_t tmp = htonl(id_);
  memcpy(buffer_, reinterpret_cast<char*>(&tmp), sizeof(tmp));
  tmp = htonl(len_);
  memcpy(buffer_ + sizeof(tmp),
         reinterpret_cast<char*>(&tmp), sizeof(tmp));
  return buffer_;
}

Meta::Meta()
  : impl_(new Impl) {
}

Meta::~Meta() {
  delete impl_;
}

void Meta::Encode(const char *buffer) {
  impl_->Encode(buffer);
}

void Meta::EncodeWithMessage(const string &method_fullname,
                                   const gpb::Message *message,
                                   string *buffer) {
  impl_->EncodeWithMessage(method_fullname, message, buffer);
}

const char* Meta::Decode() {
  return impl_->Decode();
}

void Meta::set_message_len(int len) {
  return impl_->set_message_len(len);
}

void Meta::set_method_id(const string &method_fullname) {
  return impl_->set_method_id(method_fullname);
}

uint32_t Meta::message_len() {
  return impl_->message_len();
}

uint32_t Meta::method_id() {
  return impl_->method_id();
}

EVENTRPC_NAMESPACE_END
