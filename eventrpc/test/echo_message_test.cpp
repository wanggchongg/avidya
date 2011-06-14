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
#include "eventrpc/string_utility.h"
#include "test/echo.pb.h"
#include "test/echo_opcode.h"
using namespace std;
namespace eventrpc {
static const uint32 kMaxConnection = 80;
static uint32 kCount = 0;
class EchoClientMessageHandler : public ChannelMessageHandler {
 public:
  EchoClientMessageHandler(MessageChannel *channel,
                           Monitor *monitor)
    : ChannelMessageHandler(channel),
      monitor_(monitor) {
  }

  virtual ~EchoClientMessageHandler() {
  }

  bool HandleConnection(bool is_connected) {
    if (!is_connected) {
      return false;
    }
    for (uint32 i = 0; i < kMaxConnection; ++i) {
      echo::EchoRequest request;
      string content = StringUtility::ConvertUint32ToString(i);
      VLOG_INFO() << "i: " << i;
      request.set_message(content);
      channel_->SendPacket(CMSG_ECHO, &request);
    }
    return true;
  }

  bool HandlePacket(const MessageHeader &header,
                    Buffer* buffer) {
    if (header.opcode != SMSG_ECHO) {
      VLOG_ERROR() << "opcode error: " << header.opcode;
      return false;
    }
    echo::EchoResponse response;
    if (!buffer->DeserializeToMessage(&response, header.length)) {
      VLOG_ERROR() << "DeserializeToMessage error: " << header.opcode;
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
class EchoMessageTest : public testing::Test {
 public:
  EchoMessageTest()
    : server("127.0.0.1", 21118) {
  }

  void SetUp() {
    kCount = 0;
    dispatcher.Start();
    server.set_dispatcher(&dispatcher);
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
  Dispatcher client_dispatcher;
  Monitor monitor;
  MessageChannel channel("127.0.0.1", 21118);
  EchoClientMessageHandler handler(&channel, &monitor);
  channel.set_dispatcher(&client_dispatcher);
  channel.set_message_handler(&handler);
  client_dispatcher.Start();
  channel.Connect();
  monitor.Wait();
  ASSERT_EQ(kCount, kMaxConnection);
  VLOG_INFO() << "out of wait";
  channel.Close();
  client_dispatcher.Stop();
};
};
int main(int argc, char *argv[]) {
  eventrpc::SetProgramName(argv[0]);
  testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}
