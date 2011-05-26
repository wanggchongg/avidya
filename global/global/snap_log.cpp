/*
 * Copyright (C) Lichuang
 */
#include <stdio.h>
#include <eventrpc/string_utility.h>
#include <eventrpc/file_utility.h>
#include <eventrpc/log.h>
#include "global/snap_log.h"
#include "global/transaction_log_header.h"
#include "global/utility.h"
#include "global/serialize_utility.h"
namespace {
static const char kSnapLogFileHeaderMagic[] = "GSNLOG";
static const uint32 kLogVersion = 1;
static const uint64 kDbId = 1;
};
using namespace eventrpc;
namespace global {
struct SnapLog::Impl {
 public:
  explicit Impl(const string &log_dir);
  ~Impl();

  bool Deserialize(DataTree *data_tree,
                   map<uint64, uint64> *session_timeouts) const;
  bool Serialize(const DataTree &data_tree,
                 const map<uint64, uint64> &session_timeouts,
                 string *output);
  bool FindNValidSnapshots(uint32 count, list<string> *files) const;

  string log_dir_;
};

SnapLog::Impl::Impl(const string &log_dir)
  : log_dir_(log_dir) {
}

SnapLog::Impl::~Impl() {
}

bool SnapLog::Impl::Deserialize(
    DataTree *data_tree,
    map<uint64, uint64> *session_timeouts) const {
  list<string> files;
  if (!FindNValidSnapshots(100, &files)) {
    return false;
  }
  list<string>::iterator iter = files.begin();
  for(; iter != files.end(); ++iter) {
    string content;
    ASSERT_TRUE(FileUtility::ReadFileContents(*iter, &content));
    SnapLogFileHeader header;
    ASSERT_TRUE(header.Deserialize(content));
    global::SessionList sessions;
    content = content.substr(SNAP_LOG_HEADER_SIZE);
    ASSERT_TRUE(sessions.ParseFromString(content.substr(0, header.session_size)));
    ASSERT_TRUE(DeserializeSessionList(sessions, session_timeouts));
    content = content.substr(header.session_size);
    ASSERT_TRUE(data_tree->Deserialize(content));
  }
  return true;
}

bool SnapLog::Impl::Serialize(
    const DataTree &data_tree,
    const map<uint64, uint64> &session_timeouts,
    string *output) {
  global::SnapLogFileHeader header;
  header.magic = atol(kSnapLogFileHeaderMagic);
  header.version = kLogVersion;
  header.dbid = kDbId;
  global::SessionList session_list;
  ASSERT_TRUE(SerializeSessionList(session_timeouts, &session_list));
  header.session_size = session_list.ByteSize();
  string header_content;
  ASSERT_TRUE(header.Serialize(&header_content));
  string session_list_content;
  ASSERT_TRUE(session_list.SerializeToString(&session_list_content));
  string data_tree_content;
  ASSERT_TRUE(data_tree.Serialize(&data_tree_content));
  *output = header_content;
  output->append(session_list_content);
  output->append(data_tree_content);
  return true;
}

bool SnapLog::Impl::FindNValidSnapshots(uint32 count,
                                        list<string> *files) const {
  if (!SortFiles(log_dir_, "snap", false, files)) {
    return false;
  }
  return true;
}

SnapLog::SnapLog(const string &log_dir)
  : impl_(new Impl(log_dir)) {
}

SnapLog::~SnapLog() {
  delete impl_;
}

bool SnapLog::Deserialize(
    DataTree *data_tree,
    map<uint64, uint64> *session_timeouts) const {
  return impl_->Deserialize(data_tree, session_timeouts);
}

bool SnapLog::Serialize(
    const DataTree &data_tree,
    const map<uint64, uint64> &session_timeouts,
    string *output) {
  return impl_->Serialize(data_tree, session_timeouts, output);
}
};
