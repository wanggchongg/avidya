#ifndef __EVENTRPC_THREAD_H__
#define __EVENTRPC_THREAD_H__

#include <string>
#include <pthread.h>
#include <eventrpc/noncopyable.h>
#include <eventrpc/shared_ptr.h>

namespace eventrpc {

class Thread;

class Runnable {
 public:
  virtual ~Runnable() {};
  virtual void Run() = 0;

  void set_thread(Thread* thread) {
    thread_ = thread;
  }

 private:
  Thread* thread_;
};

class Thread {
 public:
  typedef unsigned long thread_id_t;

  Thread(const std::string &name, shared_ptr<Runnable> runnable);
  Thread(shared_ptr<Runnable> runnable);

  ~Thread();

  void Start();

  void Join();

  pthread_t thread_id() {
    return thread_id_;
  }

  void set_cpu_affinity(int cpu_affinity) {
    cpu_affinity_ = cpu_affinity;
  }

  void set_detach(bool detach) {
    detach_ = detach;
  }

  bool detach() const {
    return detach_;
  }

  // in MB unit
  void set_stack_size(int stack_size) {
    stack_size_ = stack_size;
  }

  int stack_size() const {
    return stack_size_;
  }

  void set_policy(int policy) {
    if (policy != SCHED_OTHER ||
        policy != SCHED_FIFO  ||
        policy != SCHED_RR) {
      policy_ = SCHED_FIFO;
    } else {
      policy_ = policy;
    }
  }

  int policy() const {
    return policy_;
  }

 private:
  static void* threadMain(void* arg);
  bool Init();

 private:
  std::string name_;
  shared_ptr<Runnable> runnable_;
  int cpu_affinity_;
  bool detach_;
  int stack_size_;
  int policy_;
  pthread_t thread_id_;

  DISALLOW_EVIL_CONSTRUCTOR(Thread);
};

};
#endif // __EVENTRPC_THREAD_H__
