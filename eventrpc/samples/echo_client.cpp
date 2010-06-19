#include <iostream>
#include <stdio.h>
#include "rpcclientevent.h"
#include "eventpoller.h"
#include "echo.pb.h"

using namespace eventrpc;
using namespace std;

void echo_done(echo::EchoResponse* resp, RpcClientEvent *event)
{
  printf("response: %s\n", resp->response().c_str());
  event = event;
  event->Close();
}

int main() {
  EventPoller eventpoller;
  RpcClientEvent *event = new RpcClientEvent("127.0.0.1", 2008);
  eventpoller.AddEvent(event);

  echo::EchoService::Stub stub(event);
  echo::EchoRequest request;
  echo::EchoResponse response;
  request.set_message("hello");
  stub.Echo(NULL, &request, &response,
            gpb::NewCallback(::echo_done, &response, event));

  /*
  request.set_message("hello2");
  stub.Echo(NULL, &request, &response,
            gpb::NewCallback(::echo_done, &response, event));
  */

  eventpoller.Loop();

  return 0;
}

