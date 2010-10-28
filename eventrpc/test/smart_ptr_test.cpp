
#include <gtest/gtest.h>
#include <eventrpc/shared_ptr.h>

using namespace eventrpc;

class SharedPtrTest : public testing::Test {
};

TEST_F(SharedPtrTest, TestSharedPtr) {
  int value = 1;
  shared_ptr<int> ptr1(new int(value));
  ASSERT_EQ(1, ptr1.use_count());
  {
    shared_ptr<int> ptr2(ptr1);
    ASSERT_EQ(2, ptr1.use_count());
    ASSERT_EQ(2, ptr2.use_count());
    ASSERT_FALSE(ptr1.unique());
  }
  {
    shared_ptr<int> ptr2 = ptr1;
    ASSERT_EQ(2, ptr1.use_count());
    ASSERT_EQ(2, ptr2.use_count());
    ASSERT_FALSE(ptr1.unique());
  }
  ASSERT_EQ(1, ptr1.use_count());

  int *p = ptr1.get();
  ASSERT_EQ(value, *p);
  ASSERT_EQ(value, *ptr1);
  ASSERT_TRUE(ptr1.unique());

  ptr1.reset(NULL);
  ASSERT_EQ(1, ptr1.use_count());
}

TEST_F(SharedPtrTest, TestWeakPtr) {
  weak_ptr<int> wptr;
  int value = 1;

  {
    shared_ptr<int> ptr1(new int(value));
    wptr = ptr1.weaker();
    ASSERT_EQ(1, ptr1.use_count());
    ASSERT_EQ(1, wptr.use_count());
    ASSERT_FALSE(wptr.expired());
    shared_ptr<int> ptr2 = wptr.lock();
    ASSERT_EQ(1, ptr1.use_count());
  }
  ASSERT_TRUE(wptr.expired());
}

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
