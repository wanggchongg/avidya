#include <gtest/gtest.h>
#include <eventrpc/atomic.h>

class AtomicTest : public testing::Test {
};

TEST_F(AtomicTest, TestAtomicOperation) {
  uint32_t value = 1;
  uint32_t mem;

  atomic_set(&mem, value);
  ASSERT_EQ(1, atomic_read(&mem));

  ASSERT_EQ(2, atomic_add(&mem, value));
  ASSERT_EQ(2, atomic_read(&mem));

  ASSERT_EQ(1, atomic_sub(&mem, value));
  ASSERT_EQ(1, atomic_read(&mem));

  ASSERT_EQ(2, atomic_inc(&mem));
  ASSERT_EQ(2, atomic_read(&mem));

  ASSERT_EQ(1, atomic_dec(&mem));
  ASSERT_EQ(1, atomic_read(&mem));

  ASSERT_EQ(1, atomic_cas(&mem, 1, 3));
  ASSERT_EQ(3, atomic_read(&mem));

  ASSERT_EQ(3, atomic_xchg(&mem, 1));
  ASSERT_EQ(1, atomic_read(&mem));

  uint32_t *ptr = &mem;
  value = 100;
  ASSERT_EQ(&mem, atomic_casptr((volatile void**)&ptr, &mem, &value));
  ASSERT_EQ(ptr, &value);

  ASSERT_EQ(&value, atomic_xchgptr((volatile void**)&ptr, &mem));
  ASSERT_EQ(ptr, &mem);
}

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
