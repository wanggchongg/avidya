/*
 * Copyright (C) Lichuang
 *
 */
#ifndef __GLOBAL_FAST_ELECTION_H__
#define __GLOBAL_FAST_ELECTION_H__
#include <eventrpc/base.h>
#include <string>
using namespace std;
namespace global {
class FastLeaderElection {
 public:
  explicit FastLeaderElection();
  ~FastLeaderElection();

  void Start(const string &config_file);
 private:
  struct Impl;
  Impl *impl_;
};
};
#endif  // __GLOBAL_FAST_ELECTION_H__
