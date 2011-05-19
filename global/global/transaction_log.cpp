/*
 * Copyright (C) Lichuang
 */
#include <stdio.h>
#include "global/transaction_log.h"
#include "global/utility.h"
namespace {
static const char kLogFileHeaderMagic[] = "GTLOG";
static const uint32 kLogVersion = 1;
static const uint64 kDbId = 1;
};
namespace global {
struct TransactionLog::Impl {
 public:
  explicit Impl(const string& log_dir);
  ~Impl();

  void Roll();

  bool Append(const global::TransactionHeader &header,
              const ::google::protobuf::Message &message);

  TransactionLogIterator* Read(uint64 gxid);

  void SerializeToString(const global::TransactionHeader &header,
                         const ::google::protobuf::Message &message,
                         string *output);

  uint64 GetLastLoggedGxid() const;

  uint64 GetDbId() const;

  void Commit();

  void Close();

 private:
  string log_dir_;
  FILE *file_;
  global::FileHeader file_header_;
  string file_header_string_;
};

TransactionLog::Impl::Impl(const string &log_dir)
  : log_dir_(log_dir),
    file_(NULL) {
  file_header_.set_magic(kLogFileHeaderMagic);
  file_header_.set_version(kLogVersion);
  file_header_.set_dbid(kDbId);
  file_header_.SerializeToString(&file_header_string_);
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
    const ::google::protobuf::Message &message) {
  string output;
  SerializeToString(header, message, &output);
  if (file_ == NULL) {
    string filename = log_dir_ + "/log";
    file_ = fopen(filename.c_str(), "w");
    fwrite(file_header_string_.c_str(), 1,
           file_header_string_.length(), file_);
  }
  fwrite(output.c_str(), 1, output.length(), file_);
  return true;
}

TransactionLogIterator* TransactionLog::Impl::Read(uint64 gxid) {
  return NULL;
}

uint64 TransactionLog::Impl::GetLastLoggedGxid() const {
  return 0;
}

uint64 TransactionLog::Impl::GetDbId() const {
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

TransactionLogIterator::TransactionLogIterator(
    const string &log_dir, uint64 gxid)
  : log_dir_(log_dir),
    gxid_(gxid) {
  Init();
}

TransactionLogIterator::~TransactionLogIterator() {
}

bool TransactionLogIterator::Init() {
  list<string> files;
  SortFiles(log_dir_, "log", false, &files);
  list<string>::iterator iter;
  for (iter = files.begin(); iter != files.end(); ++iter) {
  }
}

bool TransactionLogIterator::Next() {
}

void TransactionLogIterator::Close() {
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
                            const ::google::protobuf::Message &message) {
  return impl_->Append(header, message);
}

TransactionLogIterator* TransactionLog::Read(uint64 gxid) {
  return impl_->Read(gxid);
}

uint64 TransactionLog::GetLastLoggedGxid() const {
  return impl_->GetLastLoggedGxid();
}

uint64 TransactionLog::GetDbId() const {
  return impl_->GetDbId();
}

void TransactionLog::Commit() {
  impl_->Commit();
}

void TransactionLog::Close() {
  impl_->Close();
}
};
