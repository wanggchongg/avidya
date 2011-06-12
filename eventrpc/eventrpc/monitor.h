/*
 * Copyright(C) lichuang
 */
#ifndef __EVENTRPC_MONITOR_H__
#define __EVENTRPC_MONITOR_H__
#include <pthread.h>
#include "eventrpc/base.h"
namespace eventrpc {
class Monitor {
 public:
  Monitor();

  ~Monitor();

  void TimeWait(int64 timeout_ms) const;

  void Wait() const;

  void Notify() const;

  void NotifyAll() const;

 private:
  mutable pthread_mutex_t pthread_mutex_;
  mutable pthread_cond_t pthread_cond_;
};
};
#endif  // __EVENTRPC_MONITOR_H__
