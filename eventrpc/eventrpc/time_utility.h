#ifndef __EVENTRPC_TIME_UTILITY_H__
#define __EVENTRPC_TIME_UTILITY_H__
#include "base.h"
namespace eventrpc {
class TimeUtility {
 private:
  static const int64_t NS_PER_S = 1000000000LL;
  static const int64_t US_PER_S = 1000000LL;
  static const int64_t MS_PER_S = 1000LL;

  static const int64_t NS_PER_MS = NS_PER_S / MS_PER_S;
  static const int64_t US_PER_MS = US_PER_S / MS_PER_S;
 public:
  static int64 CurrentTime();

  static void inline MakeTimespec(int64 ms, struct timespec *result) {
    result->tv_sec = ms / MS_PER_S; // ms to s
    result->tv_nsec = (ms % MS_PER_S) * NS_PER_MS; // ms to ns
  }

  static void inline ConvertTimevalToMS(const struct timeval& value,
                                        int64 *result) {
    *result = (value.tv_sec * MS_PER_S) + (value.tv_usec / US_PER_MS);
    if ((value.tv_usec % US_PER_MS) >= (US_PER_MS / 2)) {
      ++(*result);
    }
  }
};
};
#endif  // __EVENTRPC_TIME_UTILITY_H__
