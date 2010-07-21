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
#include "net/rpcserverevent.h"
#include "util/utility.h"
#include "net/connectionevent.h"
#include "net/eventpoller.h"
#include "net/socket_utility.h"
#include "concurrency/workerthread.h"

using std::string;
using std::vector;

EVENTRPC_NAMESPACE_BEGIN

struct RpcServerEvent::Impl {
  friend class RpcServerEvent;

 public:
  Impl(const char *ip, int port, RpcServerEvent *serverevent);

  ~Impl();

  bool OnWrite();

  bool OnRead();

  bool RegisterService(gpb::Service *service);

 private:
  RpcServerEvent *server_event_;
  RpcMethodMap rpc_methods_;
  vector<WorkerThread*> worker_thread_vec_;
  int thread_count_;
};

RpcServerEvent::Impl::Impl(const char *ip, int port,
                           RpcServerEvent *server_event)
  : server_event_(server_event),
    thread_count_(0) {
    server_event_->fd_ = Listen(ip, port);
    int count = GetCpuNum();
    for (; count > 0; --count) {
      WorkerThread *worker_thread = new WorkerThread(server_event);
      worker_thread->Start(count - 1);
      worker_thread_vec_.push_back(worker_thread);
    }
}

RpcServerEvent::Impl::~Impl() {
  while (!worker_thread_vec_.empty()) {
    WorkerThread *worker_thread = worker_thread_vec_.back();
    worker_thread_vec_.pop_back();
    delete worker_thread;
  }
}

bool RpcServerEvent::Impl::OnWrite() {
  return false;
}

bool RpcServerEvent::Impl::OnRead() {
  int fd;
  struct sockaddr_in addr;
  socklen_t len = sizeof(addr);

  fd = ::accept(server_event_->fd_, (struct sockaddr *)&addr, &len);
  if (fd == -1) {
    return false;
  }

  thread_count_ = (thread_count_ + 1) % worker_thread_vec_.size();
  worker_thread_vec_[thread_count_]->PushNewConnection(fd);

  return true;
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
  : impl_(new Impl(ip, port, this)) {
  event_flags_ = READ_EVENT;
}

RpcServerEvent::~RpcServerEvent() {
  delete impl_;
}

bool RpcServerEvent::OnRead() {
  return impl_->OnRead();
}

bool RpcServerEvent::OnWrite() {
  return impl_->OnWrite();
}

bool RpcServerEvent::RegisterService(gpb::Service* service) {
  return impl_->RegisterService(service);
}

RpcMethodMap* RpcServerEvent::rpc_methods() {
  return &(impl_->rpc_methods_);
}

EVENTRPC_NAMESPACE_END
