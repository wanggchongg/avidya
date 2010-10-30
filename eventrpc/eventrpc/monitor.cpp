
#include <stdlib.h>
#include "monitor.h"
#include "log.h"
#include "time_util.h"

namespace eventrpc {

Monitor::Monitor() {
  ASSERT_EQ(0, pthread_mutex_init(&pthread_mutex_, NULL));
  ASSERT_EQ(0, pthread_cond_init(&pthread_cond_, NULL));
}

Monitor::~Monitor() {
  ASSERT_EQ(0, pthread_mutex_destroy(&pthread_mutex_));
  ASSERT_EQ(0, pthread_cond_destroy(&pthread_cond_));
}

void Monitor::Lock() const {
  pthread_mutex_lock(&pthread_mutex_);
}

void Monitor::Unlock() const {
  pthread_mutex_unlock(&pthread_mutex_);
}

bool Monitor::TimeWait(uint32_t timeout_ms) const {
  struct timespec abstime;
  int64_t now = TimeUtil::GetCurrentTime();
  TimeUtil::MakeTimespec(&abstime, now + timeout_ms);
  return pthread_cond_timedwait(&pthread_cond_,
                                &pthread_mutex_,
                                &abstime) == 0;
}

void Monitor::Notify() const {
  pthread_cond_signal(&pthread_cond_);
}

void Monitor::NotifyAll() const {
  pthread_cond_broadcast(&pthread_cond_);
}
};
