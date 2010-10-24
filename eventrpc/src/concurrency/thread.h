#ifndef __EVENTRPC_THREAD_H__
#define __EVENTRPC_THREAD_H__

#include <smart_ptr/shared_ptr.h>
#include <base/base.h>

EVENTRPC_NAMESPACE_BEGIN

class Thread;

class Runnable {
 public:
  virtual ~Runnable() {};
  virtual void Run() = 0;

  virtual shared_ptr<Thread> thread() { return thread_.lock(); }

  virtual void set_thread(shared_ptr<Thread> thread) { thread_ = thread; }

 private:
  weak_ptr<Thread> thread_;
};

class Thread {
 public:
  typedef unsigned long id_t;

  virtual ~Thread() {};

  virtual void Start() = 0;

  virtual void Join() = 0;

  virtual id_t id() = 0;

  virtual shared_ptr<Runnable> runnable() const { return runnable_; }

  void set_cpu(int cpu_num) {
    cpu_num_ = cpu_num;
  }

 protected:
  virtual void runnable(shared_ptr<Runnable> runnable) { runnable_ = runnable; }

  shared_ptr<Runnable> runnable_;
  int cpu_num_;
};

class ThreadFactory {
 public:
  virtual ~ThreadFactory() {}
  virtual shared_ptr<Thread> newThread(shared_ptr<Runnable> runnable) const = 0;
};


EVENTRPC_NAMESPACE_END

#endif // __EVENTRPC_THREAD_H__
