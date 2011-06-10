#include <iostream>
#include <stdio.h>
#include "eventrpc/monitor.h"
#include "eventrpc/dispatcher.h"
#include "eventrpc/rpc_channel.h"
#include "eventrpc/message_channel.h"
#include "eventrpc/message_header.h"
#include "eventrpc/log.h"
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
  Monitor monitor;
  MessageChannel channel("127.0.0.1", 21118);
  channel.set_dispatcher(&dispatcher);
  RpcChannel rpc_channel(&channel);
  dispatcher.Start();
  if (!channel.Connect()) {
    printf("connect to server failed, abort\n");
    exit(-1);
  }
  echo::EchoRequest request;
  request.set_message("hello");
  echo::EchoService::Stub stub(&rpc_channel);
  echo::EchoResponse response;
  stub.Echo(NULL, &request, &response,
            gpb::NewCallback(::echo_done, &response, &monitor));
  monitor.Wait();
  channel.Close();
  dispatcher.Stop();
  return 0;

#if 0
  SetProgramName(argv[0]);
  Dispatcher dispatcher;
  RpcChannel channel("127.0.0.1", 21118, &dispatcher);
  dispatcher.Start();
  if (!channel.Connect()) {
    printf("connect to server failed, abort\n");
    exit(-1);
  }
  echo::EchoService::Stub stub(&channel);
  echo::EchoRequest request;
  echo::EchoResponse response;

  {
  Monitor monitor;
  request.set_message("hello2");
  stub.Echo(NULL, &request, &response,
            gpb::NewCallback(::echo_done, &response, &monitor));
  monitor.Wait();
  }
  {
  Monitor monitor;
  request.set_message("hello");
  stub.Echo(NULL, &request, &response,
            gpb::NewCallback(::echo_done, &response, &monitor));
  monitor.Wait();
  }
  echo::DummyRequest dummy_request;
  dummy_request.set_message("dummy");
  stub.Dummy(NULL, &dummy_request, NULL, NULL);
  sleep(2);
  channel.Close();
  dispatcher.Stop();

  return 0;
#endif
}

