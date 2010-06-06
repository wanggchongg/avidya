#include <string>
#include <vector>
#include <map>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/stubs/common.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "utility.h"
#include "meta.h"
#include "rpcserverevent.h"
#include "eventpoller.h"
#include "socket_utility.h"

using std::string;
using std::vector;
using std::map;

EVENTRPC_NAMESPACE_BEGIN

struct RpcMethod {
 public:
  RpcMethod(gpb::Service *service,
            const gpb::Message *request,
            const gpb::Message *response,
            const gpb::MethodDescriptor *method)
    : service_(service)
      , request_(request)
      , response_(response)
      , method_(method) {
      }

  gpb::Service *service_;
  const gpb::Message *request_;
  const gpb::Message *response_;
  const gpb::MethodDescriptor *method_;
};

typedef map<uint32_t, RpcMethod*> RpcMethodMap;

struct ClientEvent : public Event {
 public:
  ClientEvent(int fd, const RpcMethodMap &rpc_methods,
              RpcServerEvent *server_event)
    : Event(READ_EVENT, fd)
      , rpc_methods_(rpc_methods)
      , server_event_(server_event)
      , state_(INIT) {
      }

  virtual int OnWrite();

  virtual int OnRead();

  void HandleServiceDone();

  void Init();

 private:
  char buf_[BUFFER_LENGTH];
  RpcMethodMap rpc_methods_;
  Meta meta_;
  RpcServerEvent *server_event_;
  gpb::Message *request_;
  gpb::Message *response_;
  const gpb::MethodDescriptor *method_;
  RpcMethod *rpc_method_;
  string message_;
  Request_State state_;
  ssize_t count_;
  ssize_t sent_count_;
};

int ClientEvent::OnWrite() {
  int len, ret;
  while (true) {
    ret = Send(fd_, message_.c_str() + sent_count_, count_, &len);
    if (ret < 0) {
      Close();
      return -1;
    } else if (len < count_) {
      count_ -= len;
      sent_count_ += len;
      if (!(event_ & WRITE_EVENT)) {
        event_ |= WRITE_EVENT;
        event_poller_->AddEvent(WRITE_EVENT, this);
      }
      return 0;
    } else if (len == count_) {
      Close();
      return 0;
    }
  }

  return 0;
}

int ClientEvent::OnRead() {
  int len, ret;
  ssize_t recv_count;
  while (true) {
    recv_count = count_ > BUFFER_LENGTH ? BUFFER_LENGTH : count_;
    ret = Recv(fd_, buf_, recv_count, &len);
    if (ret < 0) {
      Close();
      return -1;
    } else if (len < recv_count) {
      count_ -= len;
      message_.append(buf_, len);
      return 0;
    } else if (len == recv_count) {
      message_.append(buf_, len);
      if (state_ == READ_META) {
        meta_.Encode(message_.c_str());
        state_ = READ_MESSAGE;
        RpcMethodMap::iterator iter;
        if ((iter = rpc_methods_.find(meta_.method_id()))
            != rpc_methods_.end()) {
          rpc_method_ = iter->second;
          count_ = meta_.message_len();
          state_ = READ_MESSAGE;
          message_ = "";
        } else {
          Close();
          return -1;
        }
      } else if (state_ == READ_MESSAGE) {
        method_ = rpc_method_->method_;;
        request_ = rpc_method_->request_->New();
        response_ = rpc_method_->response_->New();
        request_->ParseFromString(message_);
        gpb::Closure *done = gpb::NewCallback(
            this,
            &ClientEvent::HandleServiceDone);
        rpc_method_->service_->CallMethod(method_,
                                          NULL,
                                          request_, response_, done);
        return 0;
      }
    }
  }

  return -1;
}

void ClientEvent::HandleServiceDone() {
  message_ = "";
  meta_.EncodeWithMessage(method_->full_name(), response_, &message_);
  sent_count_ = 0;
  count_ = message_.length();
  delete request_;
  delete response_;
  OnWrite();
}

void ClientEvent::Init() {
  state_ = READ_META;
  count_ = META_LEN;
}

struct RpcServerEvent::Impl {
 public:
  Impl(const char *ip, int port, RpcServerEvent *serverevent);

  ~Impl();

  int OnWrite();

  int OnRead();

  bool RegisterService(gpb::Service *service);

 private:
  RpcServerEvent *server_event_;
  vector<ClientEvent*> client_events_;
  RpcMethodMap rpc_methods_;
};

RpcServerEvent::Impl::Impl(const char *ip, int port,
                           RpcServerEvent *server_event)
: server_event_(server_event) {
  server_event_->fd_ = Listen(ip, port);
}

RpcServerEvent::Impl::~Impl() {
}

int RpcServerEvent::Impl::OnWrite() {
  return -1;
}

int RpcServerEvent::Impl::OnRead() {
  int fd;
  struct sockaddr_in addr;
  socklen_t len = sizeof(addr);

  fd = accept(server_event_->fd_, (struct sockaddr *)&addr, &len);
  if (fd == -1) {
    return -1;
  }

  if (SetNonBlocking(fd) < 0) {
    return -1;
  }

  ClientEvent *client_event = new ClientEvent(
      fd, rpc_methods_, server_event_);
  client_event->Init();
  server_event_->event_poller()->AddEvent(READ_EVENT, client_event);
  client_events_.push_back(client_event);

  return 0;
}

bool RpcServerEvent::Impl::RegisterService(gpb::Service *service) {
  const gpb::ServiceDescriptor *descriptor = service->GetDescriptor();
  for (int i = 0; i < descriptor->method_count(); ++i) {
    const gpb::MethodDescriptor *method = descriptor->method(i);
    const google::protobuf::Message *request =
      &service->GetRequestPrototype(method);
    const google::protobuf::Message *response =
      &service->GetResponsePrototype(method);
    RpcMethod *rpc_method = new RpcMethod(service, request,
                                          response, method);

    uint32_t method_id = hash_string(method->full_name());
    rpc_methods_[method_id] = rpc_method;
  }

  return true;
}

RpcServerEvent::RpcServerEvent(const char *ip, int port)
  : Event(READ_EVENT)
  , impl_(new Impl(ip, port, this)) {
}

RpcServerEvent::~RpcServerEvent() {
  delete impl_;
}

int RpcServerEvent::OnRead() {
  return impl_->OnRead();
}

int RpcServerEvent::OnWrite() {
  return impl_->OnWrite();
}

bool RpcServerEvent::RegisterService(gpb::Service* service) {
  return impl_->RegisterService(service);
}

EVENTRPC_NAMESPACE_END
