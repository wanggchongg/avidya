#include <iostream>
#include <stdio.h>
#include "eventrpc/monitor.h"
#include "eventrpc/dispatcher.h"
#include "eventrpc/message_channel.h"
#include "eventrpc/message_header.h"
#include "eventrpc/log.h"
#include "eventrpc/buffer.h"
#include "sample/echo.pb.h"
#include "sample/echo_opcode.h"

using namespace eventrpc;
using namespace std;

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
    echo::EchoRequest request;
    request.set_message("hello");
    VLOG_INFO() << "HandleConnection";
    channel_->SendPacket(CMSG_ECHO, &request);
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
    VLOG_INFO() << "response: " << response.response();
    monitor_->Notify();
    return true;
  }
  Monitor *monitor_;
};

int main(int argc, char *argv[]) {
  SetProgramName(argv[0]);
  Dispatcher dispatcher;
  Monitor monitor;
  MessageChannel channel("127.0.0.1", 21118);
  EchoClientMessageHandler handler(&channel, &monitor);
  channel.set_dispatcher(&dispatcher);
  channel.set_message_handler(&handler);
  dispatcher.Start();
  channel.Connect();
  monitor.Wait();
  channel.Close();
  dispatcher.Stop();

  return 0;
}

