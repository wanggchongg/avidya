/*
 * Copyright(C) lichuang
 */
#include <map>
#include <list>
#include <string>
#include "eventrpc/error_code.h"
#include "eventrpc/log.h"
#include "eventrpc/event.h"
#include "eventrpc/dispatcher.h"
#include "eventrpc/net_utility.h"
#include "eventrpc/message_connection.h"
#include "eventrpc/buffer.h"
using namespace std;
namespace eventrpc {
struct MessageConnectionEventHandler : public EventHandler {
  MessageConnectionEventHandler(MessageConnection::Impl *impl)
    : impl_(impl) {
  }

  virtual ~MessageConnectionEventHandler() {
  }

  bool HandleRead();

  bool HandleWrite();

  MessageConnection::Impl *impl_;
};

struct MessageConnection::Impl {
  Impl(MessageConnection *connection, MessageConnectionManager *manager);

  ~Impl();

  void set_fd(int fd);

  void set_client_address(const NetAddress &address);

  void set_dispacher(Dispatcher *dispatcher);

  void set_message_handler(MessageHandler *handler);

  EventHandler* event_handler();

  void SendPacket(uint32 opcode,
                  const ::google::protobuf::Message *message);

  void Close();

  bool HandleRead();

  bool HandleWrite();

 private:
  int fd_;
  MessageConnection *connection_;
  MessageConnectionEventHandler *event_handler_;
  NetAddress client_address_;
  MessageConnectionManager *connection_manager_;
  Dispatcher *dispatcher_;
  Buffer input_buffer_;
  Buffer output_buffer_;
  MessageHandler *message_handler_;
  MessageHeader message_header_;
  ReadMessageState state_;
};

MessageConnection::Impl::Impl(MessageConnection *connection,
                              MessageConnectionManager *manager)
  : fd_(-1),
    connection_(connection),
    event_handler_(new MessageConnectionEventHandler(this)),
    connection_manager_(manager),
    message_handler_(NULL),
    state_(READ_HEADER) {
}

MessageConnection::Impl::~Impl() {
  Close();
  delete event_handler_;
}

bool MessageConnection::Impl::HandleRead() {
  if (input_buffer_.Read(fd_) == -1) {
    VLOG_ERROR() << "recv message from "
      << client_address_.DebugString() << " error";
    Close();
    return false;
  }
  while (!input_buffer_.is_read_complete()) {
    uint32 result = ReadMessageStateMachine(&input_buffer_,
                                            &message_header_,
                                            &state_);
    if (result == kRecvMessageNotCompleted) {
      return false;
    }
    if (!message_handler_->HandlePacket(message_header_, &input_buffer_)) {
    VLOG_ERROR() << "handle message from "
      << client_address_.DebugString() << " error";
    Close();
    }
  }
  input_buffer_.Clear();
  return true;
}

bool MessageConnection::Impl::HandleWrite() {
  if (output_buffer_.is_read_complete()) {
    VLOG_INFO() << "read complete";
    return true;
  }
  uint32 result = WriteMessage(&output_buffer_, fd_);
  if (result == kSendMessageError) {
    VLOG_ERROR() << "send message to "
      << client_address_.DebugString() << " error";
    Close();
  }
  return (result == kSuccess);
}

void MessageConnection::Impl::set_fd(int fd) {
  fd_ = fd;
}

void MessageConnection::Impl::set_client_address(const NetAddress &address) {
  client_address_ = address;
}

void MessageConnection::Impl::set_dispacher(Dispatcher *dispatcher) {
  dispatcher_ = dispatcher;
}

void MessageConnection::Impl::set_message_handler(MessageHandler *handler) {
  message_handler_ = handler;
}

void MessageConnection::Impl::SendPacket(
    uint32 opcode, const ::google::protobuf::Message *message) {
  EncodePacket(opcode, message, &output_buffer_);
}

EventHandler* MessageConnection::Impl::event_handler() {
  return event_handler_;
}

void MessageConnection::Impl::Close() {
  if (fd_ > 0) {
    shutdown(fd_, SHUT_WR);
    fd_ = -1;
    connection_manager_->PutConnection(connection_);
    input_buffer_.Clear();
    output_buffer_.Clear();
    state_ = READ_HEADER;
  }
}

MessageConnection::MessageConnection(MessageConnectionManager *manager) {
  impl_ = new Impl(this, manager);
}

MessageConnection::~MessageConnection() {
  delete impl_;
}

void MessageConnection::set_fd(int fd) {
  impl_->set_fd(fd);
}

void MessageConnection::set_client_address(const NetAddress &address) {
  impl_->set_client_address(address);
}

void MessageConnection::set_dispacher(Dispatcher *dispatcher) {
  impl_->set_dispacher(dispatcher);
}

void MessageConnection::set_message_handler(MessageHandler *handler) {
  impl_->set_message_handler(handler);
}

bool MessageConnectionEventHandler::HandleRead() {
  return impl_->HandleRead();
}

bool MessageConnectionEventHandler::HandleWrite() {
  return impl_->HandleWrite();
}

void MessageConnection::SendPacket(
    uint32 opcode, const ::google::protobuf::Message *message) {
  impl_->SendPacket(opcode, message);
}

EventHandler* MessageConnection::event_handler() {
  return impl_->event_handler();
}

void MessageConnection::Close() {
  impl_->Close();
}
};
