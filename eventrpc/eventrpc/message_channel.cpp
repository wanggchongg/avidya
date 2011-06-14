/*
 * Copyright(C) lichuang
 */
#include <sys/socket.h>
#include "eventrpc/error_code.h"
#include "eventrpc/task.h"
#include "eventrpc/event.h"
#include "eventrpc/message_channel.h"
#include "eventrpc/net_address.h"
#include "eventrpc/net_utility.h"
#include "eventrpc/log.h"
#include "eventrpc/assert_log.h"
namespace eventrpc {
struct ConnectTask : public Task {
  ConnectTask(MessageChannel::Impl *impl)
    : impl_(impl) {
  }

  void Handle();

  MessageChannel::Impl *impl_;
};

struct MessageChannelEventHandler : public EventHandler {
  MessageChannelEventHandler(MessageChannel::Impl *impl)
    : impl_(impl) {
  }

  virtual ~MessageChannelEventHandler() {
  }

  bool HandleRead();

  bool HandleWrite();

  MessageChannel::Impl *impl_;
};

struct MessageChannel::Impl {
 public:
  Impl(const string &host, int port);

  ~Impl();

  bool Connect();

  void Close();

  void SendPacket(uint32 opcode, const ::google::protobuf::Message *message);

  void set_message_handler(ChannelMessageHandler *handler);

  void set_dispatcher(Dispatcher *dispatcher);

  Dispatcher* dispatcher() {
    return dispatcher_;
  }

  bool HandleRead();

  bool HandleWrite();

  void ErrorMessage(const string &message);

  void ConnectToServer();

 private:
  int fd_;
  NetAddress server_address_;
  Buffer input_buffer_;
  Buffer output_buffer_;
  Dispatcher *dispatcher_;
  MessageChannelEventHandler *event_handler_;
  ConnectTask *connect_task_;
  ChannelMessageHandler *handler_;
  MessageHeader message_header_;
  ReadMessageState state_;
};

MessageChannel::Impl::Impl(const string &host, int port)
  : fd_(-1),
    server_address_(host, port),
    dispatcher_(NULL),
    event_handler_(new MessageChannelEventHandler(this)),
    connect_task_(new ConnectTask(this)),
    handler_(NULL),
    state_(READ_HEADER) {
}

MessageChannel::Impl::~Impl() {
  Close();
  delete event_handler_;
}

bool MessageChannel::Impl::Connect() {
  ASSERT_TRUE(dispatcher_ != NULL) << "should be called after set_dispatcher";
  dispatcher_->PushTask(connect_task_);
  return true;
}

void MessageChannel::Impl::ConnectToServer() {
  fd_ = NetUtility::Connect(server_address_);
  bool is_connected = (fd_ > 0);
  VLOG_INFO() << "create connection fd " << fd_
    << " for " << server_address_.DebugString();
  handler_->HandleConnection(is_connected);
  dispatcher_->AddEvent(fd_, EVENT_READ | EVENT_WRITE, event_handler_);
}

void MessageChannel::Impl::Close() {
  if (fd_ > 0) {
    VLOG_INFO() << "close connection to "
      << server_address_.DebugString() << ", fd: " << fd_;
    shutdown(fd_, SHUT_WR);
    fd_ = -1;
  }
}

void MessageChannel::Impl::SendPacket(
    uint32 opcode,
    const ::google::protobuf::Message *message) {
  EncodePacket(opcode, message, &output_buffer_);
}

void MessageChannel::Impl::set_message_handler(ChannelMessageHandler *handler) {
  handler_ = handler;
}

void MessageChannel::Impl::set_dispatcher(Dispatcher *dispatcher) {
  dispatcher_ = dispatcher;
}

bool MessageChannel::Impl::HandleRead() {
  VLOG_ERROR() << "HandleRead";
  if (input_buffer_.Read(fd_) == -1) {
    VLOG_ERROR() << "recv message from "
      << server_address_.DebugString() << " error";
    Close();
    return false;
  }
  while (!input_buffer_.is_read_complete()) {
    uint32 result = ReadMessageStateMachine(&input_buffer_,
                                            &message_header_,
                                            &state_);
    if (result == kRecvMessageNotCompleted) {
      return result;
    }
    if (!handler_->HandlePacket(message_header_, &input_buffer_)) {
    VLOG_ERROR() << "handle message from "
      << server_address_.DebugString() << " error";
    Close();
    return false;
    }
  }
  input_buffer_.Clear();
  return true;
}

bool MessageChannel::Impl::HandleWrite() {
  if (output_buffer_.is_read_complete()) {
    return true;
  }
  VLOG_ERROR() << "before write: " << output_buffer_.size();
  uint32 result = WriteMessage(&output_buffer_, fd_);
  if (result == kSendMessageError) {
    VLOG_ERROR() << "send message to "
      << server_address_.DebugString() << " error";
    Close();
  }
  VLOG_ERROR() << "after write: " << output_buffer_.size();
  return (result == kSuccess);
}

bool MessageChannelEventHandler::HandleRead() {
  return impl_->HandleRead();
}

bool MessageChannelEventHandler::HandleWrite() {
  return impl_->HandleWrite();
}

void ConnectTask::Handle() {
  impl_->ConnectToServer();
}

MessageChannel::MessageChannel(const string &host, int port) {
   impl_ = new Impl(host, port);
}

MessageChannel::~MessageChannel() {
  delete impl_;
}

bool MessageChannel::Connect() {
  return impl_->Connect();
}

void MessageChannel::Close() {
  impl_->Close();
}

void MessageChannel::SendPacket(
    uint32 opcode,
    const ::google::protobuf::Message *message) {
  impl_->SendPacket(opcode, message);
}

void MessageChannel::set_message_handler(ChannelMessageHandler *handler) {
  impl_->set_message_handler(handler);
}

void MessageChannel::set_dispatcher(Dispatcher *dispatcher) {
  impl_->set_dispatcher(dispatcher);
}

Dispatcher* MessageChannel::dispatcher() {
  return impl_->dispatcher();
}
};
