#include <iostream>
#include <stdio.h>
#include "dispatcher.h"
#include "rpc_channel.h"
#include "monitor.h"
#include "log.h"
#include "echo.pb.h"

using namespace eventrpc;
using namespace std;

void echo_done(echo::EchoResponse* resp,
               Monitor *monitor) {
  printf("response: %s\n", resp->response().c_str());
  monitor->Notify();
}

int main(int argc, char *argv[]) {
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
  request.set_message("hello 2");
  stub.Echo(NULL, &request, &response, NULL);
  sleep(1);
  request.set_message("hello");
  stub.Echo(NULL, &request, &response, NULL);
  sleep(1);
  channel.Close();
  dispatcher.Stop();

  return 0;
}

