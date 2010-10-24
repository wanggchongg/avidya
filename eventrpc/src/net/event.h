
#ifndef __EVENTRPC_EVENTI_H__
#define __EVENTRPC_EVENTI_H__

#include <event.h>
#include <stdio.h>
#include <base/base.h>

#define READ_EVENT  (EV_READ  | EV_PERSIST)
#define WRITE_EVENT (EV_WRITE | EV_PERSIST)

EVENTRPC_NAMESPACE_BEGIN

class Dispatcher;

class Event {
 public:
  virtual ~Event() {
    Close();
  }

  struct event* event() {
    return &event_;
  }

  short event_flags() {
    return event_flags_;
  }

  void set_event_flags(short event_flags) {
    event_flags_ = event_flags;
  }

  bool UpdateEvent(short new_event_flags);

  int fd() {
    return fd_;
  }

  void set_fd(int fd) {
    fd_ = fd;
  }

  Dispatcher* dispatcher() {
    return dispatcher_;
  }

  void set_dispatcher(Dispatcher *dispatcher) {
    dispatcher_ = dispatcher;
  }

  virtual bool OnWrite() = 0;

  virtual bool OnRead() = 0;

  void Close();

 protected:
  Event()
    : event_flags_(-1)
    , fd_(-1)
    , dispatcher_(NULL) {
  }

 protected:
  short event_flags_;
  int fd_;
  struct event event_;
  Dispatcher *dispatcher_;
};

EVENTRPC_NAMESPACE_END

#endif  // __EVENTRPC_EVENTI_H__
