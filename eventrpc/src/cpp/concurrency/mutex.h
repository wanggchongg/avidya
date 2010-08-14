#ifndef __EVENTRPC_MUTEX_H__
#define __EVENTRPC_MUTEX_H__

#include "base/base.h"

EVENTRPC_NAMESPACE_BEGIN

class Mutex {
 public:
  Mutex();

  ~Mutex();

  bool Lock();

  bool UnLock();

 private:
  struct Impl;
  Impl *impl_;
};

class MutexLock {
 public:
  MutexLock(Mutex *mutex)
    : mutex_(mutex) {
    mutex_->Lock();
  }

  ~MutexLock() {
    mutex_->UnLock();
  }

 private:
  Mutex * mutex_;
};

EVENTRPC_NAMESPACE_END

#endif  // __EVENTRPC_MUTEX_H__
