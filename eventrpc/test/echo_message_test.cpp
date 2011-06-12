#include <iostream>
#include <stdio.h>
#include <gtest/gtest.h>
#include "eventrpc/message_channel.h"
#include "eventrpc/monitor.h"
#include "eventrpc/dispatcher.h"
#include "eventrpc/message_header.h"
#include "eventrpc/message_server.h"
#include "eventrpc/log.h"
#include "eventrpc/buffer.h"
#include "test/echo.pb.h"
#include "test/echo_opcode.h"
using namespace std;
namespace eventrpc {
static const uint32 kMaxConnection = 400;
static uint32 kCount = 0;
class EchoClientMessageHandler : public MessageHandler {
 public:
  EchoClientMessageHandler(Monitor *monitor)
    : monitor_(monitor) {
  }

  virtual ~EchoClientMessageHandler() {
  }

  bool HandlePacket(const MessageHeader &header,
                    Buffer* buffer) {
    if (header.opcode != SMSG_ECHO) {
      VLOG_ERROR() << "opcode error: " << header.opcode;
      return false;
    }
    string content = buffer->ToString(header.length);
    echo::EchoResponse response;
    if (!response.ParseFromString(content)) {
      monitor_->Notify();
      return false;
    }
    ++kCount;
    VLOG_INFO() << "response: " << response.response()
      << ", count: " << kCount;
    if (kCount == kMaxConnection) {
      monitor_->Notify();
    }
    return true;
  }
  Monitor *monitor_;
};

class EchoServerMessageHandler : public ServerMessageHandler {
 public:
  EchoServerMessageHandler(MessageConnection *connection)
    : ServerMessageHandler(connection) {
  }

  virtual ~EchoServerMessageHandler() {
  }

  bool HandlePacket(const MessageHeader &header,
                    Buffer* buffer) {
    if (header.opcode != CMSG_ECHO) {
      VLOG_ERROR() << "opcode error: " << header.opcode;
      return false;
    }
    string content = buffer->ToString(header.length);
    echo::EchoRequest request;
    if (!request.ParseFromString(content)) {
      VLOG_ERROR() << "ParseFromString error: " << header.length;
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
class EchoMessageTest : public testing::Test {
 public:
  void SetUp() {
    kCount = 0;
    dispatcher.Start();
    server.set_dispatcher(&dispatcher);
    server.set_host_and_port("127.0.0.1", 21118);
    server.set_message_handler_factory(&factory);
    server.Start();
  }

  void TearDown() {
    server.Stop();
    dispatcher.Stop();
  }

  EchoServerMessageHandlerFactory factory;
  MessageServer server;
  Dispatcher dispatcher;
};

TEST_F(EchoMessageTest, Test1) {
  Dispatcher dispatcher;
  Monitor monitor;
  EchoClientMessageHandler handler(&monitor);
  MessageChannel channel("127.0.0.1", 21118);
  channel.set_dispatcher(&dispatcher);
  channel.set_message_handler(&handler);
  dispatcher.Start();
  if (!channel.Connect()) {
    printf("connect to server failed, abort\n");
    exit(-1);
  }
  echo::EchoRequest request;
  for (uint32 i = 0; i < kMaxConnection; ++i) {
    VLOG_INFO() << "i: " << i;
    request.set_message("hello");
    channel.SendPacket(CMSG_ECHO, &request);
  }
  monitor.Wait();
  ASSERT_EQ(kCount, kMaxConnection);
  channel.Close();
  dispatcher.Stop();
};
};
int main(int argc, char *argv[]) {
  eventrpc::SetProgramName(argv[0]);
  testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}
