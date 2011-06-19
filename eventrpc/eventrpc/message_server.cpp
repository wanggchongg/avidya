/*
 * Copyright(C) lichuang
 */
#include <arpa/inet.h>
#include "eventrpc/event.h"
#include "eventrpc/log.h"
#include "eventrpc/net_address.h"
#include "eventrpc/net_utility.h"
#include "eventrpc/utility.h"
#include "eventrpc/message_connection_manager.h"
#include "eventrpc/message_server.h"
namespace eventrpc {
class MessageServerEventHandler;
struct MessageServer::Impl {
 public:
  Impl(const string &host, int port);

  ~Impl();

  void Start();

  void Stop();

  void set_dispatcher(Dispatcher *dispatcher) {
    dispatcher_ = dispatcher;
  }

  void set_message_handler_factory(ServerMessageHandlerFactory *factory) {
    manager_.set_message_handler_factory(factory);
  }

  bool HandleAccept();

  void Close();

 public:
  NetAddress listen_address_;
  int listen_fd_;
  MessageServerEventHandler *event_handler_;
  Dispatcher *dispatcher_;
  MessageConnectionManager manager_;
};

struct MessageServerEventHandler : public EventHandler {
 public:
  MessageServerEventHandler(MessageServer::Impl *server)
    : server_(server) {
  }

  virtual ~MessageServerEventHandler() {
  }

  bool HandleRead() {
    return server_->HandleAccept();
  }

  bool HandleWrite() {
    // should never happen
    return false;
  }

 public:
  MessageServer::Impl *server_;
};

MessageServer::Impl::Impl(const string &host, int port)
  : listen_address_(host, port),
    listen_fd_(0),
    event_handler_(new MessageServerEventHandler(this)),
    dispatcher_(NULL) {
}

MessageServer::Impl::~Impl() {
  Close();
}

void MessageServer::Impl::Start() {
  listen_fd_ = NetUtility::Listen(listen_address_);
  EASSERT_TRUE(listen_fd_ > 0);
  VLOG_INFO() << "create listen fd " << listen_fd_
    << " for " << listen_address_.DebugString();
  dispatcher_->AddEvent(listen_fd_, EVENT_READ, event_handler_);
}

void MessageServer::Impl::Stop() {
  Close();
}

void MessageServer::Impl::Close() {
  if (listen_fd_ > 0) {
    LOG_INFO() << "shut down listen on " << listen_address_.DebugString();
    close(listen_fd_);
  }
}

bool MessageServer::Impl::HandleAccept() {
  int fd = -1;
  bool result = false;
  while (true) {
    struct sockaddr_in address;
    result = NetUtility::Accept(listen_fd_, &address, &fd);
    if (result == false) {
      return false;
    }
    if (fd == 0) {
      break;
    }
    NetAddress client_address(address);
    VLOG_INFO() << "accept connection from "
      << client_address.DebugString()
      << ", fd: " << fd;
    MessageConnection* connection = manager_.GetConnection();
    connection->set_fd(fd);
    connection->set_client_address(address);
    connection->set_dispacher(dispatcher_);
    dispatcher_->AddEvent(fd, EVENT_READ | EVENT_WRITE,
                          connection->event_handler());
  }
  return true;
}

MessageServer::MessageServer(const string &host, int port) {
  impl_ = new Impl(host, port);
}

MessageServer::~MessageServer() {
  delete impl_;
}

void MessageServer::Start() {
  impl_->Start();
}

void MessageServer::Stop() {
  impl_->Stop();
}

void MessageServer::set_dispatcher(Dispatcher *dispatcher) {
  impl_->set_dispatcher(dispatcher);
}

void MessageServer::set_message_handler_factory(
    ServerMessageHandlerFactory *factory) {
  impl_->set_message_handler_factory(factory);
}
}; // namespace eventrpc
