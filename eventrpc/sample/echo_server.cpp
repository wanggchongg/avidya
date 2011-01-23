#include <unistd.h>
#include <iostream>
#include "reactor_rpc_server.h"
#include "echo.pb.h"

using namespace eventrpc;
using namespace std;

class EchoServiceImpl : public echo::EchoService {
 public:
  EchoServiceImpl() {
  };

  virtual void Echo(::google::protobuf::RpcController* controller,
                       const ::echo::EchoRequest* request,
                       ::echo::EchoResponse* response,
                       ::google::protobuf::Closure* done) {
    printf ("request: %s\n", request->message().c_str());
    response->set_response(request->message());
    if (done) {
      done->Run();
    }
  }
};

int main() {
  ReactorRpcServer reactor_rpc_server;
  gpb::Service *service = new EchoServiceImpl();
  reactor_rpc_server.rpc_method_manager()->RegisterService(service);
  reactor_rpc_server.set_host_and_port("127.0.0.1", 21118);
  reactor_rpc_server.Run();

  return 0;
}
