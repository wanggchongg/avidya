#include <string>
#include <vector>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/stubs/common.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "utility.h"
#include "rpcserverevent.h"
#include "connectionevent.h"
#include "eventpoller.h"
#include "socket_utility.h"
#include "workerthread.h"

using std::string;
using std::vector;

EVENTRPC_NAMESPACE_BEGIN

struct RpcServerEvent::Impl {
  friend class RpcServerEvent;

 public:
  Impl(const char *ip, int port, RpcServerEvent *serverevent);

  ~Impl();

  int OnWrite();

  int OnRead();

  bool RegisterService(gpb::Service *service);

 private:
  RpcServerEvent *server_event_;
  RpcMethodMap rpc_methods_;
  WorkerThread worker_thread_;
};

RpcServerEvent::Impl::Impl(const char *ip, int port,
                           RpcServerEvent *server_event)
  : server_event_(server_event)
  , worker_thread_(server_event){
    server_event_->fd_ = Listen(ip, port);
    worker_thread_.Start();
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

  worker_thread_.PushNewConnection(fd);

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

RpcMethodMap* RpcServerEvent::rpc_methods() {
  return &(impl_->rpc_methods_);
}

EVENTRPC_NAMESPACE_END
