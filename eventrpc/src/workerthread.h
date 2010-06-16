#ifndef __EVENTRPC_WORKERTHREAD_H__
#define __EVENTRPC_WORKERTHREAD_H__

#include "base.h"
#include "connectionevent.h"

EVENTRPC_NAMESPACE_BEGIN

class EventPoller;
class RpcServerEvent;

class WorkerThread {
 public:
  WorkerThread(RpcServerEvent *server_event);

  ~WorkerThread();

  int Start();

  void PushNewConnection(int fd);

 private:
  struct Impl;
  Impl *impl_;
};

EVENTRPC_NAMESPACE_END

#endif  //  __EVENTRPC_WORKERTHREAD_H__
