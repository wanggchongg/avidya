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
  Impl(MessageChannel *message_channel);

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
  MessageChannel *message_channel_;
  typedef list<MessageResponse*> MessageResponseList;
  typedef map<uint64, MessageResponseList> MessageResponseMap;
  MessageResponseMap message_response_map_;
  MessageResponseList free_response_list_;
};

RpcChannel::Impl::Impl(MessageChannel *message_channel)
  : message_channel_(message_channel) {
  message_channel_->set_message_handler(this);
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
  uint32 opcode = hash_string(request->GetTypeName());
  message_response_map_[opcode].push_back(message_response);
  message_channel_->SendMessage(request);
}

bool RpcChannel::Impl::HandlePacket(const MessageHeader &header,
                                    Buffer* buffer) {
  MessageResponseMap::iterator iter;
  uint32 opcode = ::ntohl(header.opcode);
  iter = message_response_map_.find(opcode);
  if (iter == message_response_map_.end()) {
    LOG_ERROR() << "cannot find handler for opcode: " << header.opcode;
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

RpcChannel::RpcChannel(MessageChannel *message_channel) {
   impl_ = new Impl(message_channel);
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
