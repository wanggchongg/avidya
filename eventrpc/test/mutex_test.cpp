#include <vector>
#include <gtest/gtest.h>
#include <eventrpc/mutex.h>
#include <eventrpc/shared_ptr.h>
#include <eventrpc/thread.h>

using namespace eventrpc;
using namespace std;

class MutexTest : public testing::Test {
};

class IncTask : public Runnable {
 public:
  IncTask(int *value_ptr, Mutex *mutex)
    : value_ptr_(value_ptr),
    mutex_(mutex) {
  }

  virtual void Run() {
    MutexLock lock(mutex_);
    ++(*value_ptr_);
  }

 private:
  int *value_ptr_;
  Mutex *mutex_;
};

TEST_F(MutexTest, TestMultiThread) {
  Mutex mutex;
  int value = 0;
  int test_num = 100;
  vector<shared_ptr<Thread> > threads;

  for (int i = 0; i < test_num; ++i) {
    shared_ptr<Runnable> task = new IncTask(&value, &mutex);
    shared_ptr<Thread> thread = new Thread("test thread", task);
    threads.push_back(thread);
    thread->Start();
  }
  for (int i = 0; i < test_num; ++i) {
    threads[i]->Join();
  }
  ASSERT_EQ(100, test_num);
}

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
