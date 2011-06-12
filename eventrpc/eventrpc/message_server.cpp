/*
 * Copyright(C) lichuang
 */
#include <arpa/inet.h>
#include "eventrpc/event.h"
#include "eventrpc/log.h"
#include "eventrpc/assert_log.h"
#include "eventrpc/net_utility.h"
#include "eventrpc/utility.h"
#include "eventrpc/message_connection_manager.h"
#include "eventrpc/message_server.h"
namespace eventrpc {
class MessageServerEventHandler;
struct MessageServer::Impl {
 public:
  Impl();

  ~Impl();

  void Start();

  void Stop();

  void set_host_and_port(const string &host, uint32 port) {
    host_ = host;
    port_ = port;
  }

  void set_dispatcher(Dispatcher *dispatcher) {
    dispatcher_ = dispatcher;
  }

  void set_message_handler_factory(ServerMessageHandlerFactory *factory) {
    manager_.set_message_handler_factory(factory);
  }

  bool HandleAccept();

  void Close();

 public:
  string host_;
  uint32 port_;
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

MessageServer::Impl::Impl()
  : host_(""),
    port_(0),
    listen_fd_(0),
    event_handler_(new MessageServerEventHandler(this)),
    dispatcher_(NULL) {
}

MessageServer::Impl::~Impl() {
  Close();
}

void MessageServer::Impl::Start() {
  listen_fd_ = NetUtility::Listen(host_.c_str(), port_);
  ASSERT_TRUE(listen_fd_ > 0);
  dispatcher_->AddEvent(listen_fd_, EVENT_READ, event_handler_);
}

void MessageServer::Impl::Stop() {
  Close();
}

void MessageServer::Impl::Close() {
  if (listen_fd_ > 0) {
    LOG_INFO() << "shut down listen on " << host_ << " : " << port_;
    close(listen_fd_);
  }
}

bool MessageServer::Impl::HandleAccept() {
  int fd;
  char buffer[30];
  while (true) {
    struct sockaddr_in address;
    fd = NetUtility::Accept(listen_fd_, &address);
    if (fd == -1) {
      break;
    }
    VLOG_INFO() << "accept connection from "
      << inet_ntop(AF_INET, &address.sin_addr, buffer, sizeof(buffer))
      << ":" << ntohs(address.sin_port);
    MessageConnection* connection = manager_.GetConnection();
    connection->set_fd(fd);
    connection->set_client_address(address);
    connection->set_dispacher(dispatcher_);
    dispatcher_->AddEvent(fd, EVENT_READ | EVENT_WRITE,
                          connection->event_handler());
  }
  return true;
}

MessageServer::MessageServer() {
  impl_ = new Impl();
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

void MessageServer::set_host_and_port(const string &host, uint32 port) {
  impl_->set_host_and_port(host, port);
}

void MessageServer::set_dispatcher(Dispatcher *dispatcher) {
  impl_->set_dispatcher(dispatcher);
}

void MessageServer::set_message_handler_factory(
    ServerMessageHandlerFactory *factory) {
  impl_->set_message_handler_factory(factory);
}
}; // namespace eventrpc
