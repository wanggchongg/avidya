#ifndef __EVENTRPC_RPCCLIENTEVENT_H__
#define __EVENTRPC_RPCCLIENTEVENT_H__

#include <google/protobuf/service.h>
#include "base.h"
#include "eventi.h"

EVENTRPC_NAMESPACE_BEGIN

class RpcClientEvent
  : public Event
  , public gpb::RpcChannel {
 public:
  RpcClientEvent(const char *ip, int port);

  virtual ~RpcClientEvent();

  virtual int OnWrite();

  virtual int OnRead();

  virtual void CallMethod(const gpb::MethodDescriptor* method,
                          gpb::RpcController* controller,
                          const gpb::Message* request,
                          gpb::Message* response,
                          gpb::Closure* done);

 private:
  struct Impl;
  Impl *impl_;
};

EVENTRPC_NAMESPACE_END
#endif  //  __EVENTRPC_RPCCLIENTEVENT_H__
