#include <iostream>
#include <stdio.h>
#include "dispatcher.h"
#include "rpc_channel.h"
#include "echo.pb.h"

using namespace eventrpc;
using namespace std;

void echo_done(echo::EchoResponse* resp, RpcChannel *channel) {
  printf("response: %s\n", resp->response().c_str());
  channel->Close();
}

int main() {
  Dispatcher dispatcher;
  RpcChannel *channel = new RpcChannel("127.0.0.1", 21118, &dispatcher);
  if (!channel->Connect()) {
    printf("connect to server failed, abort\n");
    exit(-1);
  }
  echo::EchoService::Stub stub(channel);
  echo::EchoRequest request;
  echo::EchoResponse response;
  request.set_message("hello");
  stub.Echo(NULL, &request, &response,
            gpb::NewCallback(::echo_done, &response, channel));

  while (dispatcher.Poll() == 0) {
    ;
  }
  delete channel;

  return 0;
}

