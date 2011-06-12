#ifndef __EVENTRPC_EVENT_H__
#define __EVENTRPC_EVENT_H__
#include <stdio.h>
#include "eventrpc/base.h"
namespace eventrpc {
enum EventFlags {
  EVENT_READ = 0x1,
  EVENT_WRITE = 0x1 << 1,
  EVENT_NULL
};
struct EventHandler {
  virtual bool HandleRead() = 0;
  virtual bool HandleWrite() = 0;

  EventHandler() {
  }
  virtual ~EventHandler() {
  };
};
}; // namespace eventrpc
#endif  // __EVENTRPC_EVENT_H__
