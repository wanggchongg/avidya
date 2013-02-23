// Copyright (C) 2013, avidya
// Created by lichuang1982@gmail.com
// Last modified: anqin.qin@gmail.com

#ifndef __EVENTRPC_TASK_H__
#define __EVENTRPC_TASK_H__

#include <string>

namespace eventrpc {
// use inherit class instead of using evil template trick
struct Task {
 public:
  Task() {
  }
  virtual ~Task() {
  }
  virtual void Handle() = 0;
  virtual std::string TaskName() = 0;
};

struct TimerTask {
 public:
  TimerTask() {
  }
  virtual ~TimerTask() {
  }
  virtual void Handle() = 0;
};
};
#endif  // __EVENTRPC_TASK_H__
