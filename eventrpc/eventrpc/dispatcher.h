/*
 * Copyright(C) lichuang
 */
#ifndef __EVENTRPC_DISPATCHER_H__
#define __EVENTRPC_DISPATCHER_H__
#include "eventrpc/event.h"
#include "eventrpc/task.h"
namespace eventrpc {
// thread-binding dispather implementation
class Dispatcher {
 public:
  Dispatcher();

  ~Dispatcher();

  void AddEvent(int fd, uint32 flags, EventHandler *handler);

  void PushTask(Task *task);

  void Start();

  void Stop();

  struct Impl;
 private:
  Impl *impl_;
};
};
#endif  //  __EVENTRPC_DISPATCHER_H__
