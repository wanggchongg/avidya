#ifndef __EVENTRPC_CONNECTIONEVENT_H__
#define __EVENTRPC_CONNECTIONEVENT_H__

#include <map>
#include <google/protobuf/service.h>
#include "net/meta.h"
#include "net/eventi.h"
#include "base/base.h"
#include "concurrency/workerthread.h"

using std::map;

EVENTRPC_NAMESPACE_BEGIN

class RpcServerEvent;
class WorkerThread;

struct RpcMethod {
 public:
  RpcMethod(gpb::Service *service,
            const gpb::Message *request,
            const gpb::Message *response,
            const gpb::MethodDescriptor *method)
    : service_(service),
      request_(request),
      response_(response),
      method_(method) {
  }

  gpb::Service *service_;
  const gpb::Message *request_;
  const gpb::Message *response_;
  const gpb::MethodDescriptor *method_;
};

typedef map<uint32_t, RpcMethod*> RpcMethodMap;

struct ConnectionEvent : public Event {
 public:
  ConnectionEvent(int fd, RpcServerEvent *server_event,
                  Dispatcher *dispatcher);

  virtual ~ConnectionEvent();

  virtual bool OnWrite();

  virtual bool OnRead();

  void Close();

  void Init(int fd, WorkerThread *worker_thread);

 private:
  struct Impl;
  Impl *impl_;
};

EVENTRPC_NAMESPACE_END

#endif  //  __EVENTRPC_CONNECTIONEVENT_H__
