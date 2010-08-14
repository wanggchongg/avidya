
#include <pthread.h>
#include "concurrency/mutex.h"

EVENTRPC_NAMESPACE_BEGIN

class Mutex::Impl {
 public:
  Impl() {
    pthread_mutex_init(&mutex_, NULL);
  }

  ~Impl() {
    pthread_mutex_destroy(&mutex_);
  }

  bool Lock() {
    return pthread_mutex_lock(&mutex_) == 0 ? true : false;
  }

  bool UnLock() {
    return pthread_mutex_unlock(&mutex_) == 0 ? true : false;
  }

 private:
  pthread_mutex_t mutex_;
};

Mutex::Mutex()
  : impl_(new Impl) {
}

Mutex::~Mutex() {
  delete impl_;
}

bool Mutex::Lock() {
  return impl_->Lock();
}

bool Mutex::UnLock() {
  return impl_->UnLock();
}

EVENTRPC_NAMESPACE_END
