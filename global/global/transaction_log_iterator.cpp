/*
 * Copyright (C) Lichuang
 */
#include <list>
#include <string>
#include <eventrpc/base.h>
#include <eventrpc/string_utility.h>
#include <eventrpc/file_utility.h>
#include <eventrpc/log.h>
#include "global/transaction.h"
#include "global/transaction_log.h"
#include "global/transaction_log_iterator.h"
#include "global/utility.h"
#include "global/serialize_utility.h"
#include "global/record.pb.h"
using namespace eventrpc;
namespace global {
struct TransactionLogIterator::Impl {
 public:
  Impl(const string &log_dir, uint64 gxid);
  ~Impl();
  void Init();
  bool GetNextFileContent();
  bool Next();
  void Close();
  TransactionHeader* header();

  list<string> sorted_files_;
  string log_dir_;
  uint64 gxid_;
  string buffer_;
  TransactionHeader header_;
};

TransactionLogIterator::Impl::Impl(
    const string &log_dir, uint64 gxid)
  : log_dir_(log_dir),
    gxid_(gxid) {
  Init();
}

TransactionLogIterator::Impl::~Impl() {
}

void TransactionLogIterator::Impl::Init() {
  list<string> files;
  if (!SortFiles(log_dir_, "log", false, &files)) {
    return;
  }
  list<string>::iterator iter;
  for (iter = files.begin(); iter != files.end(); ++iter) {
    uint64 gxid = GetGxidOfFileName(*iter, "log");
    if (gxid >= gxid_) {
      sorted_files_.push_back(*iter);
      continue;
    }
    sorted_files_.push_back(*iter);
    break;
  }
  if (!GetNextFileContent()) {
    return;
  }
  if (!Next()) {
    return;
  }
  while (header_.gxid < gxid_) {
    if (!Next()) {
      return;
    }
  }
}

bool  TransactionLogIterator::Impl::GetNextFileContent() {
  if (sorted_files_.empty()) {
    return false;
  }
  string file = log_dir_ + sorted_files_.back();
  sorted_files_.pop_back();
  ASSERT_TRUE(FileUtility::ReadFileContents(file, &buffer_));
  TransactionLogFileHeader file_header;
  ASSERT_TRUE(ParseFileHeaderFromString(buffer_, &file_header));
  // skip the file header
  buffer_ = buffer_.substr(FILE_HEADER_SIZE);
  return true;
}

bool TransactionLogIterator::Impl::Next() {
  if (buffer_.length() < TRANSACTION_HEADER_SIZE) {
    if (!GetNextFileContent()) {
      return false;
    }
    return Next();
  }
  if (!ParseTransactionHeaderFromString(buffer_, &header_)) {
    return false;
  }
  buffer_ = buffer_.substr(TRANSACTION_HEADER_SIZE + header_.record_length);
  return true;
}

void TransactionLogIterator::Impl::Close() {
  sorted_files_.clear();
}

TransactionHeader* TransactionLogIterator::Impl::header() {
  return &header_;
}

TransactionLogIterator::TransactionLogIterator(
    const string &log_dir, uint64 gxid) {
  impl_ = new Impl(log_dir, gxid);
}

TransactionLogIterator::~TransactionLogIterator() {
  delete impl_;
}

bool TransactionLogIterator::Next() {
  return impl_->Next();
}

void TransactionLogIterator::Close() {
  impl_->Close();
}

global::TransactionHeader* TransactionLogIterator::header() {
  return impl_->header();
}
};
