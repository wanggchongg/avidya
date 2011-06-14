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
#include "eventrpc/task.h"
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

struct CallMethodTask : public Task {
  CallMethodTask(uint32 opcode,
                     gpb::Message* request,
                     RpcChannel::Impl *impl)
    : opcode_(opcode),
      request_(request),
      impl_(impl) {
  }

  virtual ~CallMethodTask() {
  }
  void Handle();

  uint32 opcode_;
  gpb::Message* request_;
  RpcChannel::Impl *impl_;
};

struct RpcChannel::Impl : public ChannelMessageHandler {
 public:
  Impl(MessageChannel *channel);

  ~Impl();

  void CallMethod(const gpb::MethodDescriptor* method,
                  gpb::RpcController* controller,
                  const gpb::Message* request,
                  gpb::Message* response,
                  gpb::Closure* done);

  bool HandleConnection(bool is_connected);

  bool HandlePacket(const MessageHeader &header,
                    Buffer* buffer);

  MessageResponse* GetMessageResponse();

  void FreeMessageResponse(MessageResponse *response);

  void SendPacket(uint32 opcode, const ::google::protobuf::Message *message);

  void PushTask(CallMethodTask *callback);

 public:
  typedef list<MessageResponse*> MessageResponseList;
  typedef map<uint64, MessageResponseList> MessageResponseMap;
  MessageResponseMap message_response_map_;
  MessageResponseList free_response_list_;
};

RpcChannel::Impl::Impl(MessageChannel *channel)
  : ChannelMessageHandler(channel) {
  channel->set_message_handler(this);
}

RpcChannel::Impl::~Impl() {
}

bool RpcChannel::Impl::HandleConnection(bool is_connected) {
  return is_connected;
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
  gpb::Message* save_request = request->New();
  if (!save_request->IsInitialized()) {
    save_request->CopyFrom(*request);
  }
  VLOG_INFO() << "call service: " << method->full_name()
    << ", opcode: " << opcode
    << ", request: " << save_request->DebugString();
  message_response_map_[opcode].push_back(message_response);
  CallMethodTask *callback = new CallMethodTask(
      opcode,
      save_request,
      this);
  PushTask(callback);
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
  if (!buffer->DeserializeToMessage(response->response,
                                    header.length)) {
    VLOG_ERROR() << "DeserializeToMessage " << header.opcode << " error";
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
  free_response_list_.push_back(response);
}

void RpcChannel::Impl::SendPacket(uint32 opcode,
                                  const ::google::protobuf::Message *message) {
  channel_->SendPacket(opcode, message);
}

void RpcChannel::Impl::PushTask(CallMethodTask *task) {
  channel_->dispatcher()->PushTask(task);
}

void CallMethodTask::Handle() {
  VLOG_INFO() << "request: " << request_->DebugString();
  impl_->SendPacket(opcode_, request_);
  delete request_;
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
