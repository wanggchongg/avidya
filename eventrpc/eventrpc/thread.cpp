
#include "thread.h"
#include "log.h"

namespace eventrpc {

static const int MB = 1024 * 1024;

Thread::Thread(const std::string &name, Runnable *runnable)
  : name_(name),
    runnable_(runnable),
    cpu_affinity_(0),
    detach_(true),
    stack_size_(1),
    policy_(SCHED_RR) {
}

Thread::Thread(Runnable *runnable)
  : name_("Anonymous Thread"),
    runnable_(runnable),
    cpu_affinity_(0),
    detach_(true),
    stack_size_(1),
    policy_(SCHED_RR) {
}

bool Thread::Init() {
  pthread_attr_t thread_attr;
  if (pthread_attr_init(&thread_attr) != 0) {
    LOG_ERROR() << "pthread_attr_init error";
    return false;
  }

  if(pthread_attr_setdetachstate(
      &thread_attr,
      detach_ ?  PTHREAD_CREATE_DETACHED : PTHREAD_CREATE_JOINABLE) != 0) {
    LOG_ERROR() << "pthread_attr_setdetachstate error";
    return false;
  }

  if (pthread_attr_setstacksize(&thread_attr,
                                MB * stack_size_) != 0) {
    LOG_ERROR() << "pthread_attr_setstacksize error";
    return false;
  }

  if (pthread_attr_setschedpolicy(&thread_attr, policy_) != 0) {
    LOG_ERROR() << "pthread_attr_setschedpolicy error";
    return false;
  }

  /*
  struct sched_param sched_param;
  sched_param.sched_priority = priority_;

  if (pthread_attr_setschedparam(&thread_attr, &sched_param) != 0) {
    LOG_ERROR() << "pthread_attr_setschedparam error";
    return false;
  }
  */

  if (pthread_create(&thread_id_, &thread_attr,
                     &Thread::threadMain, (void*)this) != 0) {
    LOG_ERROR() << "pthread_create error";
    return false;
  }


  return true;
}

void * Thread::threadMain(void *arg) {
  Thread *self = static_cast<Thread*>(arg);

  int cpu_affinity = self->cpu_affinity_;
  if (cpu_affinity >= 0) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu_affinity, &mask);
    int ret = sched_setaffinity(0, sizeof(mask), &mask);
    LOG_INFO() << "SetAffinity to: " << cpu_affinity
      << " ret: " << ret;
  }
  self->runnable_->Run();
  return NULL;
}

void Thread::Start() {
  if (!Init()) {
    return;
  }
}

void Thread::Join() {
  if (!detach_) {
    detach_ = pthread_join(thread_id_, NULL) == 0;
  }
}

};
