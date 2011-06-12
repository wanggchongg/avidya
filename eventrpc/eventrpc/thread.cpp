#include <pthread.h>
#include "thread.h"

namespace eventrpc {
Thread::Thread(ThreadWorker *thread_worker)
  : thread_worker_(thread_worker) {

}

Thread::~Thread() {
}

void Thread::Start() {
  pthread_attr_t thread_attr;

  if (pthread_attr_init(&thread_attr) != 0) {
    return;
  }

  if (pthread_create(&pthread_, &thread_attr,
                     ThreadMain, this) != 0) {
    return;
  }
}

void* Thread::ThreadMain(void* arg) {
  Thread *thread = static_cast<Thread*>(arg);
  thread->thread_worker_->Run();
  return NULL;
}
};
