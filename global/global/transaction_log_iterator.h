/*
 * Copyright (C) Lichuang
 */
#ifndef __GLOBAL_TRANSACTION_LOG_ITERATOR_H__
#define __GLOBAL_TRANSACTION_LOG_ITERATOR_H__
#include <eventrpc/base.h>
#include "global/transaction.pb.h"
using namespace std;
namespace global {
class TransactionLogIterator {
 public:
  TransactionLogIterator(const string &log_dir, uint64 gxid);
  ~TransactionLogIterator();
  bool Next();
  void Close();
  global::TransactionHeader* header();
 private:
  struct Impl;
  Impl *impl_;
};
};
#endif  // __GLOBAL_TRANSACTION_LOG_ITERATOR_H__
