#include <iostream>
#include <stdio.h>
#include "net/rpcclientevent.h"
#include "net/dispatcher.h"
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
  Dispatcher dispatcher;
  RpcClientEvent *event = new RpcClientEvent("127.0.0.1", 2008);
  dispatcher.AddEvent(event);

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

  dispatcher.Loop();
  delete event;

  return 0;
}

