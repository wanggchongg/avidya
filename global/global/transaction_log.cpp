/*
 * Copyright (C) Lichuang
 */
#include <stdio.h>
#include <eventrpc/string_utility.h>
#include <eventrpc/file_utility.h>
#include <eventrpc/log.h>
#include "global/transaction_log.h"
#include "global/transaction_log_iterator.h"
#include "global/utility.h"
#include "global/serialize_utility.h"
namespace {
static const char kLogFileHeaderMagic[] = "GTLOG";
static const uint32 kLogVersion = 1;
static const uint64 kDbId = 1;
};
using namespace eventrpc;
namespace global {
struct TransactionLog::Impl {
 public:
  explicit Impl(const string& log_dir);
  ~Impl();

  void Roll();

  bool Append(const global::TransactionHeader &header,
              const ::google::protobuf::Message *message);

  TransactionLogIterator* Read(uint64 gxid);

  uint64 GetLastLoggedGxid() const;

  uint64 dbid() const;

  void Commit();

  void Close();

 private:
  string log_dir_;
  FILE *file_;
  TransactionLogFileHeader file_header_;
  string file_header_string_;
};

TransactionLog::Impl::Impl(const string &log_dir)
  : log_dir_(log_dir),
    file_(NULL) {
  file_header_.magic = atol(kLogFileHeaderMagic);
  file_header_.version = kLogVersion;
  file_header_.dbid = kDbId;
  SerializeFileHeaderToString(file_header_, &file_header_string_);
}

TransactionLog::Impl::~Impl() {
  Close();
}

void TransactionLog::Impl::Roll() {
  fflush(file_);
  fclose(file_);
  file_ = NULL;
}

bool TransactionLog::Impl::Append(
    const global::TransactionHeader &header,
    const ::google::protobuf::Message *message) {
  if (file_ == NULL) {
    string filename = log_dir_ + "/log.";
    filename += StringUtility::ConvertUint64ToString(header.gxid);
    file_ = fopen(filename.c_str(), "a");
    fwrite(file_header_string_.c_str(), sizeof(char),
           file_header_string_.length(), file_);
  }
  string output;
  SerializeTransactionHeaderToString(header, &output);
  fwrite(output.c_str(), sizeof(char), output.length(), file_);
  output = "";
  message->SerializeToString(&output);
  fwrite(output.c_str(), sizeof(char), output.length(), file_);
  return true;
}

TransactionLogIterator* TransactionLog::Impl::Read(uint64 gxid) {
  return NULL;
}

uint64 TransactionLog::Impl::GetLastLoggedGxid() const {
  return 0;
}

uint64 TransactionLog::Impl::dbid() const {
  return kDbId;
}

void TransactionLog::Impl::Commit() {
  if (file_) {
    fflush(file_);
  }
}

void TransactionLog::Impl::Close() {
  Roll();
}

TransactionLog::TransactionLog(const string &log_dir)
  : impl_(new Impl(log_dir)) {
}

TransactionLog::~TransactionLog() {
  delete impl_;
}

void TransactionLog::Roll() {
  impl_->Roll();
}

bool TransactionLog::Append(const global::TransactionHeader &header,
                            const ::google::protobuf::Message *message) {
  return impl_->Append(header, message);
}

TransactionLogIterator* TransactionLog::Read(uint64 gxid) {
  return impl_->Read(gxid);
}

uint64 TransactionLog::GetLastLoggedGxid() const {
  return impl_->GetLastLoggedGxid();
}

uint64 TransactionLog::dbid() const {
  return impl_->dbid();
}

void TransactionLog::Commit() {
  impl_->Commit();
}

void TransactionLog::Close() {
  impl_->Close();
}
};
