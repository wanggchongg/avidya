/*
 * Copyright (C) Lichuang
 */
#ifndef __GLOBAL_TRANSACTION_H__
#define __GLOBAL_TRANSACTION_H__
#include <string>
#include <eventrpc/base.h>
using namespace std;
namespace global {
enum RecordType {
  CREATE = 1,
};
struct TransactionLogFileHeader {
  uint32 magic;
  uint32 version;
  uint32 dbid;
};

struct TransactionHeader {
  uint64 client_id;
  uint32 cxid;
  uint64 gxid;
  uint64 time;
  uint32 type;
  uint32 checksum;
  uint32 record_length;
};
#define FILE_HEADER_SIZE sizeof(TransactionLogFileHeader)
#define TRANSACTION_HEADER_SIZE sizeof(TransactionHeader)
};
#endif  // __GLOBAL_TRANSACTION_H__
