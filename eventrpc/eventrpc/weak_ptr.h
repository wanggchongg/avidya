
#ifndef __WEAK_PTR_H__
#define __WEAK_PTR_H__

#include <eventrpc/shared_ptr.h>
#include <eventrpc/ref_count_impl.h>

namespace eventrpc {

template <typename T> class shared_ptr;

template <typename T>
class weak_ptr {
  template <typename U> friend class shared_ptr;

 private:
  typedef T* pointer;

 public:
  weak_ptr()
    : ptr_(NULL),
      ref_count_(NULL) {
  }

  weak_ptr(shared_ptr<T> sp) {
    Init(sp.ptr_, sp.ref_count_);
  }

  weak_ptr(const weak_ptr<T> &sp) {
    Init(sp.ptr_, sp.ref_count_);
  }

  weak_ptr<T>& operator=(const weak_ptr<T>& sp) {
    weak_ptr tmp(sp);
    tmp.swap(*this);
    return *this;
  }

  weak_ptr<T>& operator=(const shared_ptr<T> &sp) {
    weak_ptr tmp(sp);
    tmp.swap(*this);
    return *this;
  }

  void reset() {
    weak_ptr tmp;
    tmp.swap(*this);
  }

  int use_count() const {
    return ref_count_ != NULL ? ref_count_->use_count_ : 0;
  }

  bool expired() const {
    return ref_count_->use_count_ == 0;
  }

  shared_ptr<T> lock() const {
    return shared_ptr<T>(*this);
  }

  void swap(weak_ptr& ptr) {
    std::swap(ptr_, ptr.ptr_);
    std::swap(ref_count_, ptr.ref_count_);
  }

 private:
  void Init(T *ptr, RefCount *ref_count) {
    ptr_ = ptr;
    ref_count_ = ref_count;
    if (ref_count_) {
      ref_count_->inc_weak_ref();
    }
  }

 private:
  pointer ptr_;
  RefCount *ref_count_;
};
};

#endif  //  __WEAK_PTR_H__
