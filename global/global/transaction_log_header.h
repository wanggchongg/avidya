/*
 * Copyright (C) Lichuang
 */
#ifndef __GLOBAL_TRANSACTION_HEADER_H__
#define __GLOBAL_TRANSACTION_HEADER_H__
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

  TransactionLogFileHeader()
    : magic(0), version(0), dbid(0) {
  }
  bool Serialize(string *output) const;
  bool Deserialize(const string &input);
};

struct SnapLogFileHeader {
  uint32 magic;
  uint32 version;
  uint32 dbid;
  uint32 session_size;

  SnapLogFileHeader()
    : magic(0), version(0), dbid(0), session_size(0) {
  }
  bool Serialize(string *output) const;
  bool Deserialize(const string &input);
};

struct TransactionHeader {
  uint64 client_id;
  uint32 cxid;
  uint64 gxid;
  uint64 time;
  uint32 type;
  uint32 checksum;
  uint32 record_length;
  TransactionHeader()
    : client_id(0), cxid(0), gxid(0), time(0),
      type(0), checksum(0), record_length(0) {
  }
  bool Serialize(string *output) const;
  bool Deserialize(const string &input);
};

#define FILE_HEADER_SIZE sizeof(TransactionLogFileHeader)
#define TRANSACTION_HEADER_SIZE sizeof(TransactionHeader)
#define SNAP_LOG_HEADER_SIZE sizeof(SnapLogFileHeader)
};
#endif  // __GLOBAL_TRANSACTION_HEADER_H__
