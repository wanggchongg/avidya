#include <iostream>
#include <stdio.h>
#include "dispatcher.h"
#include "rpc_channel.h"
#include "monitor.h"
#include "echo.pb.h"

using namespace eventrpc;
using namespace std;

void echo_done(echo::EchoResponse* resp,
               Monitor *monitor) {
  printf("response: %s\n", resp->response().c_str());
  monitor->Notify();
}

int main() {
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
  request.set_message("hello");
  Monitor monitor;
  stub.Echo(NULL, &request, &response,
            gpb::NewCallback(::echo_done, &response, &monitor));
  monitor.Wait();
  channel.Close();
  dispatcher.Stop();

  return 0;
}

