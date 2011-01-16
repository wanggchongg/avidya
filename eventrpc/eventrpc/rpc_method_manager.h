#ifndef __EVENTRPC_RPC_METHOD_MANAGER_H__
#define __EVENTRPC_RPC_METHOD_MANAGER_H__

#include <map>
#include <string>
#include <google/protobuf/service.h>
#include "base.h"
using std::map;
using std::string;
namespace eventrpc {
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

typedef map<uint32, RpcMethod*> RpcMethodMap;

class Meta;
class RpcMethodManager {
 public:
  RpcMethodManager();

  void RegisterService(gpb::Service *service);
  // TODO: add more check, not only method id!
  bool IsServiceRegisted(uint32 method_id);

  int  HandleService(string *message, Meta *meta);

 private:
  RpcMethodMap rpc_methods_;
};
};
#endif  //  __EVENTRPC_RPC_METHOD_MANAGER_H__
