// Copyright (C) 2013, avidya
// Created by lichuang1982@gmail.com
// Last modified: anqin.qin@gmail.com

#include <sys/socket.h>
#include "eventrpc/error_code.h"
#include "eventrpc/task.h"
#include "eventrpc/event.h"
#include "eventrpc/message_channel.h"
#include "eventrpc/net_address.h"
#include "eventrpc/net_utility.h"
#include "eventrpc/log.h"
namespace eventrpc {
namespace {
static const int32 kMaxTryConnectTime = 5;
};
struct ConnectTask : public Task {
  ConnectTask(MessageChannel::Impl *impl)
    : impl_(impl) {
  }

  virtual ~ConnectTask() {
  }

  void Handle();

  std::string TaskName() {
    return "ConnectTask";
  }
  MessageChannel::Impl *impl_;
};

struct SendPacketTask : public Task {
  SendPacketTask(MessageChannel::Impl *impl,
                 uint32 opcode,
                 const ::google::protobuf::Message *message)
    : impl_(impl),
      opcode_(opcode) {
    message_ = message->New();
    message_->CopyFrom(*message);
  }

  void Handle();

  std::string TaskName() {
    return "SendPacketTask";
  }

  virtual ~SendPacketTask() {
  }

  MessageChannel::Impl *impl_;
  uint32 opcode_;
  ::google::protobuf::Message *message_;
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

  void HandleSendPacket(uint32 opcode,
                        const ::google::protobuf::Message *message);

  void set_message_handler(ChannelMessageHandler *handler);

  void set_dispatcher(Dispatcher *dispatcher);

  Dispatcher* dispatcher() {
    return dispatcher_;
  }

  bool HandleRead();

  bool HandleWrite();

  void ErrorMessage(const string &message);

  void ConnectToServer();

  bool IsConnected() {
    return is_connected_;
  }

 private:
  int fd_;
  bool is_connected_;
  int32 try_connect_count_;
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
    is_connected_(false),
    try_connect_count_(0),
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
  EASSERT_TRUE(dispatcher_ != NULL) << "should be called after set_dispatcher";
  dispatcher_->PushTask(connect_task_);
  return true;
}

void MessageChannel::Impl::ConnectToServer() {
  fd_ = NetUtility::Connect(server_address_);
  if (fd_ > 0) {
    is_connected_ = true;
    VLOG_INFO() << "create connection fd " << fd_
      << " for " << server_address_.DebugString();
    uint32 result = WriteMessage(&output_buffer_, fd_);
    if (result == kSendMessageError) {
      VLOG_ERROR() << "send message to "
        << server_address_.DebugString() << " error";
      Close();
    }
    dispatcher_->AddEvent(fd_, EVENT_READ | EVENT_WRITE, event_handler_);
    return;
  }
  ++try_connect_count_;
  if (try_connect_count_ > kMaxTryConnectTime) {
    VLOG_ERROR() << "try connect to "
      << server_address_.DebugString() << kMaxTryConnectTime
      << "times, now give up and quit";
    Close();
    return;
  }
  VLOG_ERROR() << "try connect to "
    << server_address_.DebugString() << ", " << try_connect_count_
    << " times fail, now try again...";
  dispatcher_->PushTask(new ConnectTask(this));
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
  SendPacketTask *task = new SendPacketTask(this,
                                            opcode,
                                            message);
  dispatcher_->PushTask(task);
}

void MessageChannel::Impl::HandleSendPacket(
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
  uint32 result = WriteMessage(&output_buffer_, fd_);
  if (result == kSendMessageError) {
    VLOG_ERROR() << "send message to "
      << server_address_.DebugString() << " error";
    Close();
  }
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

void SendPacketTask::Handle() {
  impl_->HandleSendPacket(opcode_, message_);
  delete message_;
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

bool MessageChannel::IsConnected() {
  return impl_->IsConnected();
}
};
