#ifndef __EVENTRPC_RPCEVENT_H__
#define __EVENTRPC_RPCEVENT_H__

#include <google/protobuf/service.h>
#include <map>
#include "base/base.h"
#include "net/eventi.h"

using std::map;

EVENTRPC_NAMESPACE_BEGIN

struct RpcMethod;

class RpcServerEvent : public Event {
 public:
  RpcServerEvent(const char *ip, int port);

  virtual ~RpcServerEvent();

  virtual bool OnWrite();

  virtual bool OnRead();

  bool RegisterService(gpb::Service *service);

  map<uint32_t, RpcMethod*>* rpc_methods();

 private:
  struct Impl;
  Impl *impl_;
};

EVENTRPC_NAMESPACE_END

#endif  //  __EVENTRPC_RPCEVENT_H__
