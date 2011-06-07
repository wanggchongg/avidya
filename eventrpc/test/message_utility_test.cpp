/*
 * Copyright (C) Lichuang
 */
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <gtest/gtest.h>
#include <list>
#include <string>
#include "echo.pb.h"
#include "eventrpc/message_utility.h"

using namespace std;
namespace eventrpc {
class MessageUtilityTest : public testing::Test {
 public:
  void SetUp() {
  }

  void TearDown() {
  }
};

TEST_F(MessageUtilityTest, TestDecodeEncode) {
  ::echo::EchoResponse response, result;
  string content;
  MessageHeader header;
  response.set_response("test");
  ASSERT_TRUE(EncodeMessage(response, &content));
  ASSERT_TRUE(DecodeMessageHeader(content, &header));
  ASSERT_EQ(header.message_length, response.ByteSize());
  ASSERT_TRUE(result.ParseFromString(content.substr(sizeof(uint32) * 2)));
}
};

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}
