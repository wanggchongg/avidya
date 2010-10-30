
#ifndef __EVENTRPC_TIME_UTIL_H__
#define __EVENTRPC_TIME_UTIL_H__

#include <time.h>
#include <sys/time.h>
#include <stdint.h>

namespace eventrpc {
class TimeUtil {

 public:
  static void MakeTimespec(timespec *result, uint64_t value);

  static void MakeTimeval(timeval *result, uint64_t value);

  static void MakeTimeSpecToMs(int64_t *result,
                               const struct timespec& value);

  static void MakeTimeValToMs(int64_t *result,
                              const struct timeval& value);

  static const int64_t GetCurrentTime();
};
};
#endif  // __EVENTRPC_TIME_UTIL_H__
