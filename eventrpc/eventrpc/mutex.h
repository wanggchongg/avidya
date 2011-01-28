#ifndef __EVENTRPC_MUTEX_H__
#define __EVENTRPC_MUTEX_H__

#include <pthread.h>
#include <stdlib.h>
#include "noncopyable.h"

namespace eventrpc {

class Mutex {
 public:
  Mutex() {
    if (pthread_mutex_init(&pthread_mutex_, NULL) != 0) {
      abort();
    }
  }

  ~Mutex() {
    if (pthread_mutex_destroy(&pthread_mutex_) != 0) {
      abort();
    }
  }

  void Lock() const {
    pthread_mutex_lock(&pthread_mutex_);
  }

  bool Trylock() const {
    return (pthread_mutex_trylock(&pthread_mutex_) == 0);
  }

  void Unlock() const {
    pthread_mutex_unlock(&pthread_mutex_);
  }

 private:
  mutable pthread_mutex_t pthread_mutex_;
  DISALLOW_EVIL_CONSTRUCTOR(Mutex);
};

class MutexLock {
 public:
  MutexLock(Mutex *mutex)
    : mutex_(mutex) {
    mutex_->Lock();
  }

  ~MutexLock() {
    mutex_->Unlock();
  }

 private:
  Mutex *mutex_;
};

class SpinMutex {
 public:
  SpinMutex() {
    if (pthread_spin_init(&pthread_spin_mutex_, NULL) != 0) {
      abort();
    }
  }

  ~SpinMutex() {
    if (pthread_spin_destroy(&pthread_spin_mutex_) != 0) {
      abort();
    }
  }

  void Lock() const {
    pthread_spin_lock(&pthread_spin_mutex_);
  }

  bool Trylock() const {
    return (pthread_spin_trylock(&pthread_spin_mutex_) == 0);
  }

  void Unlock() const {
    pthread_spin_unlock(&pthread_spin_mutex_);
  }

 private:
  mutable pthread_spinlock_t pthread_spin_mutex_;
  DISALLOW_EVIL_CONSTRUCTOR(SpinMutex);
};

class SpinMutexLock {
 public:
  SpinMutexLock(SpinMutex *mutex)
    : mutex_(mutex) {
    mutex_->Lock();
  }

  ~SpinMutexLock() {
    mutex_->Unlock();
  }

 private:
  SpinMutex *mutex_;
};

};

#endif  // __EVENTRPC_MUTEX_H__
