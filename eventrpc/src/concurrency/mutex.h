#ifndef __EVENTRPC_MUTEX_H__
#define __EVENTRPC_MUTEX_H__

#include <pthread.h>
#include <stdlib.h>
#include <base/base.h>

typedef pthread_rwlock_t MutexType;

EVENTRPC_NAMESPACE_BEGIN

class Mutex {
 public:
  inline Mutex();

  inline ~Mutex();

  inline void Lock();

  inline void Unlock();

  inline void ReaderLock();

  inline void ReaderUnlock();

  inline void WriterLock();

  inline void WriterUnlock();

 private:
  MutexType mutex_;
};

#define SAFE_PTHREAD_CALL(fn)           \
do {                                    \
  if (fn(&mutex_) != 0) abort();        \
} while(0)

Mutex::Mutex() {
  if (pthread_rwlock_init(&mutex_, NULL) != 0) {
    abort();
  }
}

Mutex::~Mutex() {
  SAFE_PTHREAD_CALL(pthread_rwlock_destroy);
}

void Mutex::Lock() {
  SAFE_PTHREAD_CALL(pthread_rwlock_wrlock);
}

void Mutex::Unlock() {
  SAFE_PTHREAD_CALL(pthread_rwlock_unlock);
}

void Mutex::ReaderLock() {
  SAFE_PTHREAD_CALL(pthread_rwlock_rdlock);
}

void Mutex::ReaderUnlock() {
  SAFE_PTHREAD_CALL(pthread_rwlock_unlock);
}

void Mutex::WriterLock() {
  Lock();
}

void Mutex::WriterUnlock() {
  Unlock();
}

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

EVENTRPC_NAMESPACE_END

#endif  // __EVENTRPC_MUTEX_H__
