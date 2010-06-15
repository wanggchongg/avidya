#ifndef __EVENTRPC_CONNECTIONEVENT_H__
#define __EVENTRPC_CONNECTIONEVENT_H__

#include <map>
#include <google/protobuf/service.h>
#include "meta.h"
#include "eventi.h"
#include "base.h"

using std::map;

EVENTRPC_NAMESPACE_BEGIN

class RpcServerEvent;

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

struct ConnectionEvent : public Event {
 public:
  ConnectionEvent(int fd, const RpcMethodMap &rpc_methods,
                  RpcServerEvent *server_event, EventPoller *event_poller);

  virtual ~ConnectionEvent();

  virtual int OnWrite();

  virtual int OnRead();

  void Init();

 private:
  struct Impl;
  Impl *impl_;
};

EVENTRPC_NAMESPACE_END

#endif  //  __EVENTRPC_CONNECTIONEVENT_H__
