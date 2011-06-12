#ifndef __EVENTRPC_TASK_H__
#define __EVENTRPC_TASK_H__
namespace eventrpc {
// use inherit class instead of using evil template trick
struct Task {
 public:
  Task() {
  }
  virtual ~Task() {
  }
  virtual void Handle() = 0;
};
};
#endif  // __EVENTRPC_TASK_H__
