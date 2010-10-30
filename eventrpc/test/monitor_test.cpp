
#include <vector>
#include <gtest/gtest.h>
#include <eventrpc/monitor.h>
#include <eventrpc/thread.h>
#include <eventrpc/shared_ptr.h>

using namespace eventrpc;
using namespace std;

class MonitorTest : public testing::Test {
};

class TestMonitorTask: public Runnable {
 public:
  TestMonitorTask(const Monitor &monitor, int *count)
    : monitor_(monitor),
    count_(count) {
  }

  void Run() {
    Synchronized s(monitor_);

    (*count_)--;

    if (*count_ == 0) {
      monitor_.Notify();
    }
  }

  const Monitor& monitor_;
  int *count_;
};

TEST_F(MonitorTest, TestMonitor) {
  Monitor monitor;
  int value = 100;
  int loop_num = 100;
  vector<shared_ptr<Thread> > threads;

  for (int i = 0; i < loop_num; ++i) {
    shared_ptr<Runnable> task = new TestMonitorTask(monitor, &value);
    shared_ptr<Thread> thread = new Thread("test thread", task);
    threads.push_back(thread);
    thread->Start();
  }

  {
    Synchronized s(monitor);
    while (value != 0) {
      monitor.TimeWait(1000);
    }
  }
  for (int i = 0; i < loop_num; ++i) {
    threads[i]->Join();
  }

  ASSERT_EQ(0, value);
}

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
