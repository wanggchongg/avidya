
#include "concurrency/posixthread.h"

EVENTRPC_NAMESPACE_BEGIN

class PthreadThread: public Thread {
 public:

  enum STATE {
    uninitialized,
    starting,
    started,
    stopping,
    stopped
  };

  static const int MB = 1024 * 1024;

  static void* threadMain(void* arg);

 private:
  pthread_t pthread_;
  STATE state_;
  int policy_;
  int priority_;
  int stackSize_;
  weak_ptr<PthreadThread> self_;
  bool detached_;

 public:

  PthreadThread(int policy, int priority, int stackSize, bool detached, shared_ptr<Runnable> runnable) :
    pthread_(0),
    state_(uninitialized),
    policy_(policy),
    priority_(priority),
    stackSize_(stackSize),
    detached_(detached) {

      this->Thread::runnable(runnable);
    }
};

EVENTRPC_NAMESPACE_END
