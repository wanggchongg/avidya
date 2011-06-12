/*
 * Copyright(C) lichuang
 */
#ifndef __EVENTRPC_ASSERT_LOG_H__
#define __EVENTRPC_ASSERT_LOG_H__
#include "eventrpc/log.h"
namespace eventrpc {
// ASSERT condition macros
#ifdef NDEBUG
// define a null stream that actually has no effect
// cause ASSERT marco has no effect in release version
#define NULL_STREAM                             \
  if (0) std::ostringstream()

#define ASSERT(condition)                NULL_STREAM

#define ASSERT_EQ(expected, actual)      NULL_STREAM
#define ASSERT_NE(expected, actual)      NULL_STREAM
#define ASSERT_LE(expected, actual)      NULL_STREAM
#define ASSERT_LT(expected, actual)      NULL_STREAM
#define ASSERT_GE(expected, actual)      NULL_STREAM
#define ASSERT_GT(expected, actual)      NULL_STREAM
#else   // ifdef NDEBUG
#define ASSERT(condition)                                \
 if (!(condition))                                      \
   LOG_FATAL() << "ASSERT failed: " #condition "\n"

#define DEFINE_ASSERT_OP_IMPL(name, op)                  \
template<class t1, class t2>                            \
inline bool                                             \
ASSERT##name##impl(const t1& v1, const t2 &v2) {         \
  return (v1 op v2);                                    \
}

DEFINE_ASSERT_OP_IMPL(EQ, ==)
DEFINE_ASSERT_OP_IMPL(NE, !=)
DEFINE_ASSERT_OP_IMPL(LE, <=)
DEFINE_ASSERT_OP_IMPL(LT, <)
DEFINE_ASSERT_OP_IMPL(GE, >=)
DEFINE_ASSERT_OP_IMPL(GT, >)
#undef DEFINE_ASSERT_OP_IMPL

#define ASSERT_OP(name, op, val1, val2)                  \
  if (!eventrpc::ASSERT##name##impl(val1, val2))         \
    LOG_FATAL() << "ASSERT failed: "                     \
      << #val1 " " #op " " #val2                        \
      << "(" #val1 << " vs. " << #val2 ")\n"

#define ASSERT_EQ(expected, actual)                      \
  ASSERT_OP(EQ, ==, (expected), actual)
#define ASSERT_NE(expected, actual)                      \
  ASSERT_OP(NE, !=, expected, actual)
#define ASSERT_TRUE(actual)                             \
  ASSERT_EQ(true, (actual))
#define ASSERT_FALSE(actual)                            \
  ASSERT_NE(false, (actual))
#define ASSERT_LE(expected, actual)                      \
  ASSERT_OP(LE, <=, expected, actual)
#define ASSERT_LT(expected, actual)                      \
  ASSERT_OP(LT, <, expected, actual)
#define ASSERT_GE(expected, actual)                      \
  ASSERT_OP(GE, >=, expected, actual)
#define ASSERT_GT(expected, actual)                      \
  ASSERT_OP(GT, >, expected, actual)
#endif // ifndef NDEBUG
};
#endif  // __EVENTRPC_ASSERT_LOG_H__
