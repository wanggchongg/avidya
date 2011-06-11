/*
 * Copyright(C) lichuang
 */
#include <map>
#include <list>
#include <arpa/inet.h>  // htonl, ntohl
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "eventrpc/error_code.h"
#include "eventrpc/base.h"
#include "eventrpc/event.h"
#include "eventrpc/rpc_channel.h"
#include "eventrpc/net_utility.h"
#include "eventrpc/utility.h"
#include "eventrpc/log.h"
namespace eventrpc {
struct MessageResponse {
  gpb::Message* response;
  gpb::Closure* done;
};

struct RpcChannel::Impl : public MessageHandler {
 public:
  Impl(RpcChannel *rpc_channel);

  ~Impl();

  void CallMethod(const gpb::MethodDescriptor* method,
                  gpb::RpcController* controller,
                  const gpb::Message* request,
                  gpb::Message* response,
                  gpb::Closure* done);

  bool HandlePacket(const MessageHeader &header,
                    Buffer* buffer);

  MessageResponse* GetMessageResponse();

  void FreeMessageResponse(MessageResponse *response);

 public:
  RpcChannel *rpc_channel_;
  typedef list<MessageResponse*> MessageResponseList;
  typedef map<uint64, MessageResponseList> MessageResponseMap;
  MessageResponseMap message_response_map_;
  MessageResponseList free_response_list_;
};

RpcChannel::Impl::Impl(RpcChannel *rpc_channel)
  : rpc_channel_(rpc_channel) {
  rpc_channel_->set_message_handler(this);
}

RpcChannel::Impl::~Impl() {
}

void RpcChannel::Impl::CallMethod(const gpb::MethodDescriptor* method,
                                  gpb::RpcController* controller,
                                  const gpb::Message* request,
                                  gpb::Message* response,
                                  gpb::Closure* done) {
  MessageResponse *message_response = GetMessageResponse();
  message_response->response = response;
  message_response->done     = done;
  uint32 opcode = hash_string(method->full_name());
  VLOG_INFO() << "register service: " << method->full_name()
    << ", opcode: " << opcode;
  message_response_map_[opcode].push_back(message_response);
  //rpc_channel_->SendMessage(request);
  rpc_channel_->SendPacket(opcode, request);
}

bool RpcChannel::Impl::HandlePacket(const MessageHeader &header,
                                    Buffer* buffer) {
  MessageResponseMap::iterator iter;
  iter = message_response_map_.find(header.opcode);
  if (iter == message_response_map_.end()) {
    VLOG_ERROR() << "cannot find handler for opcode: " << header.opcode;
    return false;
  }
  MessageResponse *response = iter->second.front();
  iter->second.pop_front();
  string content = buffer->ToString(header.length);
  if (!response->response->ParseFromString(content)) {
    FreeMessageResponse(response);
    return false;
  }
  response->done->Run();
  FreeMessageResponse(response);
  return true;
}

MessageResponse* RpcChannel::Impl::GetMessageResponse() {
  if (free_response_list_.empty()) {
    return new MessageResponse();
  }
  MessageResponse *message_response = free_response_list_.front();
  free_response_list_.pop_front();
  return message_response;
}

void RpcChannel::Impl::FreeMessageResponse(MessageResponse *response) {
  delete response->response;
  delete response->done;
  free_response_list_.push_back(response);
}

RpcChannel::RpcChannel(const string &host, int port)
  : MessageChannel(host, port) {
   impl_ = new Impl(this);
}

RpcChannel::~RpcChannel() {
  delete impl_;
}

void RpcChannel::CallMethod(const gpb::MethodDescriptor* method,
                            gpb::RpcController* controller,
                            const gpb::Message* request,
                            gpb::Message* response,
                            gpb::Closure* done) {
  impl_->CallMethod(method, controller, request, response, done);
}
};
