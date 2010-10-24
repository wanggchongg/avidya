
#include <stdio.h>
#include <sys/time.h>
#include "util/time_utility.h"

EVENTRPC_NAMESPACE_BEGIN

static int64 CycleClock_Now() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return static_cast<int64>(tv.tv_sec) * 1000000 + tv.tv_usec;
}

static int64 UsecToCycles(int64 usec) {
  return usec;
}

WallTime WallTime_Now() {
  return CycleClock_Now() * 0.000001;
}

EVENTRPC_NAMESPACE_END
