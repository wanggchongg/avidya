
#ifndef __EVENTRPC_MONITOR_H__
#define __EVENTRPC_MONITOR_H__

#include <stdint.h>
#include <pthread.h>
#include <eventrpc/noncopyable.h>

namespace eventrpc {

class Monitor {
 public:
  Monitor();

  ~Monitor();

  void Lock() const;

  void Unlock() const;

  bool TimeWait(uint32_t timeout_ms) const;

  void Wait() const;

  void Notify() const;

  void NotifyAll() const;

 private:
  mutable pthread_mutex_t pthread_mutex_;
  mutable pthread_cond_t pthread_cond_;

  DISALLOW_EVIL_CONSTRUCTOR(Monitor);
};

};

#endif  //  __EVENTRPC_MONITOR_H__
