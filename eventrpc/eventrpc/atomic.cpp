
#include "atomic.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t atomic_read(volatile uint32_t *mem) {
  return *mem;
}

void atomic_set(volatile uint32_t *mem, uint32_t val) {
  *mem = val;
}

uint32_t atomic_add(volatile uint32_t *mem, uint32_t val) {
  return __sync_add_and_fetch(mem, val);
}

uint32_t atomic_sub(volatile uint32_t *mem, uint32_t val) {
  return __sync_sub_and_fetch(mem, val);
}

uint32_t atomic_inc(volatile uint32_t *mem) {
  return __sync_add_and_fetch(mem, 1);
}

uint32_t atomic_dec(volatile uint32_t *mem) {
  return __sync_sub_and_fetch(mem, 1);
}

uint32_t atomic_cas(volatile uint32_t *mem,
                      uint32_t oldval,
                      uint32_t newval) {
  return __sync_val_compare_and_swap(mem, oldval, newval);
}

uint32_t atomic_xchg(volatile uint32_t *mem,
                       uint32_t val) {
  return __sync_val_compare_and_swap(mem, *mem, val);
}

extern void* atomic_casptr(volatile void **mem,
                           void *oldval,
                           const void *newval) {
  return __sync_val_compare_and_swap((void**)mem, oldval, newval);
}

void* atomic_xchgptr(volatile void **mem, void *val) {
  return __sync_val_compare_and_swap((void**)mem, *mem, val);
}

#ifdef __cplusplus
}
#endif
