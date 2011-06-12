#include <iostream>
#include <stdio.h>
#include "eventrpc/monitor.h"
#include "eventrpc/dispatcher.h"
#include "eventrpc/rpc_channel.h"
#include "eventrpc/message_channel.h"
#include "eventrpc/message_header.h"
#include "eventrpc/log.h"
#include "eventrpc/assert_log.h"
#include "eventrpc/buffer.h"
#include "sample/echo.pb.h"
using namespace eventrpc;
using namespace std;

void echo_done(echo::EchoResponse* resp,
               Monitor *monitor) {
  VLOG_INFO() << "response: " << resp->response();
  monitor->Notify();
}

int main(int argc, char *argv[]) {
  SetProgramName(argv[0]);
  Dispatcher dispatcher;
  echo::EchoRequest request;
  request.set_message("hello");
  VLOG_INFO() << "request: " << request.DebugString();
  Monitor monitor;
  RpcChannel rpc_channel("127.0.0.1", 21118);
  rpc_channel.set_dispatcher(&dispatcher);
  dispatcher.Start();
  rpc_channel.Connect();
  echo::EchoService::Stub stub(&rpc_channel);
  echo::EchoResponse response;
  stub.Echo(NULL, &request, &response,
            gpb::NewCallback(::echo_done, &response, &monitor));
  monitor.Wait();
  rpc_channel.Close();
  dispatcher.Stop();
  return 0;
}

