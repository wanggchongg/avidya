/*
 * Copyright (C) Lichuang
 */
#include <string>
#include <gtest/gtest.h>
#include <eventrpc/file_utility.h>
#include "global/serialize_utility.h"
using namespace std;
namespace global {
class SerializeUtilityTest : public testing::Test {
 public:
  void SetUp() {
  }

  void TearDown() {
  }
};

TEST_F(SerializeUtilityTest, FileHeaderTest) {
  TransactionLogFileHeader file_header, parse_result;
  file_header.magic = 100;
  file_header.version = 101;
  file_header.dbid = 102;
  string result;
  ASSERT_TRUE(SerializeFileHeaderToString(file_header, &result));
  ASSERT_TRUE(ParseFileHeaderFromString(result, &parse_result));
  ASSERT_EQ(file_header.magic, parse_result.magic);
  ASSERT_EQ(file_header.version, parse_result.version);
  ASSERT_EQ(file_header.dbid, parse_result.dbid);
}

TEST_F(SerializeUtilityTest, TransactionHeaderTest) {
  TransactionHeader header, parse_result;
  header.client_id = 10;
  header.cxid = 101;
  header.gxid = 102;
  header.time = 155;
  header.type = 1;
  header.checksum = 11111;
  header.record_length = 100;

  string result;
  ASSERT_TRUE(SerializeTransactionHeaderToString(header, &result));
  ASSERT_TRUE(ParseTransactionHeaderFromString(result, &parse_result));
  ASSERT_EQ(header.client_id, parse_result.client_id);
  ASSERT_EQ(header.cxid, parse_result.cxid);
  ASSERT_EQ(header.gxid, parse_result.gxid);
  ASSERT_EQ(header.time, parse_result.time);
  ASSERT_EQ(header.type, parse_result.type);
  ASSERT_EQ(header.checksum, parse_result.checksum);
  ASSERT_EQ(header.record_length, parse_result.record_length);
}
};

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}
