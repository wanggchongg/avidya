
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/stubs/common.h>
#include "rpc_method_manager.h"
#include "log.h"
#include "utility.h"
#include "meta.h"
#include "callback.h"

namespace eventrpc {
RpcMethodManager::RpcMethodManager() {
}

void RpcMethodManager::RegisterService(gpb::Service *service) {
  const gpb::ServiceDescriptor *descriptor = service->GetDescriptor();
  for (int i = 0; i < descriptor->method_count(); ++i) {
    const gpb::MethodDescriptor *method = descriptor->method(i);
    const google::protobuf::Message *request =
      &service->GetRequestPrototype(method);
    const google::protobuf::Message *response =
      &service->GetResponsePrototype(method);
    RpcMethod *rpc_method = new RpcMethod(service, request,
                                          response, method);
    uint32 method_id = hash_string(method->full_name());
    ASSERT_EQ(rpc_methods_.find(method_id),
              rpc_methods_.end()) << "rpc method "
              << method->full_name() << " duplicated";
    rpc_methods_[method_id] = rpc_method;
  }
}

bool RpcMethodManager::IsServiceRegistered(uint32 method_id) {
  return (rpc_methods_.find(method_id) != rpc_methods_.end());
}

struct HandleServiceEntry {
  HandleServiceEntry(const gpb::MethodDescriptor *method,
                     gpb::Message *request,
                     gpb::Message *response,
                     string *message,
                     Meta *meta,
                     Callback *callback)
    : method_(method),
      request_(request),
      response_(response),
      message_(message),
      meta_(meta),
      callback_(callback) {
  }
  const gpb::MethodDescriptor *method_;
  gpb::Message *request_;
  gpb::Message *response_;
  string *message_;
  Meta *meta_;
  Callback *callback_;
};

static void HandleServiceDone(HandleServiceEntry *entry) {
  *(entry->message_) = "";
  entry->meta_->EncodeWithMessage(entry->method_->full_name(),
                                  entry->response_, entry->message_);
  Callback *callback = entry->callback_;
  delete entry->request_;
  delete entry->response_;
  delete entry;
  callback->Run();
}

int  RpcMethodManager::HandleService(string *message,
                                     Meta *meta, Callback *callback) {
  RpcMethod *rpc_method = rpc_methods_[meta->method_id()];
  const gpb::MethodDescriptor *method = rpc_method->method_;
  gpb::Message *request = rpc_method->request_->New();
  gpb::Message *response = rpc_method->response_->New();
  request->ParseFromString(*message);
  HandleServiceEntry *entry = new HandleServiceEntry(method,
                                                     request,
                                                     response,
                                                     message,
                                                     meta,
                                                     callback);
  gpb::Closure *done = gpb::NewCallback(
      &HandleServiceDone, entry);
  rpc_method->service_->CallMethod(method,
                                   NULL,
                                   request, response, done);
  return 0;
}
};
