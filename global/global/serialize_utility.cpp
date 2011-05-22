/*
 * Copyright (C) Lichuang
 */
#include <eventrpc/log.h>
#include <eventrpc/string_utility.h>
#include "global/serialize_utility.h"
using namespace eventrpc;
namespace global {
bool SerializeFileHeaderToString(
    const TransactionLogFileHeader &file_header,
    string *output) {
  ASSERT(output != NULL);
  // uint32 magic
  output->append(StringUtility::SerializeUint32ToString(file_header.magic));
  // uint32 version
  output->append(StringUtility::SerializeUint32ToString(file_header.version));
  // uint32 dbid
  output->append(StringUtility::SerializeUint32ToString(file_header.dbid));
  return true;
}

bool ParseFileHeaderFromString(
    const string &input,
    TransactionLogFileHeader *file_header) {
  ASSERT(file_header != NULL);
  uint32 pos = 0;
  uint32 size = sizeof(uint32);
  file_header->magic = StringUtility::DeserializeStringToUint32(
      input.substr(pos, size));
  pos += size;
  file_header->version = StringUtility::DeserializeStringToUint32(

      input.substr(pos, size));
  pos += size;
  file_header->dbid = StringUtility::DeserializeStringToUint32(
      input.substr(pos));
  return true;
}

bool SerializeTransactionHeaderToString(
    const TransactionHeader &header,
    string *output) {
  ASSERT(output != NULL);
  // uint64 client id
  output->append(StringUtility::SerializeUint64ToString(header.client_id));
  // uint32 cxid
  output->append(StringUtility::SerializeUint32ToString(header.cxid));
  // uint64 gxid
  output->append(StringUtility::SerializeUint64ToString(header.gxid));
  // uint64 time
  output->append(StringUtility::SerializeUint64ToString(header.time));
  // uint32 type
  output->append(StringUtility::SerializeUint32ToString(header.type));
  // uint32 checksum
  output->append(StringUtility::SerializeUint32ToString(header.checksum));
  // uint32 record_length
  output->append(StringUtility::SerializeUint32ToString(header.record_length));
  return true;
}

bool ParseTransactionHeaderFromString(
    const string &input,
    TransactionHeader *header) {
  ASSERT(header != NULL);
  uint32 pos = 0;
  uint32 size = sizeof(uint64);
  header->client_id = StringUtility::DeserializeStringToUint64(
      input.substr(pos, size));

  pos += size;
  size = sizeof(uint32);
  header->cxid = StringUtility::DeserializeStringToUint32(

      input.substr(pos, size));

  pos += size;
  size = sizeof(uint64);
  header->gxid = StringUtility::DeserializeStringToUint64(
      input.substr(pos, size));

  pos += size;
  size = sizeof(uint64);
  header->time = StringUtility::DeserializeStringToUint64(
      input.substr(pos, size));

  pos += size;
  size = sizeof(uint32);
  header->type = StringUtility::DeserializeStringToUint32(
      input.substr(pos, size));

  pos += size;
  size = sizeof(uint32);
  header->checksum = StringUtility::DeserializeStringToUint32(
      input.substr(pos, size));

  pos += size;
  size = sizeof(uint32);
  header->record_length = StringUtility::DeserializeStringToUint32(
      input.substr(pos, size));
  return true;
};
};
