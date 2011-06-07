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
#include "meta.h"
#include "message_channel.h"
#include "net_utility.h"
#include "callback.h"
#include "dispatcher.h"
#include "log.h"
namespace {
static const int kBufferLength = 100;
}
namespace eventrpc {
struct RpcMsgChannelCallback : public Callback {
  RpcMsgChannel::Impl *impl_;
  uint32 msg_id;
  string send_message;
  uint32 sent_count;
  Meta recv_meta;
  RpcMsgChannelCallback(RpcMsgChannel::Impl *impl)
    : impl_(impl),
      msg_id(0),
      send_message(""),
      sent_count(0) {
  }
  void Run();
  void Clear() {
    msg_id = 0;
    send_message = "";
    sent_count = 0;
  }
};

struct RpcMsgChannelEvent : public Event {
  RpcMsgChannelEvent(int fd, RpcMsgChannel::Impl *impl)
    : Event(fd, EVENT_WRITE | EVENT_READ),
      impl_(impl) {
  }

  virtual ~RpcMsgChannelEvent() {
  }

  int HandleRead();

  int HandleWrite();

  RpcMsgChannel::Impl *impl_;
};

struct RpcMsgChannel::Impl {
 public:
  Impl(const string &host, int port, Dispatcher *dispatcher);

  ~Impl();

  bool Connect();

  void Close();

  void SendMessage(const ::google::protobuf::Message *message);

  int HandleRead();

  int HandleWrite();

  int SendServiceRequest(RpcMsgChannelCallback *callback);

  RpcMsgChannelCallback *get_callback();

  void FreeCurrentReadCallback();

  string host_;
  int port_;
  Dispatcher *dispatcher_;
  RpcMsgChannelEvent event_;
  char buffer_[kBufferLength];
  typedef std::list<RpcMsgChannelCallback*> RpcMsgChannelCallbackList;
  RpcMsgChannelCallbackList send_callback_list_;
  RpcMsgChannelCallbackList free_callback_list_;
  typedef std::map<uint32, RpcMsgChannelCallback*> RpcMsgChannelCallbackMap;
  RpcMsgChannelCallback *current_read_callback_;
  std::string recv_message_;
};

RpcMsgChannel::Impl::Impl(const string &host, int port, Dispatcher *dispatcher)
  : host_(host),
    port_(port),
    dispatcher_(dispatcher),
    event_(-1, this),
    current_read_callback_(NULL) {
}

RpcMsgChannel::Impl::~Impl() {
  Close();
}

bool RpcMsgChannel::Impl::Connect() {
  event_.fd_ = NetUtility::Connect(host_, port_);
  if (event_.fd_ < 0) {
    VLOG_ERROR() << "connect to [" << host_ << " : " << port_ << "] fail";
    return false;
  }
  dispatcher_->AddEvent(&event_);
  return true;
}

void RpcMsgChannel::Impl::Close() {
  if (event_.fd_ > 0) {
    VLOG_INFO() << "close connection to [" << host_ << " : " << port_ << "]";
    dispatcher_->DeleteEvent(&event_);
  }
}

RpcMsgChannelCallback *RpcMsgChannel::Impl::get_callback() {
  if (free_callback_list_.empty()) {
    return new RpcMsgChannelCallback(this);
  } else {
    RpcMsgChannelCallback *callback = free_callback_list_.front();
    free_callback_list_.pop_front();
    callback->Clear();
    return callback;
  }
}

void RpcMsgChannel::Impl::SendMessage(const gpb::Message* message) {
  Meta meta;
  RpcMsgChannelCallback *callback = get_callback();
  ASSERT(callback != NULL);
  callback->sent_count = 0;
  callback->msg_id = meta.method_id();
  meta.EncodeWithMessage(message->GetTypeName(),
                         message,
                         &(callback->send_message));
  dispatcher_->PushTask(callback);
}

void RpcMsgChannel::Impl::FreeCurrentReadCallback() {
  free_callback_list_.push_back(current_read_callback_);
  current_read_callback_ = NULL;
}

// no read here
int RpcMsgChannel::Impl::HandleRead() {
  return kSuccess;
}

int RpcMsgChannel::Impl::SendServiceRequest(RpcMsgChannelCallback *callback) {
  // is no need response?
  if (callback->send_message.empty()) {
    free_callback_list_.push_back(callback);
    return kSuccess;
  }
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

int RpcMsgChannel::Impl::HandleWrite() {
  RpcMsgChannelCallbackList::iterator iter, tmp_iter;
  uint32 result = 0;
  for (iter = send_callback_list_.begin();
       iter != send_callback_list_.end(); ) {
    VLOG_INFO() << "HandleWrite";
    result = SendServiceRequest(*iter);
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

void RpcMsgChannelCallback::Run() {
  uint32 result = impl_->SendServiceRequest(this);
  if (result == kSuccess) {
    return;
  }
  if (result == kSendMessageError) {
    VLOG_ERROR() << "send service request error";
    return;
  }
  impl_->send_callback_list_.push_back(this);
}

int RpcMsgChannelEvent::HandleRead() {
  return impl_->HandleRead();
}

int RpcMsgChannelEvent::HandleWrite() {
  return impl_->HandleWrite();
}

RpcMsgChannel::RpcMsgChannel(const string &host, int port, Dispatcher *dispatcher) {
   impl_ = new Impl(host, port, dispatcher);
}

RpcMsgChannel::~RpcMsgChannel() {
  delete impl_;
}

bool RpcMsgChannel::Connect() {
  return impl_->Connect();
}

void RpcMsgChannel::Close() {
  impl_->Close();
}

void RpcMsgChannel::SendMessage(const ::google::protobuf::Message *message) {
  impl_->SendMessage(message);
}
};
