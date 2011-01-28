
#include <stdio.h>
#include <sys/time.h>
#include "time_utility.h"

namespace eventrpc {
int64 TimeUtility::CurrentTime() {
  int64 result = 0;
  struct timeval now;
  gettimeofday(&now, NULL);
  ConvertTimevalToMS(now, &result);
  return result;
}
};
