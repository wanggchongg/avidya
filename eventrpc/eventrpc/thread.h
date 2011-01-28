#ifndef __EVENTRPC_THREAD_H__
#define __EVENTRPC_THREAD_H__

namespace eventrpc {
class Runnable {
 public:
  virtual ~Runnable() {};
  virtual void Run() = 0;
};

class Thread {
 public:
  Thread(Runnable *runnable);

  ~Thread();

  void Start();

 private:
  static void* ThreadMain(void* arg);

 private:
  pthread_t pthread_;
  Runnable *runnable_;
};
};
#endif  // __EVENTRPC_THREAD_H__
