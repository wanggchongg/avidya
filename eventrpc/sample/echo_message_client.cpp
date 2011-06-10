#include <iostream>
#include <stdio.h>
#include "eventrpc/monitor.h"
#include "eventrpc/dispatcher.h"
#include "eventrpc/message_channel.h"
#include "eventrpc/message_header.h"
#include "eventrpc/log.h"
#include "eventrpc/buffer.h"
#include "sample/echo.pb.h"

using namespace eventrpc;
using namespace std;

class EchoClientMessageHandler : public MessageHandler {
 public:
  EchoClientMessageHandler(Monitor *monitor)
    : monitor_(monitor) {
  }

  virtual ~EchoClientMessageHandler() {
  }

  bool HandlePacket(const MessageHeader &header,
                    Buffer* buffer) {
    string content = buffer->ToString(header.length);
    echo::EchoResponse response;
    if (!response.ParseFromString(content)) {
      monitor_->Notify();
      return false;
    }
    monitor_->Notify();
    VLOG_INFO() << "response: " << response.response();
    return true;
  }
  Monitor *monitor_;
};

int main(int argc, char *argv[]) {
  SetProgramName(argv[0]);
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
  request.set_message("hello");
  channel.SendMessage(&request);
  monitor.Wait();
  channel.Close();
  dispatcher.Stop();

  return 0;
}

