#ifndef __EVENTRPC_WORKERTHREAD_H__
#define __EVENTRPC_WORKERTHREAD_H__

#include "base.h"
#include "connectionevent.h"

EVENTRPC_NAMESPACE_BEGIN

class EventPoller;
class RpcServerEvent;
class ConnectionEvent;

class WorkerThread {
 public:
  WorkerThread(RpcServerEvent *server_event);

  ~WorkerThread();

  bool Start(int cpuno);

  bool PushNewConnection(int fd);

  void PushUnusedConnection(ConnectionEvent *conn_event);

 private:
  struct Impl;
  Impl *impl_;
};

EVENTRPC_NAMESPACE_END

#endif  //  __EVENTRPC_WORKERTHREAD_H__
