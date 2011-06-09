/*
 * Copyright (C) Lichuang
 */
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <gtest/gtest.h>
#include <list>
#include <string>
#include "eventrpc/buffer.h"

using namespace std;
namespace eventrpc {
class BufferTest : public testing::Test {
 public:
  void SetUp() {
  }

  void TearDown() {
  }
};

TEST_F(BufferTest, ConvertTest) {
}

TEST_F(BufferTest, SerializeTest) {
  {
    Buffer buffer;
    uint32 i = 100, output;
    ASSERT_TRUE(buffer.is_read_complete());
    buffer.SerializeFromUint32(i);
    ASSERT_FALSE(buffer.is_read_complete());
    ASSERT_EQ(sizeof(uint32), buffer.end_position());
    output = buffer.DeserializeToUint32();
    ASSERT_TRUE(buffer.is_read_complete());
    ASSERT_EQ(i, output);
  }
};
}

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}
