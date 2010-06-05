#ifndef __EVENTRPC_RPCEVENT_H__
#define __EVENTRPC_RPCEVENT_H__

#include <google/protobuf/service.h>
#include "base.h"
#include "eventi.h"

EVENTRPC_NAMESPACE_BEGIN

class RpcServerEvent : public Event {
 public:
  RpcServerEvent(const char *ip, int port);

  virtual ~RpcServerEvent();

  virtual int OnWrite();

  virtual int OnRead();

  bool RegisterService(gpb::Service *service);

 private:
  struct Impl;
  Impl *impl_;
};

EVENTRPC_NAMESPACE_END

#endif  //  __EVENTRPC_RPCEVENT_H__
