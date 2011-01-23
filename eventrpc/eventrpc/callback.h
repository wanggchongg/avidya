#ifndef __EVENTRPC_CALLBACK_H__
#define __EVENTRPC_CALLBACK_H__
namespace eventrpc {
// use inherit class instead of using evil template trick
struct Callback {
 public:
  virtual void Run() = 0;
};
};
#endif  //  __EVENTRPC_CALLBACK_H__
