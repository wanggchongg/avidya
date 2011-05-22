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
    ASSERT_FLOAT_EQ(f, StringUtility::ConvertStringToFloat(result));
  }
  {
    bool f = true;
    string result = StringUtility::ConvertBoolToString(f);
    ASSERT_EQ("1", result);
    ASSERT_EQ(f, StringUtility::ConvertStringToBool(result));
  }
  {
    uint64 f = 1000;
    string result = StringUtility::ConvertUint64ToString(f);
    ASSERT_EQ("1000", result);
    ASSERT_EQ(f, StringUtility::ConvertStringToUint64(result));
  }
  {
    int64 f = -1000;
    string result = StringUtility::ConvertInt64ToString(f);
    ASSERT_EQ("-1000", result);
    ASSERT_EQ(f, StringUtility::ConvertStringToInt64(result));
  }
  {
    uint32 f = 1000;
    string result = StringUtility::ConvertUint32ToString(f);
    ASSERT_EQ("1000", result);
    ASSERT_EQ(f, StringUtility::ConvertStringToUint32(result));
  }
  {
    int32 f = -1000;
    string result = StringUtility::ConvertInt32ToString(f);
    ASSERT_EQ("-1000", result);
    ASSERT_EQ(f, StringUtility::ConvertStringToInt32(result));
  }
}
};

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}
