#ifndef __EVENTRPC_ATOMIC_H__
#define __EVENTRPC_ATOMIC_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t atomic_read(volatile uint32_t *mem);

extern void atomic_set(volatile uint32_t *mem, uint32_t val);

extern uint32_t atomic_add(volatile uint32_t *mem, uint32_t val);

extern uint32_t atomic_sub(volatile uint32_t *mem, uint32_t val);

extern uint32_t atomic_inc(volatile uint32_t *mem);

extern uint32_t atomic_dec(volatile uint32_t *mem);

extern uint32_t atomic_cas(volatile uint32_t *mem,
                           uint32_t oldval,
                           uint32_t newval);

extern uint32_t atomic_xchg(volatile uint32_t *mem,
                            uint32_t val);

extern void* atomic_casptr(volatile void **mem,
                           void *oldval,
                           const void *newval);

extern void* atomic_xchgptr(volatile void **mem,
                            void *val);

#ifdef __cplusplus
}
#endif

#endif // __EVENTRPC_ATOMIC_H__
