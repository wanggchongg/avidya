#include <gtest/gtest.h>
#include <eventrpc/thread.h>

using namespace eventrpc;

class ThreadTest : public testing::Test {
};

class AssignTask : public Runnable {
 public:
  AssignTask(int *value_ptr, int value)
    : value_ptr_(value_ptr),
      value_(value) {
  }

  virtual void Run() {
    *value_ptr_ = value_;
  }

 private:
  int *value_ptr_;
  int value_;
};

TEST_F(ThreadTest, TestAssignThread) {
  int value = 1;
  shared_ptr<Runnable> task = new AssignTask(&value, 100);
  Thread thread("test thread", task);
  thread.Start();
  thread.Join();
  ASSERT_EQ(100, value);
}

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
