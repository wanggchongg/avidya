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
#include "dispatcher.h"
#include "log.h"
namespace {
static const int kBufferLength = 100;
}
namespace eventrpc {
struct ServiceInfo {
  gpb::Closure *done;
  uint32 method_id;
  string send_message;
  ssize_t sent_count;
  gpb::Message *response;
  Meta recv_meta;
  ServiceInfo()
    : done(NULL),
      method_id(0),
      send_message(""),
      sent_count(0),
      response(NULL) {
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

  int SendServiceRequest(ServiceInfo *service_info);

  int DecodeBuffer(ssize_t recv_count);

  RpcChannelEvent event_;
  const char *host_;
  int port_;
  Dispatcher *dispatcher_;
  char buffer_[kBufferLength];
  std::map<uint32, ServiceInfo*> service_info_map_;
  std::list<uint32> send_method_id_list_;
  ServiceInfo *current_service_info_;
  std::string recv_message_;
};

RpcChannel::Impl::Impl(const char *host, int port, Dispatcher *dispatcher)
  : host_(host),
    port_(port),
    dispatcher_(dispatcher),
    event_(-1, this),
    current_service_info_(NULL) {
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

void RpcChannel::Impl::CallMethod(const gpb::MethodDescriptor* method,
                                  gpb::RpcController* controller,
                                  const gpb::Message* request,
                                  gpb::Message* response,
                                  gpb::Closure* done) {
  Meta meta;
  meta.set_method_id(method->full_name());
  VLOG_INFO() << "call method send request "
    << method->full_name()
    << ", method id: " << meta.method_id();
  ServiceInfo *service_info = NULL;
  std::map<uint32, ServiceInfo*>::iterator iter;
  iter = service_info_map_.find(meta.method_id());
  if (iter != service_info_map_.end()) {
    service_info = iter->second;
  } else {
    service_info = new ServiceInfo();
    service_info_map_[meta.method_id()] = service_info;
  }
  service_info->sent_count = 0;
  service_info->method_id = meta.method_id();
  service_info->response = response;
  service_info->done = done;
  meta.EncodeWithMessage(method->full_name(),
                         request,
                         &(service_info->send_message));
  send_method_id_list_.push_back(meta.method_id());
}

int RpcChannel::Impl::DecodeBuffer(int length) {
  ASSERT(strlen(buffer_) != 0);
  ServiceInfo *service_info = NULL;
  Meta meta;
  recv_message_.append(buffer_, length);
  if (current_service_info_ == NULL) {
    if (recv_message_.length() < META_LEN) {
      return kRecvMessageNotCompleted;
    }
    meta.Encode(recv_message_.c_str());
    std::map<uint32, ServiceInfo*>::iterator iter;
    iter = service_info_map_.find(meta.method_id());
    if (iter == service_info_map_.end()) {
      return kCannotFindMethodId;
    }
    service_info = iter->second;
    service_info->recv_meta = meta;
  }
  if (service_info->recv_meta.message_length() !=
      (recv_message_.length() - META_LEN)) {
    return kRecvMessageNotCompleted;
  }
  if (!service_info->response->ParseFromString(
      recv_message_.substr(META_LEN))) {
    return kDecodeMessageError;
  }
  if (service_info->done != NULL) {
    service_info->done->Run();
  }
  return kSuccess;
}

int RpcChannel::Impl::HandleRead() {
  int32 length = 0;
  int32 ret = 0;
  while (true) {
    length = 0;
    if (!NetUtility::Recv(event_.fd_, buffer_,
                          kBufferLength, &length)) {
      Close();
      return -1;
    }
    ret = DecodeBuffer(length);
    if (ret == kSuccess) {
      return kSuccess;
    }
    if (ret == kRecvMessageNotCompleted && length == kBufferLength) {
      // should recv again
      continue;
    }
    Close();
    return ret;
  }

  return ret;
}

// return -1 means send error and the socket has been closed,
// return 0 means send data but not completed yet,
// return 1 means send service request done
int RpcChannel::Impl::SendServiceRequest(ServiceInfo *service_info) {
  int length = 0;
  bool ret = false;
  while (true) {
    ret = NetUtility::Send(
        event_.fd_,
        service_info->send_message.c_str() + service_info->sent_count,
        service_info->send_message.length() - service_info->sent_count,
        &length);
    if (!ret) {
      Close();
      return -1;
    } else if (length < service_info->send_message.length()) {
      service_info->sent_count += length;
      return 0;
    } else if (length == service_info->send_message.length()) {
      VLOG_INFO() << "send service request done, count: "
        << service_info->send_message.length();
      service_info->sent_count = 0;
      service_info->send_message = "";
      return 1;
    }
  }

  LOG_FATAL() << "should never reach here!";
  return 0;
}

int RpcChannel::Impl::HandleWrite() {
  std::list<uint32>::iterator iter, tmp_iter;
  std::map<uint32, ServiceInfo*>::iterator service_map_iter;
  int ret = 0;
  for (iter = send_method_id_list_.begin();
       iter != send_method_id_list_.end(); ) {
    service_map_iter = service_info_map_.find(*iter);
    ASSERT(service_map_iter != service_info_map_.end());
    ret = SendServiceRequest(service_map_iter->second);
    if (ret == 1) {
      tmp_iter = iter;
      ++iter;
      send_method_id_list_.erase(tmp_iter);
      continue;
    }
    if (ret == 0) {
      return 0;
    }
    if (ret == -1) {
      VLOG_ERROR() << "send service request error";
      return -1;
    }
  }
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
