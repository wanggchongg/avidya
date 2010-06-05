#include "echo.pb.h"
#include <string>

using namespace std;

int main()
{
  echo::EchoRequest request;
  echo::EchoResponse response;

  request.set_message("test");
  string buffer;

  request.SerializeToString(&buffer);
  printf("message: %s\n", (char*)buffer.c_str());

  response.ParseFromString(buffer);
  printf("response message: %s\n", (char*)response.response().c_str());

  return 0;
}

