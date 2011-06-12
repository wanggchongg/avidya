#ifndef __EVENTRPC_THREAD_H__
#define __EVENTRPC_THREAD_H__

namespace eventrpc {
class ThreadWorker {
 public:
  virtual ~ThreadWorker() {};
  virtual void Run() = 0;
};

class Thread {
 public:
  Thread(ThreadWorker *ThreadWorker);

  ~Thread();

  void Start();

 private:
  static void* ThreadMain(void* arg);

 private:
  pthread_t pthread_;
  ThreadWorker *thread_worker_;
};
};
#endif  // __EVENTRPC_THREAD_H__
