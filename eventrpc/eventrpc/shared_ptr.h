
#ifndef __EVENTRPC_SHARED_PTR_H__
#define __EVENTRPC_SHARED_PTR_H__

#include <algorithm>  // for swap
#include <eventrpc/ref_count_impl.h>
#include <eventrpc/weak_ptr.h>
#include <eventrpc/base.h>

namespace eventrpc {

template <typename T>
class shared_ptr {
 private:
  template <typename U> friend class weak_ptr;

 public:
  shared_ptr(T* ptr = NULL)
    : ptr_(ptr),
      ref_count_(ptr != NULL ? new RefCountImpl<T>(ptr) : NULL) {
  }

  shared_ptr(const shared_ptr<T> &sp)
    : ptr_(NULL) {
      Init(sp);
  }

  shared_ptr(const weak_ptr<T> &wp)
    : ptr_(wp.ptr_) {
    if(ref_count_ != NULL && !ref_count_->inc_ref() ) {
      ref_count_ = NULL;
    }
  }

  shared_ptr<T>& operator=(const shared_ptr<T> &sp) {
    if (sp.ptr_ != ptr_) {
      shared_ptr<T>(sp).swap(*this);
    }
    return *this;
  }

  ~shared_ptr() {
    if (ref_count_) {
      ref_count_->dec_ref();
    }
  }

  void reset(T* p = NULL) {
    if (p != ptr_) {
      shared_ptr<T> tmp(p);
      tmp.swap(*this);
    }
  }

  T* get() const {
    return ptr_;
  }

  T& operator*() {
    return *ptr_;
  }

  T* operator->() const {
    return ptr_;
  }

  int use_count() const {
    return ref_count_ ? ref_count_->use_count_ : 1;
  }

  bool unique() const {
    return ref_count_->use_count_ == 1;
  }

  const weak_ptr<T> weaker() const {
    return weak_ptr<T>(*this);
  }

  weak_ptr<T> weaker() {
    return weak_ptr<T>(*this);
  }

 private:
  void swap(shared_ptr<T>& sp) {
    std::swap(ptr_, sp.ptr_);
    std::swap(ref_count_, sp.ref_count_);
  }

  template <typename U>
  void Init(const shared_ptr<U> &sp) {
    if (sp.ref_count_ != NULL) {
      sp.ref_count_->inc_ref();
      this->ptr_ = sp.ptr_;
      this->ref_count_ = sp.ref_count_;
    }
  }

 private:
  T* ptr_;
  RefCount *ref_count_;
};
};

#endif  // __EVENTRPC_SHARED_PTR_H__
