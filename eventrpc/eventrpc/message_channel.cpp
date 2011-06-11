/*
 * Copyright(C) lichuang
 */
#include "eventrpc/error_code.h"
#include "eventrpc/event.h"
#include "eventrpc/message_channel.h"
#include "eventrpc/net_utility.h"
#include "eventrpc/log.h"
namespace eventrpc {
struct MessageChannelEvent : public Event {
  MessageChannelEvent(int fd, MessageChannel::Impl *impl)
    : Event(fd, EVENT_WRITE | EVENT_READ),
      impl_(impl) {
  }

  virtual ~MessageChannelEvent() {
  }

  int HandleRead();

  int HandleWrite();

  MessageChannel::Impl *impl_;
};

struct MessageChannel::Impl {
 public:
  Impl(const string &host, int port);

  ~Impl();

  bool Connect();

  void Close();

  void SendMessage(const ::google::protobuf::Message *message);

  void SendPacket(uint32 opcode, const ::google::protobuf::Message *message);

  void set_message_handler(MessageHandler *handler);

  void set_dispatcher(Dispatcher *dispatcher);

  int HandleRead();

  int HandleWrite();

  void ErrorMessage(const string &message);
 private:
  string host_;
  int port_;
  Buffer input_buffer_;
  Buffer output_buffer_;
  Dispatcher *dispatcher_;
  MessageChannelEvent event_;
  MessageHandler *handler_;
  MessageHeader message_header_;
  ReadMessageState state_;
};

MessageChannel::Impl::Impl(const string &host, int port)
  : host_(host),
    port_(port),
    dispatcher_(NULL),
    event_(-1, this),
    handler_(NULL),
    state_(READ_HEADER) {
}

MessageChannel::Impl::~Impl() {
  Close();
}

void MessageChannel::Impl::ErrorMessage(const string &message) {
  VLOG_ERROR() << message << "[" << host_ << ":" << port_ << "] error";
  Close();
}

bool MessageChannel::Impl::Connect() {
  event_.fd_ = NetUtility::Connect(host_, port_);
  if (event_.fd_ < 0) {
    ErrorMessage("connect to ");
    return false;
  }
  dispatcher_->AddEvent(&event_);
  return true;
}

void MessageChannel::Impl::Close() {
  if (event_.fd_ > 0) {
    VLOG_INFO() << "close connection to [" << host_ << ":" << port_ << "]";
    dispatcher_->DeleteEvent(&event_);
  }
}

void MessageChannel::Impl::SendMessage(const gpb::Message* message) {
  EncodeMessage(message, &output_buffer_);
}

void MessageChannel::Impl::SendPacket(
    uint32 opcode,
    const ::google::protobuf::Message *message) {
  EncodePacket(opcode, message, &output_buffer_);
}

void MessageChannel::Impl::set_message_handler(MessageHandler *handler) {
  handler_ = handler;
}

void MessageChannel::Impl::set_dispatcher(Dispatcher *dispatcher) {
  dispatcher_ = dispatcher;
}

int MessageChannel::Impl::HandleRead() {
  if (input_buffer_.Read(event_.fd_) == -1) {
    ErrorMessage("recv message from ");
    return kRecvMessageError;
  }
  uint32 result = ReadMessageStateMachine(&input_buffer_,
                                          &message_header_,
                                          &state_);
  if (result == kRecvMessageNotCompleted) {
    return result;
  }
  if (handler_->HandlePacket(message_header_, &input_buffer_)) {
    input_buffer_.Clear();
    return kSuccess;
  }
  ErrorMessage("handle message from ");
  return result;
}

int MessageChannel::Impl::HandleWrite() {
  if (output_buffer_.is_read_complete()) {
    return kSuccess;
  }
  uint32 result = WriteMessage(&output_buffer_, event_.fd_);
  if (result == kSendMessageError) {
    ErrorMessage("send message to ");
  }
  return result;
}

int MessageChannelEvent::HandleRead() {
  return impl_->HandleRead();
}

int MessageChannelEvent::HandleWrite() {
  return impl_->HandleWrite();
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

void MessageChannel::SendMessage(const ::google::protobuf::Message *message) {
  impl_->SendMessage(message);
}

void MessageChannel::SendPacket(
    uint32 opcode,
    const ::google::protobuf::Message *message) {
  impl_->SendPacket(opcode, message);
}

void MessageChannel::set_message_handler(MessageHandler *handler) {
  impl_->set_message_handler(handler);
}

void MessageChannel::set_dispatcher(Dispatcher *dispatcher) {
  impl_->set_dispatcher(dispatcher);
}
};
