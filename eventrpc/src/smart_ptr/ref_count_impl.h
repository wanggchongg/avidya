#ifndef __RefCountImpl_H__
#define __RefCountImpl_H__

#include "ref_count_pt.h"

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

#endif  //  __RefCountImpl_H__
