#include <map>
#include <unistd.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "net/rpcclientevent.h"
#include "net/socket_utility.h"
#include "net/dispatcher.h"
#include "util/utility.h"
#include "net/meta.h"

EVENTRPC_NAMESPACE_BEGIN

struct RequestInfo {
 public:
  RequestInfo()
    : controller_(NULL)
    , response_(NULL)
    , done_(NULL){
  }

  void Init(gpb::RpcController *controller,
            gpb::Message *response,
            uint32_t method_id,
            gpb::Closure *done) {
    controller_ = controller;
    response_ = response;
    method_id_ = method_id;
    done_ = done;
  }

  gpb::RpcController *controller_;
  gpb::Message *response_;
  uint32_t method_id_;
  gpb::Closure *done_;
};

struct RpcClientEvent::Impl {
 public:
  Impl(const char *ip, int port,
       RpcClientEvent *client_event,
       Dispatcher *dispatcher);

  ~Impl();

  bool OnRead();

  bool OnWrite();

  void CallMethod(const gpb::MethodDescriptor* method,
                  gpb::RpcController* controller,
                  const gpb::Message* request,
                  gpb::Message* response,
                  gpb::Closure* done);

  bool CallMethodDone(int callback_index);

 private:
  void Close() {
    client_event_->Close();
  }

 private:
  RpcClientEvent *client_event_;
  Dispatcher *&dispatcher_;
  RequestInfo request_info_;
  Request_State state_;
  char buf_[BUFFER_LENGTH];
  ssize_t count_;
  ssize_t sent_count_;
  Meta meta_;
  string message_;
};

RpcClientEvent::Impl::Impl(const char *ip, int port,
                           RpcClientEvent *client_event,
                           Dispatcher *dispatcher)
  : client_event_(client_event)
    , dispatcher_(dispatcher)
    , state_(INIT)
    , count_(0) {
      client_event_->fd_ = Connect(ip, port);
}

RpcClientEvent::Impl::~Impl() {
}

bool RpcClientEvent::Impl::OnRead() {
  if (state_ < SEND_REQUEST) {
    return false;
  } else if (state_ == SEND_REQUEST) {
    state_ = READ_META;
    count_ = META_LEN;
  }

  int len;
  bool ret;
  ssize_t recv_count;
  while (true) {
    recv_count = count_ > BUFFER_LENGTH ? BUFFER_LENGTH : count_;
    ret = Recv(client_event_->fd_, buf_, recv_count, &len);
    if (!ret) {
      Close();
      return false;
    } else if (len < recv_count) {
      count_ -= len;
      message_.append(buf_, len);
      return true;
    } else if (len == recv_count) {
      if (state_ == READ_META) {
          meta_.Encode(message_.c_str());
          count_ = meta_.message_len();
          state_ = READ_MESSAGE;
          if (meta_.method_id() != request_info_.method_id_) {
            Close();
            return false;
          }
          message_ = "";
      } else if (state_ == READ_MESSAGE) {
          message_.append(buf_, len);
          request_info_.response_->ParseFromString(message_);
          request_info_.done_->Run();
          // whether close depends on user
          // Close();
          return true;
      }
    }
  }

  return false;
}

bool RpcClientEvent::Impl::OnWrite() {
  int len;
  bool ret;
  while (true) {
    ret = Send(client_event_->fd_,
               message_.c_str() + sent_count_, count_, &len);
    if (!ret) {
      Close();
      return false;
    } else if (len < count_) {
      count_ -= len;
      sent_count_ += len;
      return true;
    } else if (len == count_) {
      if (!client_event_->UpdateEvent(READ_EVENT)) {
        return false;
      }
      return true;
    }
  }

  return true;
}

void RpcClientEvent::Impl::CallMethod(const gpb::MethodDescriptor *method,
                                      gpb::RpcController *controller,
                                      const gpb::Message *request,
                                      gpb::Message *response,
                                      gpb::Closure *done) {
  meta_.EncodeWithMessage(method->full_name(), request, &message_);
  request_info_.Init(controller, response, meta_.method_id(), done);

  state_ = SEND_REQUEST;
  count_ = message_.length();
  sent_count_ = 0;
  if (!client_event_->UpdateEvent(WRITE_EVENT)) {
    return;
  }
  OnWrite();
}

RpcClientEvent::RpcClientEvent(const char* ip, int port)
  : impl_(new Impl(ip, port, this, dispatcher_)) {
}

RpcClientEvent::~RpcClientEvent() {
  delete impl_;
}

bool RpcClientEvent::OnWrite() {
  return impl_->OnWrite();
}

bool RpcClientEvent::OnRead() {
  return impl_->OnRead();
}

void RpcClientEvent::CallMethod(const gpb::MethodDescriptor *method,
                                gpb::RpcController *controller,
                                const gpb::Message *request,
                                gpb::Message *response,
                                gpb::Closure *done) {
  impl_->CallMethod(method, controller, request, response, done);
}

EVENTRPC_NAMESPACE_END
