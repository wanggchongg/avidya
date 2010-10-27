#include <gtest/gtest.h>
#include <eventrpc/atomic.h>

class AtomicTest : public testing::Test {
};

TEST_F(AtomicTest, TestAtomicOperation) {
}

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
