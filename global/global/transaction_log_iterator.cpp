/*
 * Copyright (C) Lichuang
 */
#include <list>
#include <string>
#include <eventrpc/base.h>
#include <eventrpc/string_utility.h>
#include <eventrpc/file_utility.h>
#include <eventrpc/log.h>
#include "global/transaction_log_iterator.h"
#include "global/utility.h"
#include "global/record.pb.h"
using namespace eventrpc;
namespace {
// hard code seems ugly...
static const uint32 kFileHeaderSize = 11;
static const uint32 kHeaderSize = 12;
};
namespace global {
struct TransactionLogIterator::Impl {
 public:
  Impl(const string &log_dir, uint64 gxid);
  ~Impl();
  void Init();
  void GetNextFileContent();
  bool Next();
  void Close();
  global::TransactionHeader* header();

  list<string> sorted_files_;
  string log_dir_;
  uint64 gxid_;
  string buffer_;
  global::TransactionHeader header_;
  ::google::protobuf::Message *record;
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
  GetNextFileContent();
  if (!Next()) {
    return;
  }
  while (header_.gxid() < gxid_) {
    if (!Next()) {
      return;
    }
  }
}

void  TransactionLogIterator::Impl::GetNextFileContent() {
  string file = log_dir_ + sorted_files_.back();
  sorted_files_.pop_back();
  ASSERT_TRUE(FileUtility::ReadFileContents(file, &buffer_));
  global::FileHeader file_header;
  ASSERT_TRUE(file_header.ParseFromString(buffer_));
  // skip the file header
  buffer_ = buffer_.substr(kFileHeaderSize);
}

bool TransactionLogIterator::Impl::Next() {
  if (!header_.ParseFromString(buffer_)) {
    return false;
  }
  buffer_ = buffer_.substr(kHeaderSize + header_.record_length());
  return true;
}

void TransactionLogIterator::Impl::Close() {
}

global::TransactionHeader* TransactionLogIterator::Impl::header() {
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
