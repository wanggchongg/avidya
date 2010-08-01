
#ifdef USE_PTHREAD
#include "ref_count_pt.h"

RefCount::RefCount(unsigned int use_count, unsigned int weak_count)
  : use_count_(use_count),
    weak_count_(weak_count) {
  ::pthread_mutex_init(&mutex_, NULL);
}

RefCount::~RefCount() {
  ::pthread_mutex_destroy(&mutex_);
}

bool RefCount::inc_ref() {
  ::pthread_mutex_lock(&mutex_);
  bool ret = (use_count_ == 0) ? false : (++use_count_, true);
  ::pthread_mutex_unlock(&mutex_);

  return ret;
}

void RefCount::dec_ref() {
  ::pthread_mutex_lock(&mutex_);
  unsigned int new_use_count = --use_count_;
  ::pthread_mutex_unlock(&mutex_);

  if (new_use_count == 0) {
    dispose();
    dec_weak_ref();
  }
}

void RefCount::dec_weak_ref() {
  ::pthread_mutex_lock(&mutex_);
  unsigned int new_weak_count = --weak_count_;
  ::pthread_mutex_unlock(&mutex_);

  if (new_weak_count == 0) {
    destroy();
  }
}

void RefCount::inc_weak_ref() {
  ::pthread_mutex_lock(&mutex_);
  ++weak_count_;
  ::pthread_mutex_unlock(&mutex_);
}

void RefCount::destroy() {
  delete this;
}
#endif
