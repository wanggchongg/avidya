/*
 * Copyright (C) Lichuang
 *
 */
#include <eventrpc/log.h>
#include "fast_leader_election.h"

namespace global {
struct FastLeaderElection::Impl {
 public:
  Impl();
  ~Impl();

  void Start(const string &config_file);

 private:
};

FastLeaderElection::Impl::Impl() {
}

FastLeaderElection::Impl::~Impl() {
}

void FastLeaderElection::Impl::Start(const string &config_file) {
}

FastLeaderElection::FastLeaderElection()
  : impl_(new Impl) {
}

FastLeaderElection::~FastLeaderElection() {
  delete impl_;
}

void FastLeaderElection::Start(const string &config_file) {
  impl_->Start(config_file);
}
};
