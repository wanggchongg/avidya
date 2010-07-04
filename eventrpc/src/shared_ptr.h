#ifndef __EVENTRPC_SHARED_PTR_H__
#define __EVENTRPC_SHARED_PTR_H__
#include <functional>
#include <algorithm>  // for swap
#include "atomic.h"

template <typename T> class shared_ptr;
template <typename T> class weak_ptr;
class SharedPtrControlBlock {
 private:
  template <typename T> friend class shared_ptr;
  template <typename T> friend class weak_ptr;
  SharedPtrControlBlock() {
      atomic_set(&ref_count_, 1);
      atomic_set(&weak_count_, 1);
  }

  atomic_t ref_count_;
  atomic_t weak_count_;
};

template <typename T>
class shared_ptr {
 public:
  typedef T element_type;
  explicit shared_ptr(T* ptr = NULL)
      : ptr_(ptr),
        control_block_(ptr != NULL ? new SharedPtrControlBlock : NULL) {
  }

  template <typename U>
  shared_ptr(const shared_ptr<U>& ptr)
      : ptr_(NULL),
        control_block_(NULL) {
    Initialize(ptr);
  }

  shared_ptr(const shared_ptr<T>& ptr)
      : ptr_(NULL),
        control_block_(NULL) {
    Initialize(ptr);
  }

  template <typename U>
  shared_ptr<T> & operator = (const shared_ptr<U>& ptr) {
    if (ptr_ != ptr.ptr_) {
      shared_ptr<T> me(ptr);
      swap(me);
    }
    return *this;
  }

  shared_ptr<T>& operator = (const shared_ptr<T>& ptr) {
    if (ptr_ != ptr.ptr_) {
      shared_ptr<T> me(ptr);
      swap(me);
    }
    return *this;
  }

  ~shared_ptr() {
    if (ptr_ != NULL) {
      if (atomic_sub_and_fetch(&(control_block_->ref_count_), 1) == 0) {
        delete ptr_;
        if (atomic_sub_and_fetch((&control_block_->weak_count_), 1) == 0) {
          delete control_block_;
        }
      }
    }
  }

  void reset(T* p = NULL) {
    if (p != ptr_) {
      shared_ptr<T> tmp(p);
      tmp.swap(*this);
    }
  }

  void swap(shared_ptr<T>& r) {
    std::swap(ptr_, r.ptr_);
    std::swap(control_block_, r.control_block_);
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
    return control_block_ ? atomic_read(&(control_block_->ref_count_)) : 1;
  }

  bool unique() const {
    return use_count() == 1;
  }

  const weak_ptr<T> weaker() const {
    return weak_ptr<T>(*this);
  }

  weak_ptr<T> weaker() {
    return weak_ptr<T>(*this);
  }
 private:
  template <typename U>
  void Initialize(const shared_ptr<U> &r) {
    if (r.control_block_ != NULL) {
      atomic_add_and_fetch(&(r.control_block_->ref_count_), 1);
      ptr_ = r.ptr_;
      control_block_ = r.control_block_;
    }
  }

  T* ptr_;
  SharedPtrControlBlock* control_block_;
  template <typename U> friend class shared_ptr;
  template <typename U> friend class weak_ptr;
};

// Matches the interface of std::swap as an aid to generic programming.
template <typename T> void swap(
    shared_ptr<T>& r, shared_ptr<T>& s) {
  r.swap(s);
}

template <typename T>
class weak_ptr {
  template <typename U> friend class weak_ptr;
 public:
  weak_ptr()
    : ptr_(NULL),
    control_block_(NULL) {
  }

  template <typename U> weak_ptr(const shared_ptr<U>& ptr) {
    CopyFrom(ptr.ptr_, ptr.control_block_);
  }

  template <typename U> weak_ptr(const weak_ptr<U> &ptr) {
    CopyFrom(ptr.ptr_, ptr.control_block_);
  }

  weak_ptr(const weak_ptr& ptr) {
    CopyFrom(ptr.ptr_, ptr.control_block_);
  }

  ~weak_ptr() {
    if (control_block_ != NULL &&
        atomic_sub_and_fetch(&(control_block_->weak_count_), 1) == 0) {
      delete control_block_;
    }
  }

  weak_ptr& operator=(const weak_ptr& ptr) {
    if (&ptr != this) {
      weak_ptr tmp(ptr);
      tmp.swap(*this);
    }
    return *this;
  }

  template <typename U> weak_ptr& operator=(
      const weak_ptr<U>& ptr) {
    weak_ptr tmp(ptr);
    tmp.swap(*this);
    return *this;
  }

  template <typename U> weak_ptr& operator=(
      const shared_ptr<U> &ptr) {
    weak_ptr tmp(ptr);
    tmp.swap(*this);
    return *this;
  }

  void swap(weak_ptr& ptr) {
    std::swap(ptr_, ptr.ptr_);
    std::swap(control_block_, ptr.control_block_);
  }

  void reset() {
    weak_ptr tmp;
    tmp.swap(*this);
  }

  int use_count() const {
    return control_block_ ? atomic_read(&(control_block_->ref_count_)) : 0;
  }

  bool expired() const {
    return use_count() == 0;
  }

  shared_ptr<T> lock() const {
    shared_ptr<T> result;
    if (control_block_ != NULL) {
      int old_refcount;
      do {
        old_refcount = control_block_->ref_count_;
        if (old_refcount == 0) {
          break;
        }
      } while (!atomic_bool_compare_and_swap(
          &control_block_->ref_count_, old_refcount,
          old_refcount + 1));
      if (old_refcount > 0) {
        result.ptr_ = ptr_;
        result.control_block_ = control_block_;
      }
    }
    return result;
  }

 private:
  void CopyFrom(T* ptr, SharedPtrControlBlock* control_block) {
    ptr_ = ptr;
    control_block_ = control_block;
    if (control_block_ != NULL) {
      atomic_add_and_fetch(&control_block_->weak_count_, 1);
    }
  }

  T* ptr_;
  SharedPtrControlBlock *control_block_;
};

template <typename T> void swap(weak_ptr<T> &r,
                                weak_ptr<T> &s) {
  r.swap(s);
}
#endif  // __EVENTRPC_SHARED_PTR_H__
