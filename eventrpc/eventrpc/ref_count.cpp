
#include "ref_count.h"
#include "atomic.h"

namespace eventrpc {
RefCount::RefCount(unsigned int use_count, unsigned int weak_count)
  : use_count_(use_count),
    weak_count_(weak_count) {
}

RefCount::~RefCount() {
}

bool RefCount::inc_ref() {
  return atomic_inc(&use_count_);
}

void RefCount::dec_ref() {
  if (atomic_dec(&use_count_) == 0) {
    dispose();
    dec_weak_ref();
  }
}

void RefCount::dec_weak_ref() {
  if (atomic_dec(&weak_count_) == 0) {
    destroy();
  }
}

void RefCount::inc_weak_ref() {
  atomic_inc(&weak_count_);
}

void RefCount::destroy() {
  delete this;
}
};
