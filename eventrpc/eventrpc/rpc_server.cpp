/*
 * Copyright(C) lichuang
 */
#include "eventrpc/utility.h"
#include "eventrpc/rpc_server.h"
#include "eventrpc/event.h"
#include "eventrpc/log.h"
#include "eventrpc/net_utility.h"
#include "eventrpc/rpc_connection.h"
#include "eventrpc/rpc_method_manager.h"
namespace eventrpc {
struct RpcServerMessageHandler : public ServerMessageHandler {
  virtual ~RpcServerMessageHandler() {}
  bool HandlePacket(const MessageHeader &header,
                    Buffer* buffer);

  RpcServerMessageHandler(MessageConnection *connection)
    : ServerMessageHandler(connection) {
  }
  RpcMethodManager *method_manager_;
};

class RpcServerMessageHandlerFactory: public ServerMessageHandlerFactory {
 public:
  RpcServerMessageHandlerFactory(RpcMethodManager *method_manager)
    : method_manager_(method_manager){
  }

  virtual ~RpcServerMessageHandlerFactory() {
  }

  virtual ServerMessageHandler* CreateHandler(MessageConnection *connection) {
    RpcServerMessageHandler *handler =
      new RpcServerMessageHandler(connection);
    handler->method_manager_ = method_manager_;
    return handler;
  }
  RpcMethodManager *method_manager_;
};

struct RpcServer::Impl {
 public:
  Impl(RpcServer *rpc_server);
  ~Impl();

  void RegisterService(gpb::Service *service);

 private:
  RpcServer *rpc_server_;
  RpcMethodManager method_manager_;
  RpcServerMessageHandlerFactory factory_;
};

RpcServer::Impl::Impl(RpcServer *rpc_server)
  : rpc_server_(rpc_server),
    factory_(&method_manager_) {
  rpc_server_->set_message_handler_factory(&factory_);
}

RpcServer::Impl::~Impl() {
}

void RpcServer::Impl::RegisterService(gpb::Service *service) {
  method_manager_.RegisterService(service);
}

bool RpcServerMessageHandler::HandlePacket(
    const MessageHeader &header,
    Buffer* buffer) {
  return method_manager_->HandlePacket(header, buffer, connection_);
}

RpcServer::RpcServer(const string &host, int port)
  : MessageServer(host, port) {
  impl_ = new Impl(this);
}

RpcServer::~RpcServer() {
  delete impl_;
}

void RpcServer::RegisterService(gpb::Service *service) {
  impl_->RegisterService(service);
}
}; // namespace eventrpc

