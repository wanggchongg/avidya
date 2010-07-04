#ifndef __EVENTRPC_ATOMIC_H__
#define __EVENTRPC_ATOMIC_H__

#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
  volatile int counter;
} atomic_t;

#define ATOMIC_INIT(i)  { (i) }

#define atomic_read(v) ((v)->counter)

#define atomic_set(v,i) (((v)->counter) = (i))

static inline void atomic_add(int i, atomic_t *v)
{
  (void)__sync_add_and_fetch(&v->counter, i);
}

static inline void atomic_sub(int i, atomic_t *v)
{
  (void)__sync_sub_and_fetch(&v->counter, i);
}

static inline int atomic_sub_and_test(int i, atomic_t *v)
{
  return !(__sync_sub_and_fetch(&v->counter, i));
}

static inline void atomic_inc(atomic_t *v)
{
  (void)__sync_fetch_and_add(&v->counter, 1);
}

static inline void atomic_dec(atomic_t *v)
{
  (void)__sync_fetch_and_sub(&v->counter, 1);
}

static inline int atomic_dec_and_test(atomic_t *v)
{
  return !(__sync_sub_and_fetch(&v->counter, 1));
}

static inline int atomic_inc_and_test(atomic_t *v)
{
  return !(__sync_add_and_fetch(&v->counter, 1));
}

static inline int atomic_add_negative(int i, atomic_t *v)
{
  return (__sync_add_and_fetch(&v->counter, i) < 0);
}

#ifdef __cplusplus
};
#endif

#endif // __EVENTRPC_ATOMIC_H__
