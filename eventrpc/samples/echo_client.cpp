#include <iostream>
#include <stdio.h>
#include "rpcclientevent.h"
#include "eventpoller.h"
#include "echo.pb.h"

using namespace eventrpc;
using namespace std;

void echo_done(echo::EchoResponse* resp)
{
  printf("response: %s\n", resp->response().c_str());
}

int main() {
  EventPoller eventpoller;
  RpcClientEvent *event = new RpcClientEvent("127.0.0.1", 2008);
  eventpoller.AddEvent(event->event(), event);

  echo::EchoService::Stub stub(event);
  echo::EchoRequest request;
  echo::EchoResponse response;
  request.set_message("hello");
  stub.Echo(NULL, &request, &response,
            gpb::NewCallback(::echo_done, &response));

  eventpoller.Loop();

  return 0;
}

