/*
 * Copyright(C) lichuang
 */
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include "eventrpc/base.h"
#include "eventrpc/net_utility.h"
#include "eventrpc/log.h"
#include "eventrpc/buffer.h"
namespace eventrpc {
template <class Type>
static Type DeserializeBufferToValue(Buffer *input) {
  Type result;
  memcpy((char*)(&result), input->read_content(), sizeof(Type));
  input->ReadSkip(sizeof(Type));
  return result;
}

template <class Type>
static void SerializeBufferFromValue(Type &value, Buffer *buffer) {
  memcpy((char*)(buffer->write_content()), &value, sizeof(Type));
  buffer->WriteSkip(sizeof(Type));
}

Buffer::Buffer()
  : read_index_(0),
    write_index_(0) {
  buffer_.reserve(buffer_size_);
}

Buffer::~Buffer() {
}

int Buffer::Read(int fd) {
  int to_read = 0;
  if (ioctl(fd, FIONREAD, &to_read) == -1) {
    LOG_ERROR() << "ioctl error: " << strerror(errno);
    return -1;
  }
  if (static_cast<uint32>(to_read) > (buffer_.capacity() - write_index_)) {
    buffer_.resize(to_read + write_index_);
  }
  int32 length = 0;
  if (!NetUtility::Recv(fd, write_content(), to_read, &length)) {
    return -1;
  }
  write_index_ += length;
  return length;
}

int Buffer::Write(int fd) {
  int send_length = 0, length = 0;
  while (true) {
    if (!NetUtility::Send(fd, read_content(),
                          write_index_ - read_index_,
                          &length)) {
      return -1;
    }
    read_index_ += length;
    send_length += length;
    if (read_index_ == write_index_) {
      break;
    }
  }
  return send_length;
}

void Buffer::ReadSkip(int size) {
  if (size + read_index_ < buffer_.capacity()) {
    read_index_ += size;
  }
}

void Buffer::WriteSkip(int size) {
  if (size + write_index_ < buffer_.capacity()) {
    write_index_ += size;
  }
}

void Buffer::SerializeFromUint32(uint32 value) {
  SerializeBufferFromValue<uint32>(value, this);
}

uint32 Buffer::DeserializeToUint32() {
  return DeserializeBufferToValue<uint32>(this);
}

void Buffer::AppendString(const string &content) {
  if (content.size() > (buffer_.capacity() - write_index_)) {
    buffer_.resize(content.size() + write_index_);
  }
  memcpy(write_content(), content.c_str(), content.size());
  write_index_ += content.size();
}

string Buffer::ToString(int size) {
  string content;
  content.append(read_content(), size);
  read_index_ += size;
  return content;
}
};
