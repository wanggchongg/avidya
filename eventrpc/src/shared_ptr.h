#ifndef __EVENTRPC_SHARED_PTR_H__
#define __EVENTRPC_SHARED_PTR_H__

#include "atomic.h"

template<class T>
class shared_ptr
{
  template<typename Y> friend class shared_ptr;
 public:
  typedef T value_type;
  typedef T * pointer;
  typedef T& reference;

  explicit shared_ptr(T* p = NULL)
    : ptr_(p) {
    counter_ = new atomic_t;
    *counter_ = ATOMIC_INIT(1);
  }

  shared_ptr& operator= (T* p) {
    if(this->ptr_ == p) {
      return *this;
    }
    dispose();
    counter_ = new atomic_t;
    *counter_ = ATOMIC_INIT(1);

    return *this;
  }

  shared_ptr(const shared_ptr& r)
    : ptr_(r.ptr_) {
    atomic_add(r.counter_);
    counter_ = r.counter_;
  }

  shared_ptr& operator= (const shared_ptr& r)  {
    if(this == &r) {
      return *this;
    }
    dispose();
    ptr_ = r.ptr_;
    atomic_add(r.counter_);
    counter_ = r.counter_;
    return *this;
  }

  template<typename Y>
  shared_ptr(const shared_ptr<Y>& r) {
    ptr_ = r.ptr_;
    atomic_add(r.counter_);
    counter_ = r.counter_;
  }

  template<typename Y>
  shared_ptr& operator= (const shared_ptr<Y>& r) {
    dispose();
    ptr_ = r.ptr_;
    atomic_add(r.counter_);
    counter_ = r.counter_;
    return *this;
  }

  template<typename Y>
  shared_ptr(Y* py) {
    counter_ = new atomic_t;
    *counter_ = ATOMIC_INIT(1);
    ptr_ = py;
  }

  template<typename Y>
  shared_ptr& operator= (Y* py) {
    if(this->ptr_ == py) {
      return *this;
    }
    dispose();
    counter_ = new atomic_t;
    *counter_ = ATOMIC_INIT(1);
    ptr_=py;
    return *this;
  }

  ~shared_ptr() {
    dispose();
  }

  void reset(T* p= NULL) {
    if ( ptr_ == p ) return;
    if (atomic_dec_and_test(counter_) == 0) {
      delete(ptr_);
    } else {
      counter_ = new atomic_t;
    }
    *counter_ = ATOMIC_INIT(1);
    ptr_ = p;
  }

  reference operator*() const {
    return *ptr_;
  }

  pointer operator->() const {
    return ptr_;
  }

  pointer get() const {
    return ptr_;
  }

  int32 use_count() const {
    return atomic_read(counter_);
  }

  bool unique() const  {
    return atomic_read(counter_) == 1;
  }

 private:
  void dispose() {
    if (atomic_dec_and_test(counter_) == 0) {
      delete ptr_;
      delete counter_;
    }
  }

  T *ptr_;
  atomic_t* counter_;
};

template<typename A,typename B>
inline bool operator==(shared_ptr<A> const & l, shared_ptr<B> const & r)
{
  return l.get() == r.get();
}

template<typename A,typename B>
inline bool operator!=(shared_ptr<A> const & l, shared_ptr<B> const & r)
{
  return l.get() != r.get();
}

#endif  // __EVENTRPC_SHARED_PTR_H__
