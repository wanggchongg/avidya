/*
 * Copyright(C) lichuang
 */
#ifndef __EVENTRPC_BUFFER_H__
#define __EVENTRPC_BUFFER_H__
#include <vector>
#include <google/protobuf/message.h>
#include "eventrpc/base.h"
#include "eventrpc/noncopyable.h"
namespace eventrpc {
class Buffer {
  static const int buffer_size_ = 1024;
 public:
  Buffer();
  ~Buffer();

  int Read(int fd);

  int Write(int fd);

  void Clear() {
    buffer_.clear();
    read_index_ = write_index_ = 0;
  }

  char* content() {
    return &(buffer_[0]);
  }

  char* read_content() {
    return &(buffer_[read_index_]);
  }

  char* write_content() {
    return &(buffer_[write_index_]);
  }

  size_t size() {
    return (write_index_ - read_index_);
  }

  void ReadSkip(int size);

  void WriteSkip(int size);

  size_t end_position() const {
    return write_index_;
  }

  bool is_read_complete() const {
    return (read_index_ == write_index_);
  }

  void SerializeFromUint32(uint32 value);

  uint32 DeserializeToUint32();

  void SerializeFromMessage(const ::google::protobuf::Message *message);

  bool DeserializeToMessage(::google::protobuf::Message *message,
                            uint32 length);

  void Resize(int resize);
 private:
  // TODO: replace vector,using POD instead
  vector<char> buffer_;
  size_t read_index_;
  size_t write_index_;
  DISALLOW_EVIL_CONSTRUCTOR(Buffer);
};
};
#endif  // __EVENTRPC_BUFFER_H__
