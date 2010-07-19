#ifndef __REF_COUNT_PTHREAD_H__
#define __REF_COUNT_PTHREAD_H__

#include <pthread.h>

template <typename T> class shared_ptr;
template <typename T> class weak_ptr;

class RefCount {
  template <typename T> friend class shared_ptr;
  template <typename T> friend class weak_ptr;

 public:
  RefCount(unsigned int use_count = 1, unsigned int weak_count = 1);

  virtual ~RefCount();

  bool inc_ref();

  void dec_ref();

  void inc_weak_ref();

  void dec_weak_ref();

  virtual void dispose( ) = 0;

  virtual void destroy( ) ;

 private:
  volatile unsigned int use_count_;
  volatile unsigned int weak_count_;
  pthread_mutex_t mutex_;
};

#endif  //  __REF_COUNT_PTHREAD_H__
