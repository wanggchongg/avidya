#ifndef __EVENTRPC_EVENT_H__
#define __EVENTRPC_EVENT_H__

#include "base.h"

namespace eventrpc {
enum EventFlags {
  EVENT_READ = 0x1,
  EVENT_WRITE = 0x1 << 1,
  EVENT_NULL
};

struct Event {
 public:
  Event(int fd, uint32 event_flags)
    : fd_(fd),
    event_flags_(event_flags),
    entry_(NULL) {
  }

  virtual ~Event() {
  }

  virtual int HandleRead() = 0;

  virtual int HandleWrite() = 0;

 public:
  int fd_;
  uint32 event_flags_;
  typedef void* entry_t;
  entry_t entry_;
};
}; // namespace eventrpc
#endif  // __EVENTRPC_EVENT_H__
