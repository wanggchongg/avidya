#ifndef __EVENTRPC_REF_COUNT_IMPL_H__
#define __EVENTRPC_REF_COUNT_IMPL_H__

#include <eventrpc/ref_count.h>

namespace eventrpc {

template<typename T>
class RefCountImpl : public RefCount {
 public:
  typedef T* pointer;

 private:
  pointer ptr_;

 public:
  explicit RefCountImpl(pointer p)
    : RefCount(1, 1),
      ptr_(p) {
  }

  inline virtual void dispose() {
    delete ptr_;
  }
};
};

#endif  //  __EVENTRPC_REF_COUNT_IMPL_H__
