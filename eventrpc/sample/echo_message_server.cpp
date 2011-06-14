#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <iostream>
#include "eventrpc/log.h"
#include "eventrpc/base.h"
#include "eventrpc/message_server.h"
#include "sample/echo.pb.h"
#include "sample/echo_opcode.h"

using namespace eventrpc;
using namespace std;

class EchoServerMessageHandler : public ServerMessageHandler {
 public:
  EchoServerMessageHandler(MessageConnection *connection)
    : ServerMessageHandler(connection) {
  }

  virtual ~EchoServerMessageHandler() {
  }

  bool HandlePacket(const MessageHeader &header,
                    Buffer* buffer) {
    VLOG_INFO() << "HandlePacket";
    if (header.opcode != CMSG_ECHO) {
      VLOG_ERROR() << "opcode error: " << header.opcode;
      return false;
    }
    echo::EchoRequest request;
    if (!buffer->DeserializeToMessage(&request, header.length)) {
      VLOG_ERROR() << "DeserializeToMessage error: " << header.length;
      return false;
    }
    VLOG_INFO() << "request: " << request.message();
    echo::EchoResponse response;
    response.set_response(request.message());
    connection_->SendPacket(SMSG_ECHO, &response);
    return true;
  }
};

class EchoServerMessageHandlerFactory: public ServerMessageHandlerFactory {
 public:
  EchoServerMessageHandlerFactory() {
  }
  virtual ~EchoServerMessageHandlerFactory() {
  }
  ServerMessageHandler* CreateHandler(MessageConnection *connection) {
    return new EchoServerMessageHandler(connection);
  }
};

int main(int argc, char *argv[]) {
  SetProgramName(argv[0]);
  MessageServer server("127.0.0.1", 21118);
  Dispatcher dispatcher;
  dispatcher.Start();
  server.set_dispatcher(&dispatcher);
  EchoServerMessageHandlerFactory factory;
  server.set_message_handler_factory(&factory);
  server.Start();

  sigset_t new_mask;
  sigfillset(&new_mask);
  sigset_t old_mask;
  sigset_t wait_mask;
  sigemptyset(&wait_mask);
  sigaddset(&wait_mask, SIGINT);
  sigaddset(&wait_mask, SIGQUIT);
  sigaddset(&wait_mask, SIGTERM);
  pthread_sigmask(SIG_BLOCK, &new_mask, &old_mask);
  pthread_sigmask(SIG_SETMASK, &old_mask, 0);
  pthread_sigmask(SIG_BLOCK, &wait_mask, 0);
  int sig = 0;
  sigwait(&wait_mask, &sig);

  return 0;
}
