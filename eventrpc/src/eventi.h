
#ifndef __EVENTRPC_EVENTI_H__
#define __EVENTRPC_EVENTI_H__

#include <event.h>
#include <stdio.h>
#include "base.h"

#define READ_EVENT  (EV_READ  | EV_PERSIST)
#define WRITE_EVENT (EV_WRITE | EV_PERSIST)

EVENTRPC_NAMESPACE_BEGIN

class EventPoller;

class Event {
 public:
  virtual ~Event() {
  }

  struct event* read_event() {
    return &read_event_;
  }

  struct event* write_event() {
    return &write_event_;
  }

  short event() {
    return event_;
  }

  int fd() {
    return fd_;
  }

  EventPoller* event_poller() {
    return event_poller_;
  }

  void set_event_poller(EventPoller *event_poller) {
    event_poller_ = event_poller;
  }

  virtual int OnWrite() = 0;

  virtual int OnRead() = 0;

  void Close();

 protected:
  Event(short event, int fd = -1)
    : event_(event)
      , fd_(fd)
      , event_poller_(NULL) {
      }

 protected:
  short event_;
  int fd_;
  struct event read_event_;
  struct event write_event_;
  EventPoller *event_poller_;
};

EVENTRPC_NAMESPACE_END

#endif  // __EVENTRPC_EVENTI_H__
