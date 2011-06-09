/*
 * Copyright(C) lichuang
 */
#include <map>
#include <list>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "message_header.h"
#include "message_utility.h"
#include "error_code.h"
#include "base.h"
#include "event.h"
#include "message_channel.h"
#include "net_utility.h"
#include "callback.h"
#include "dispatcher.h"
#include "log.h"
namespace {
static const int kBufferLength = 100;
enum ReadState {
  READ_HEADER,
  READ_CONTENT,
};
}
namespace eventrpc {
struct MsgChannelCallback : public Callback {
  MessageChannel::Impl *impl_;
  string send_message;
  uint32 sent_count;
  MsgChannelCallback(MessageChannel::Impl *impl)
    : impl_(impl),
      send_message(""),
      sent_count(0) {
  }
  void Run();
  void Clear() {
    send_message = "";
    sent_count = 0;
  }
};

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

  void set_message_handler(MessageHandler *handler);

  void set_dispatcher(Dispatcher *dispatcher);

  int StateMachine(char *buffer, int length);

  int HandleRead();

  int HandleWrite();

  int SendPacket(MsgChannelCallback *callback);

  MsgChannelCallback *get_callback();

  void FreeCurrentReadCallback();

  string host_;
  int port_;
  Dispatcher *dispatcher_;
  MessageChannelEvent event_;
  char buffer_[kBufferLength];
  typedef std::list<MsgChannelCallback*> MsgChannelCallbackList;
  MsgChannelCallbackList send_callback_list_;
  MsgChannelCallbackList free_callback_list_;
  typedef std::map<uint32, MsgChannelCallback*> MsgChannelCallbackMap;
  MsgChannelCallback *current_read_callback_;
  std::string recv_message_;
  MessageHandler *handler_;
  MessageHeader message_header_;
  ReadState state_;
};

MessageChannel::Impl::Impl(const string &host, int port)
  : host_(host),
    port_(port),
    event_(-1, this),
    current_read_callback_(NULL),
    handler_(NULL),
    state_(READ_HEADER) {
}

MessageChannel::Impl::~Impl() {
  Close();
}

bool MessageChannel::Impl::Connect() {
  event_.fd_ = NetUtility::Connect(host_, port_);
  if (event_.fd_ < 0) {
    VLOG_ERROR() << "connect to [" << host_ << " : " << port_ << "] fail";
    return false;
  }
  dispatcher_->AddEvent(&event_);
  return true;
}

void MessageChannel::Impl::Close() {
  if (event_.fd_ > 0) {
    VLOG_INFO() << "close connection to [" << host_ << " : " << port_ << "]";
    dispatcher_->DeleteEvent(&event_);
  }
}

MsgChannelCallback *MessageChannel::Impl::get_callback() {
  if (free_callback_list_.empty()) {
    return new MsgChannelCallback(this);
  } else {
    MsgChannelCallback *callback = free_callback_list_.front();
    free_callback_list_.pop_front();
    callback->Clear();
    return callback;
  }
}

void MessageChannel::Impl::SendMessage(const gpb::Message* message) {
  MsgChannelCallback *callback = get_callback();
  ASSERT(callback != NULL);
  callback->sent_count = 0;
  EncodeMessage(message, &(callback->send_message));
  dispatcher_->PushTask(callback);
}

void MessageChannel::Impl::set_message_handler(MessageHandler *handler) {
  handler_ = handler;
}

void MessageChannel::Impl::set_dispatcher(Dispatcher *dispatcher) {
  dispatcher_ = dispatcher;
}

void MessageChannel::Impl::FreeCurrentReadCallback() {
  free_callback_list_.push_back(current_read_callback_);
  current_read_callback_ = NULL;
}

