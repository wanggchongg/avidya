/*
 * Copyright (C) Lichuang
 */
#include <iostream>
#include <gtest/gtest.h>
#include <eventrpc/file_utility.h>
#include "global/transaction_log_iterator.h"
#include "global/transaction_log.h"
#include "global/serialize_utility.h"

using namespace eventrpc;
namespace global {
class TransactionLogTest : public testing::Test {
 public:
  void SetUp() {
    tmp_dir_ = "/tmp/test_dir/";
    ASSERT_EQ(0, mkdir(tmp_dir_.c_str(), S_IRWXU));
    CreateTestLogFile();
  }

  void TearDown() {
    DeleteTestLogFile();
    rmdir(tmp_dir_.c_str());
  }
  void CreateTestLogFile();
  void DeleteTestLogFile();
  string tmp_dir_;
};

void TransactionLogTest::CreateTestLogFile() {
  TransactionLog log(tmp_dir_);
  global::TransactionHeader header;
  header.client_id = 1;
  header.cxid = 1;
  header.gxid = 1;
  header.time = 1;
  header.type = 1;
  header.checksum = 1;
  global::Delete record;
  record.set_path("test");
  header.record_length = record.ByteSize();
  log.Append(header, &record);

  header.client_id = 1;
  header.cxid = 1;
  header.gxid = 22;
  header.time = 1;
  header.type = 1;
  header.checksum = 1;
  record.set_path("test2");
  header.record_length = record.ByteSize();
  log.Append(header, &record);
}

void TransactionLogTest::DeleteTestLogFile() {
  string file = tmp_dir_ + "/log.1";
  remove(file.c_str());
}

TEST_F(TransactionLogTest, AppendTest) {
  string file_name = tmp_dir_ + "log.1";
  string content;
  ASSERT_TRUE(FileUtility::ReadFileContents(file_name, &content));
  TransactionLogFileHeader file_header;
  ASSERT_TRUE(ParseFileHeaderFromString(content, &file_header));
  ASSERT_EQ(1u, file_header.dbid);
  ASSERT_EQ(atol("GTLOG"), file_header.magic);
  ASSERT_EQ(1u, file_header.version);

  content = content.substr(FILE_HEADER_SIZE);
  TransactionHeader header;
  ASSERT_TRUE(ParseTransactionHeaderFromString(content, &header));
  ASSERT_EQ(1u, header.gxid);
  content = content.substr(TRANSACTION_HEADER_SIZE + header.record_length);
  ASSERT_TRUE(ParseTransactionHeaderFromString(content, &header));
  ASSERT_EQ(22u, header.gxid);
  return;
}
};

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}
