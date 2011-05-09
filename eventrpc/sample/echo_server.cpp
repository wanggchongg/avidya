#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include "log.h"
#include "rpc_server.h"
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
    VLOG_INFO() << "request: " << request->message();

    response->set_response(request->message());
    if (done) {
      done->Run();
    }
  }
  virtual void Dummy(::google::protobuf::RpcController* controller,
                     const ::echo::DummyRequest* request,
                     ::echo::DummyResponse* response,
                     ::google::protobuf::Closure* done) {
    VLOG_INFO() << "dummy request: " << request->message();
    if (done) {
      done->Run();
    }
  }
};

int main(int argc, char *argv[]) {
  SetProgramName(argv[0]);
  RpcServer rpc_server;
  gpb::Service *service = new EchoServiceImpl();
  rpc_server.rpc_method_manager()->RegisterService(service);
  rpc_server.set_host_and_port("127.0.0.1", 21118);
  rpc_server.Start();

  return 0;
}
