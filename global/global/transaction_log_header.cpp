/*
 * Copyright (C) Lichuang
 */
#include <eventrpc/log.h>
#include <eventrpc/string_utility.h>
#include "global/transaction_log_header.h"
using namespace eventrpc;
namespace global {
bool TransactionLogFileHeader::Serialize(string *output) const {
  ASSERT(output != NULL);
  // uint32 magic
  output->append(StringUtility::SerializeUint32ToString(magic));
  // uint32 version
  output->append(StringUtility::SerializeUint32ToString(version));
  // uint32 dbid
  output->append(StringUtility::SerializeUint32ToString(dbid));
  return true;
}

bool TransactionLogFileHeader::Deserialize(const string &input) {
  uint32 pos = 0;
  uint32 size = sizeof(uint32);
  magic = StringUtility::DeserializeStringToUint32(
      input.substr(pos, size));
  pos += size;
  version = StringUtility::DeserializeStringToUint32(
      input.substr(pos, size));
  pos += size;
  dbid = StringUtility::DeserializeStringToUint32(
      input.substr(pos));
  return true;
}

bool SnapLogFileHeader::Serialize(string *output) const {
  ASSERT(output != NULL);
  // uint32 magic
  output->append(StringUtility::SerializeUint32ToString(magic));
  // uint32 version
  output->append(StringUtility::SerializeUint32ToString(version));
  // uint32 dbid
  output->append(StringUtility::SerializeUint32ToString(dbid));
  // uint32 session_size
  output->append(StringUtility::SerializeUint32ToString(session_size));
  return true;
}

bool SnapLogFileHeader::Deserialize(const string &input) {
  uint32 pos = 0;
  uint32 size = sizeof(uint32);
  magic = StringUtility::DeserializeStringToUint32(
      input.substr(pos, size));
  pos += size;
  version = StringUtility::DeserializeStringToUint32(
      input.substr(pos, size));
  pos += size;
  dbid = StringUtility::DeserializeStringToUint32(
      input.substr(pos));
  pos += size;
  session_size = StringUtility::DeserializeStringToUint32(
      input.substr(pos));
  return true;
}
bool TransactionHeader::Serialize(string *output) const {
  ASSERT(output != NULL);
  // uint64 client id
  output->append(StringUtility::SerializeUint64ToString(client_id));
  // uint32 cxid
  output->append(StringUtility::SerializeUint32ToString(cxid));
  // uint64 gxid
  output->append(StringUtility::SerializeUint64ToString(gxid));
  // uint64 time
  output->append(StringUtility::SerializeUint64ToString(time));
  // uint32 type
  output->append(StringUtility::SerializeUint32ToString(type));
  // uint32 checksum
  output->append(StringUtility::SerializeUint32ToString(checksum));
  // uint32 record_length
  output->append(StringUtility::SerializeUint32ToString(record_length));
  return true;
}

bool TransactionHeader::Deserialize(const string &input) {
  uint32 pos = 0;
  uint32 size = sizeof(uint64);
  client_id = StringUtility::DeserializeStringToUint64(
      input.substr(pos, size));

  pos += size;
  size = sizeof(uint32);
  cxid = StringUtility::DeserializeStringToUint32(
      input.substr(pos, size));

  pos += size;
  size = sizeof(uint64);
  gxid = StringUtility::DeserializeStringToUint64(
      input.substr(pos, size));

  pos += size;
  size = sizeof(uint64);
  time = StringUtility::DeserializeStringToUint64(
      input.substr(pos, size));

  pos += size;
  size = sizeof(uint32);
  type = StringUtility::DeserializeStringToUint32(
      input.substr(pos, size));

  pos += size;
  size = sizeof(uint32);
  checksum = StringUtility::DeserializeStringToUint32(
      input.substr(pos, size));

  pos += size;
  size = sizeof(uint32);
  record_length = StringUtility::DeserializeStringToUint32(
      input.substr(pos, size));
  return true;
};
};
