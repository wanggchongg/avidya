/*
 * Copyright(C) lichuang
 */
#include "eventrpc/log.h"
#include "eventrpc/monitor.h"
#include "eventrpc/time_utility.h"
namespace eventrpc {
Monitor::Monitor() {
  LOG_FATAL_IF(pthread_mutex_init(&pthread_mutex_, NULL) != 0);
  LOG_FATAL_IF(pthread_cond_init(&pthread_cond_, NULL) != 0);
}

Monitor::~Monitor() {
  pthread_mutex_destroy(&pthread_mutex_);
  pthread_cond_destroy(&pthread_cond_);
}

void Monitor::Wait() const {
  pthread_cond_wait(&pthread_cond_, &pthread_mutex_);
}

void Monitor::TimeWait(int64 timeout_ms) const {
  ASSERT(timeout_ms > 0LL) << "timeout_ms must greater than 0"
    << ", if timeout equal to 0, use Wait() instead";
  struct timespec time_spec;
  int64 now_ms = TimeUtility::CurrentTime();
  TimeUtility::MakeTimespec(now_ms + timeout_ms, &time_spec);
  pthread_cond_timedwait(&pthread_cond_,
                         &pthread_mutex_,
                         &time_spec);
}

void Monitor::Notify() const {
  pthread_cond_signal(&pthread_cond_);
}

void Monitor::NotifyAll() const {
  pthread_cond_broadcast(&pthread_cond_);
}
};
