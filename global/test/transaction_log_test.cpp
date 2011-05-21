/*
 * Copyright (C) Lichuang
 */
#include <gtest/gtest.h>
#include <eventrpc/file_utility.h>
#include "global/transaction_log_iterator.h"
#include "global/transaction_log.h"

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
  header.set_client_id(1);
  header.set_cxid(1);
  header.set_gxid(1);
  header.set_time(1);
  header.set_type(1);
  header.set_checksum(1);
  global::Delete record;
  record.set_path("test");
  header.set_record_length(record.ByteSize());
  log.Append(header, &record);

  header.set_client_id(2);
  header.set_cxid(2);
  header.set_gxid(22);
  header.set_time(2);
  header.set_type(2);
  header.set_checksum(2);
  global::Create record2;
  record2.set_path("create");
  header.set_record_length(record2.ByteSize());
  log.Append(header, &record2);
  log.Commit();
}

void TransactionLogTest::DeleteTestLogFile() {
  string file = tmp_dir_ + "/log.1";
  remove(file.c_str());
}

TEST_F(TransactionLogTest, AppendTest) {
  string file_name = tmp_dir_ + "log.1";
  string content;
  ASSERT_TRUE(FileUtility::ReadFileContents(file_name, &content));
  global::FileHeader file_header;
  ASSERT_TRUE(file_header.ParseFromString(content));
  global::TransactionHeader header;
  content = content.substr(11);
  ASSERT_TRUE(header.ParseFromString(content));
  ASSERT_EQ(1u, header.gxid());
}
};

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}
