#include <map>
#include <unistd.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "rpcclientevent.h"
#include "socket_utility.h"
#include "eventpoller.h"
#include "utility.h"
#include "meta.h"

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
       EventPoller *event_poller);

  ~Impl();

  int OnRead();

  int OnWrite();

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
  EventPoller *&event_poller_;
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
                           EventPoller *event_poller)
  : client_event_(client_event)
    , event_poller_(event_poller)
    , state_(INIT)
    , count_(0) {
      client_event_->fd_ = Connect(ip, port);
}

RpcClientEvent::Impl::~Impl() {
}

int RpcClientEvent::Impl::OnRead() {
  if (state_ < SEND_REQUEST) {
    return -1;
  } else if (state_ == SEND_REQUEST) {
    state_ = READ_META;
    count_ = META_LEN;
  }

  int len, ret;
  ssize_t recv_count;
  while (true) {
    recv_count = count_ > BUFFER_LENGTH ? BUFFER_LENGTH : count_;
    ret = Recv(client_event_->fd_, buf_, recv_count, &len);
    if (ret < 0) {
      Close();
      return -1;
    } else if (len < recv_count) {
      count_ -= len;
      message_.append(buf_, len);
      return 0;
    } else if (len == recv_count) {
      if (state_ == READ_META) {
          meta_.Encode(message_.c_str());
          count_ = meta_.message_len();
          state_ = READ_MESSAGE;
          if (meta_.method_id() != request_info_.method_id_) {
            Close();
            return -1;
          }
          message_ = "";
      } else if (state_ == READ_MESSAGE) {
          message_.append(buf_, len);
          request_info_.response_->ParseFromString(message_);
          request_info_.done_->Run();
          // whether close depends on user
          // Close();
          return 0;
      }
    }
  }

  return -1;
}

int RpcClientEvent::Impl::OnWrite() {
  int len, ret;
  while (true) {
    ret = Send(client_event_->fd_,
               message_.c_str() + sent_count_, count_, &len);
    if (ret < 0) {
      Close();
      return -1;
    } else if (len < count_) {
      count_ -= len;
      sent_count_ += len;
      if (!(client_event_->event_ & WRITE_EVENT)) {
        client_event_->event_ |= WRITE_EVENT;
        event_poller_->AddEvent(WRITE_EVENT, client_event_);
      }
      return 0;
    } else if (len == count_) {
      client_event_->event_ &= ~WRITE_EVENT;
      event_poller_->DelEvent(WRITE_EVENT, client_event_);
      return 0;
    }
  }

  return 0;
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
  OnWrite();
}

RpcClientEvent::RpcClientEvent(const char* ip, int port)
  : Event(READ_EVENT)
  , impl_(new Impl(ip, port, this, event_poller_)) {
}

RpcClientEvent::~RpcClientEvent() {
  delete impl_;
}

int RpcClientEvent::OnWrite() {
  return impl_->OnWrite();
}

int RpcClientEvent::OnRead() {
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
