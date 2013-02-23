// Copyright (C) 2013, avidya
// Created by lichuang1982@gmail.com

#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <iostream>
#include "eventrpc/log.h"
#include "eventrpc/rpc_server.h"
#include "sample/echo.pb.h"

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
  RpcServer rpc_server("127.0.0.1", 21118);
  Dispatcher dispatcher;
  dispatcher.Start();
  rpc_server.set_dispatcher(&dispatcher);
  gpb::Service *service = new EchoServiceImpl();
  rpc_server.RegisterService(service);
  rpc_server.Start();
  sigset_t new_mask;
  sigfillset(&new_mask);
  sigset_t old_mask;
  sigset_t wait_mask;
  sigemptyset(&wait_mask);
  sigaddset(&wait_mask, SIGINT);
  sigaddset(&wait_mask, SIGQUIT);
  sigaddset(&wait_mask, SIGTERM);
  pthread_sigmask(SIG_BLOCK, &new_mask, &old_mask);
  pthread_sigmask(SIG_SETMASK, &old_mask, 0);
  pthread_sigmask(SIG_BLOCK, &wait_mask, 0);
  int sig = 0;
  sigwait(&wait_mask, &sig);

  return 0;
}
