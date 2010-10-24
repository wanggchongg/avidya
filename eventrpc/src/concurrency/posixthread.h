
#ifndef __EVENTRPC_POSIX_THREAD_H__
#define __EVENTRPC_POSIX_THREAD_H__

#include <concurrency/thread.h>

EVENTRPC_NAMESPACE_BEGIN

class PosixThreadFactory : public ThreadFactory {
 public:
  PosixThreadFactory();

  virtual ~PosixThreadFactory();

  virtual shared_ptr<Thread> newThread(shared_ptr<Runnable> runnable);
};

EVENTRPC_NAMESPACE_END

#endif  //  __EVENTRPC_POSIX_THREAD_H__
