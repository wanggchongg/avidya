#ifndef __EVENTRPC_REF_COUNT_H__
#define __EVENTRPC_REF_COUNT_H__

#include <stdint.h>

template <typename T> class shared_ptr;
template <typename T> class weak_ptr;

class RefCount {
  template <typename T> friend class shared_ptr;
  template <typename T> friend class weak_ptr;

 public:
  RefCount(uint32_t use_count = 1, uint32_t weak_count = 1);

  virtual ~RefCount();

  bool inc_ref();

  void dec_ref();

  void inc_weak_ref();

  void dec_weak_ref();

  virtual void dispose( ) = 0;

  virtual void destroy( ) ;

 private:
  volatile uint32_t use_count_;
  volatile uint32_t weak_count_;
};

#endif  //  __EVENTRPC_REF_COUNT_H__
