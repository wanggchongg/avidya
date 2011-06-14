/*
 * Copyright(C) lichuang
 */
#include <map>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/stubs/common.h>
#include "eventrpc/rpc_method_manager.h"
#include "eventrpc/log.h"
#include "eventrpc/assert_log.h"
#include "eventrpc/utility.h"
#include "eventrpc/callback.h"
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

struct RpcMethodManager::Impl {
 public:
  Impl();

  ~Impl();

  void RegisterService(gpb::Service *service);

  bool HandlePacket(const MessageHeader &header,
                    Buffer* buffer,
                    MessageConnection *connection);
 private:
  typedef map<uint32, RpcMethod*> RpcMethodMap;
  RpcMethodMap rpc_method_map_;
};

RpcMethodManager::Impl::Impl() {
}

RpcMethodManager::Impl::~Impl() {
  RpcMethodMap::iterator iter = rpc_method_map_.begin();
  for (; iter != rpc_method_map_.end(); ) {
    RpcMethod *rpc_method = iter->second;
    ++iter;
    delete rpc_method;
  }
}

void RpcMethodManager::Impl::RegisterService(gpb::Service *service) {
  const gpb::ServiceDescriptor *descriptor = service->GetDescriptor();
  for (int i = 0; i < descriptor->method_count(); ++i) {
    const gpb::MethodDescriptor *method = descriptor->method(i);
    const google::protobuf::Message *request =
      &service->GetRequestPrototype(method);
    const google::protobuf::Message *response =
      &service->GetResponsePrototype(method);
    RpcMethod *rpc_method = new RpcMethod(service, request,
                                          response, method);
    uint32 opcode = hash_string(method->full_name());
    VLOG_INFO() << "register service: " << method->full_name()
      << ", opcode: " << opcode;
    ASSERT_EQ(rpc_method_map_.find(opcode),
              rpc_method_map_.end()) << "rpc method "
              << method->full_name() << " duplicated";
    rpc_method_map_[opcode] = rpc_method;
  }
}

struct HandleServiceEntry {
  HandleServiceEntry(const gpb::MethodDescriptor *method,
                     gpb::Message *request,
                     gpb::Message *response,
                     MessageConnection *connection)
    : method_(method),
      request_(request),
      response_(response),
      connection_(connection) {
  }
  const gpb::MethodDescriptor *method_;
  gpb::Message *request_;
  gpb::Message *response_;
  MessageConnection *connection_;
};

static void HandleServiceDone(HandleServiceEntry *entry) {
  uint32 opcode = hash_string(entry->method_->full_name());
  entry->connection_->SendPacket(opcode, entry->response_);
  delete entry->request_;
  delete entry->response_;
  delete entry;
}

bool RpcMethodManager::Impl::HandlePacket(
    const MessageHeader &header,
    Buffer* buffer,
    MessageConnection *connection) {
  uint32 opcode = header.opcode;
  RpcMethod *rpc_method = rpc_method_map_[opcode];
  if (rpc_method == NULL) {
    VLOG_ERROR() << "opcode " << header.opcode << " not registered";
    return false;
  }
  const gpb::MethodDescriptor *method = rpc_method->method_;
  gpb::Message *request = rpc_method->request_->New();
  gpb::Message *response = rpc_method->response_->New();
  if (!buffer->DeserializeToMessage(request, header.length)) {
    delete request;
    delete response;
    VLOG_ERROR() << "DeserializeToMessage " << header.opcode << " error";
    return false;
  }
  HandleServiceEntry *entry = new HandleServiceEntry(method,
                                                     request,
                                                     response,
                                                     connection);
  gpb::Closure *done = gpb::NewCallback(&HandleServiceDone,
                                        entry);
  rpc_method->service_->CallMethod(method,
                                   NULL,
                                   request, response, done);
  return true;
}

RpcMethodManager::RpcMethodManager() {
  impl_ = new Impl();
}

RpcMethodManager::~RpcMethodManager() {
  delete impl_;
}

void RpcMethodManager::RegisterService(gpb::Service *service) {
  impl_->RegisterService(service);
}

bool RpcMethodManager::HandlePacket(const MessageHeader &header,
                                    Buffer* buffer,
                                    MessageConnection *connection) {
  return impl_->HandlePacket(header, buffer, connection);
}
};
