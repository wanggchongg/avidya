/*
 * Copyright (C) Lichuang
 */
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <gtest/gtest.h>
#include <list>
#include <string>
#include "eventrpc/string_utility.h"

using namespace std;
namespace eventrpc {
class StringUtilityTest : public testing::Test {
 public:
  void SetUp() {
  }

  void TearDown() {
  }
};

TEST_F(StringUtilityTest, ConvertToStringTest) {
  {
    float f = 1.01;
    string result = StringUtility::ConvertFloatToString(f);
    ASSERT_EQ("1.01", result);
  }
  {
    bool f = true;
    string result = StringUtility::ConvertFloatToString(f);
    ASSERT_EQ("1", result);
  }
  {
    uint64 f = 1000;
    string result = StringUtility::ConvertFloatToString(f);
    ASSERT_EQ("1000", result);
  }
  {
    int64 f = -1000;
    string result = StringUtility::ConvertFloatToString(f);
    ASSERT_EQ("-1000", result);
  }
  {
    uint32 f = 1000;
    string result = StringUtility::ConvertFloatToString(f);
    ASSERT_EQ("1000", result);
  }
  {
    int32 f = -1000;
    string result = StringUtility::ConvertFloatToString(f);
    ASSERT_EQ("-1000", result);
  }
}
};

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}
