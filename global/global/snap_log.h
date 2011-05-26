/*
 * Copyright (C) Lichuang
 */
#ifndef __GLOBAL_SNAP_LOG_H__
#define __GLOBAL_SNAP_LOG_H__
#include <map>
#include <eventrpc/base.h>
#include "global/data_tree.h"
using namespace std;
namespace global {
class SnapLog {
 public:
  explicit SnapLog(const string &log_dir);
  ~SnapLog();

  bool Deserialize(DataTree *data_tree,
                   map<uint64, uint64> *session_timeouts) const;
  bool Serialize(const DataTree &data_tree,
                 const map<uint64, uint64> &session_timeouts);
 private:
  struct Impl;
  Impl *impl_;
};
};
#endif  // __GLOBAL_SNAP_LOG_H__
