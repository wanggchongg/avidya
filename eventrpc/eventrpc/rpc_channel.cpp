/*
 * Copyright(C) lichuang
 */
#include <map>
#include <list>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "error_code.h"
#include "base.h"
#include "event.h"
#include "meta.h"
#include "rpc_channel.h"
#include "net_utility.h"
#include "callback.h"
#include "dispatcher.h"
#include "log.h"
namespace {
static const int kBufferLength = 100;
}
namespace eventrpc {
struct RpcChannelCallback : public Callback {
  RpcChannel::Impl *impl_;
  gpb::Closure *done;
  uint32 method_id;
  string send_message;
  uint32 sent_count;
  gpb::Message *response;
  Meta recv_meta;
  RpcChannelCallback(RpcChannel::Impl *impl)
    : impl_(impl),
      done(NULL),
      method_id(0),
      send_message(""),
      sent_count(0),
      response(NULL) {
  }
  void Run();
  void Clear() {
    done = NULL;
    method_id = 0;
    send_message = "";
    sent_count = 0;
    response = NULL;
  }
};

struct RpcChannelEvent : public Event {
  RpcChannelEvent(int fd, RpcChannel::Impl *impl)
    : Event(fd, EVENT_WRITE | EVENT_READ),
      impl_(impl) {
  }

  virtual ~RpcChannelEvent() {
  }

  int HandleRead();

  int HandleWrite();

  RpcChannel::Impl *impl_;
};

struct RpcChannel::Impl {
 public:
  Impl(const char *host, int port, Dispatcher *dispatcher);

  ~Impl();

  bool Connect();

  void Close();

  void CallMethod(const gpb::MethodDescriptor* method,
                  gpb::RpcController* controller,
                  const gpb::Message* request,
                  gpb::Message* response,
                  gpb::Closure* done);

  int HandleRead();

  int HandleWrite();

  int SendServiceRequest(RpcChannelCallback *callback);

  int DecodeBuffer(ssize_t recv_count);

  RpcChannelCallback *get_callback();

  void FreeCurrentReadCallback();

  const char *host_;
  int port_;
  Dispatcher *dispatcher_;
  RpcChannelEvent event_;
  char buffer_[kBufferLength];
  typedef std::list<RpcChannelCallback*> RpcChannelCallbackList;
  RpcChannelCallbackList send_callback_list_;
  RpcChannelCallbackList free_callback_list_;
  typedef std::map<uint32, RpcChannelCallback*> RpcChannelCallbackMap;
  RpcChannelCallbackMap callback_map_;
  RpcChannelCallback *current_read_callback_;
  std::string recv_message_;
};

RpcChannel::Impl::Impl(const char *host, int port, Dispatcher *dispatcher)
  : host_(host),
    port_(port),
    dispatcher_(dispatcher),
    event_(-1, this),
    current_read_callback_(NULL) {
}

RpcChannel::Impl::~Impl() {
  Close();
}

bool RpcChannel::Impl::Connect() {
  event_.fd_ = NetUtility::Connect(host_, port_);
  if (event_.fd_ < 0) {
    VLOG_ERROR() << "connect to [" << host_ << " : " << port_ << "] fail";
    return false;
  }
  dispatcher_->AddEvent(&event_);
  return true;
}

void RpcChannel::Impl::Close() {
  if (event_.fd_ > 0) {
    VLOG_INFO() << "close connection to [" << host_ << " : " << port_ << "]";
    dispatcher_->DeleteEvent(&event_);
  }
}

RpcChannelCallback *RpcChannel::Impl::get_callback() {
  if (free_callback_list_.empty()) {
    return new RpcChannelCallback(this);
  } else {
    RpcChannelCallback *callback = free_callback_list_.front();
    free_callback_list_.pop_front();
    callback->Clear();
    return callback;
  }
}

void RpcChannel::Impl::CallMethod(const gpb::MethodDescriptor* method,
                                  gpb::RpcController* controller,
                                  const gpb::Message* request,
                                  gpb::Message* response,
                                  gpb::Closure* done) {
  Meta meta;
  RpcChannelCallback *callback = get_callback();
  ASSERT(callback != NULL);
  callback->sent_count = 0;
  callback->method_id = meta.method_id();
  callback->response = response;
  callback->done = done;
  static uint32 request_id = 1;
  meta.set_request_id(request_id);
  ++request_id;
  meta.EncodeWithMessage(method->full_name(),
                         request,
                         &(callback->send_message));
  VLOG_INFO() << "call method send request "
    << method->full_name()
    << ", method id: " << meta.method_id()
    << ", message length: " << meta.message_length()
    << ", request id: " << meta.request_id();
  callback_map_[meta.request_id()] = callback;
  dispatcher_->PushTask(callback);
}

int RpcChannel::Impl::DecodeBuffer(int length) {
  ASSERT(strlen(buffer_) != 0);
  RpcChannelCallback *callback = NULL;
  Meta meta;
  recv_message_.append(buffer_, length);
  if (current_read_callback_ == NULL) {
    if (recv_message_.length() < META_LEN) {
      return kRecvMessageNotCompleted;
    }
    meta.EncodeWithBuffer(recv_message_.c_str());
    RpcChannelCallbackMap::iterator iter;
    iter = callback_map_.find(meta.request_id());
    if (iter == callback_map_.end()) {
      VLOG_INFO() << "cannot find request id: " << meta.request_id();
      return kCannotFindRequestId;
    }
    callback = iter->second;
    callback->recv_meta = meta;
    current_read_callback_ = callback;
  }
  if (current_read_callback_->recv_meta.message_length() !=
      (recv_message_.length() - META_LEN)) {
    return kRecvMessageNotCompleted;
  }
  if (!current_read_callback_->response->ParseFromString(
      recv_message_.substr(META_LEN))) {
    FreeCurrentReadCallback();
    return kDecodeMessageError;
  }
  if (current_read_callback_->done != NULL) {
    current_read_callback_->done->Run();
  }
  FreeCurrentReadCallback();
  recv_message_ = "";
  return kSuccess;
}

void RpcChannel::Impl::FreeCurrentReadCallback() {
  callback_map_.erase(current_read_callback_->recv_meta.request_id());
  free_callback_list_.push_back(current_read_callback_);
  current_read_callback_ = NULL;
}

int RpcChannel::Impl::HandleRead() {
  VLOG_INFO() << "HandleRead";
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
    result = DecodeBuffer(length);
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

int RpcChannel::Impl::SendServiceRequest(RpcChannelCallback *callback) {
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

int RpcChannel::Impl::HandleWrite() {
  RpcChannelCallbackList::iterator iter, tmp_iter;
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

void RpcChannelCallback::Run() {
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

int RpcChannelEvent::HandleRead() {
  return impl_->HandleRead();
}

int RpcChannelEvent::HandleWrite() {
  return impl_->HandleWrite();
}

RpcChannel::RpcChannel(const char *host, int port, Dispatcher *dispatcher) {
   impl_ = new Impl(host, port, dispatcher);
}

RpcChannel::~RpcChannel() {
  delete impl_;
}

bool RpcChannel::Connect() {
  return impl_->Connect();
}

void RpcChannel::Close() {
  impl_->Close();
}

void RpcChannel::CallMethod(const gpb::MethodDescriptor* method,
                            gpb::RpcController* controller,
                            const gpb::Message* request,
                            gpb::Message* response,
                            gpb::Closure* done) {
  impl_->CallMethod(method, controller, request, response, done);
}
};
