#ifndef __EVENTRPC_MUTEX_H__
#define __EVENTRPC_MUTEX_H__

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

#endif  // __EVENTRPC_MUTEX_H__