int MessageChannel::Impl::StateMachine(char *buffer, int length) {
  recv_message_.append(buffer_, length);
  while (true) {
    switch (state_) {
      case READ_HEADER:
        if (recv_message_.length() < sizeof(MessageHeader)) {
          return kRecvMessageNotCompleted;
        }
        DecodeMessageHeader(
            recv_message_.substr(0, sizeof(MessageHeader)),
            &message_header_);
        state_ = READ_CONTENT;
        recv_message_ = recv_message_.substr(sizeof(MessageHeader));
        break;
      case READ_CONTENT:
        if (recv_message_.length() < message_header_.message_length) {
          return kRecvMessageNotCompleted;
        }
        if (!handler_->HandlePacket(message_header_.opcode,
                                    &recv_message_)) {
          return kHandlePacketError;
        }
        state_ = READ_HEADER;
        return kSuccess;
        break;
    }
  }
  return kSuccess;
}

int MessageChannel::Impl::HandleRead() {
  int32 length = 0;
  uint32 result = 0;
  while (true) {
    length = 0;
    if (!NetUtility::Recv(event_.fd_, buffer_,
                          kBufferLength, &length)) {
      VLOG_ERROR() << "recv message from [" << host_ << " : " << port_ << "] error";
      Close();
      return -1;
    }
    result = StateMachine(buffer_, length);
    VLOG_INFO() << "result: " << result;
    if (result == kSuccess) {
      return kSuccess;
    }
    if (result == kRecvMessageNotCompleted && length == kBufferLength) {
      // should recv again
      continue;
    }
    if (result == kRecvMessageNotCompleted) {
      return result;
    }
    VLOG_ERROR() << "handle message from [" << host_ << " : " << port_ << "] error: " << result;
    Close();
    return result;
  }

  VLOG_FATAL() << "should not reach here";
  return kSuccess;
}

int MessageChannel::Impl::SendPacket(MsgChannelCallback *callback) {
  int length = 0;
  bool ret = false;
  while (true) {
    ret = NetUtility::Send(
        event_.fd_,
        callback->send_message.c_str() + callback->sent_count,
        callback->send_message.length() - callback->sent_count,
        &length);
    if (!ret) {
      VLOG_ERROR() << "send message to [" << host_ << " : " << port_ << "] error";
      Close();
      free_callback_list_.push_back(callback);
      return kSendMessageError;
    }
    callback->sent_count += length;
    if (callback->sent_count < callback->send_message.length()) {
      return kSendMessageNotCompleted;
    }
    VLOG_INFO() << "send service request done, count: "
      << callback->send_message.length();
    callback->sent_count = 0;
    callback->send_message = "";
    free_callback_list_.push_back(callback);
    return kSuccess;
  }

  LOG_FATAL() << "should never reach here!";
  return 0;
}

int MessageChannel::Impl::HandleWrite() {
  MsgChannelCallbackList::iterator iter, tmp_iter;
  uint32 result = 0;
  for (iter = send_callback_list_.begin();
       iter != send_callback_list_.end(); ) {
    VLOG_INFO() << "HandleWrite";
    result = SendPacket(*iter);
    if (result == kSuccess) {
      tmp_iter = iter;
      ++iter;
      send_callback_list_.erase(tmp_iter);
      continue;
    }
    if (result == kSendMessageError) {
      VLOG_ERROR() << "send service request error";
      Close();
      return result;
    }
    // when reach here, send not completed, return and
    // waiting the next send time
    return result;
  }
  return result;
}

void MsgChannelCallback::Run() {
  uint32 result = impl_->SendPacket(this);
  if (result == kSuccess) {
    return;
  }
  if (result == kSendMessageError) {
    VLOG_ERROR() << "send service request error";
    return;
  }
  impl_->send_callback_list_.push_back(this);
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

void MessageChannel::set_message_handler(MessageHandler *handler) {
  impl_->set_message_handler(handler);
}

void MessageChannel::set_dispatcher(Dispatcher *dispatcher) {
  impl_->set_dispatcher(dispatcher);
}
};
