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

TEST_F(StringUtilityTest, SerializeToStringTest) {
  {
    bool f = true;
    string result = StringUtility::SerializeBoolToString(f);
    result += "abn";
    ASSERT_EQ(f, StringUtility::DeserializeStringToBool(result));
  }
  {
    uint64 f = 1000;
    string result = StringUtility::SerializeUint64ToString(f);
    ASSERT_EQ(f, StringUtility::DeserializeStringToUint64(result));
  }
  {
    int64 f = 1000;
    string result = StringUtility::SerializeInt64ToString(f);
    ASSERT_EQ(f, StringUtility::DeserializeStringToInt64(result));
  }
  {
    uint32 f = 1000;
    string result = StringUtility::SerializeUint32ToString(f);
    ASSERT_EQ(f, StringUtility::DeserializeStringToUint32(result));
  }
  {
    int32 f = 1000;
    string result = StringUtility::SerializeInt32ToString(f);
    ASSERT_EQ(f, StringUtility::DeserializeStringToInt32(result));
  }
}
};

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}
