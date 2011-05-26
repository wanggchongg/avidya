/*
 * Copyright (C) Lichuang
 */
#ifndef __GLOBAL_TRANSACTION_LOG_H__
#define __GLOBAL_TRANSACTION_LOG_H__
#include <list>
#include <string>
#include <eventrpc/base.h>
#include "global/transaction_log_header.h"
#include "global/record.pb.h"
using namespace std;
namespace global {
class TransactionLogIterator;
class TransactionLog {
 public:
  explicit TransactionLog(const string &log_dir);
  ~TransactionLog();

  void Roll();

  bool Append(const global::TransactionHeader &header,
              const ::google::protobuf::Message *message);

  TransactionLogIterator* Read(uint64 gxid);

  uint64 GetLastLoggedGxid() const;

  bool GetLogFiles(list<string> *files, uint64 gxid) const;

  bool Truncate(uint64 gxid);

  uint64 dbid() const;

  void Commit();

  void Close();
 private:
  struct Impl;
  Impl *impl_;
};
};
#endif  // __GLOBAL_TRANSACTION_LOG_H__
