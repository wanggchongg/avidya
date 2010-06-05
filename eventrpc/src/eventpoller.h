
#ifndef __EVENTRPC_EVENT_POLLER_H_
#define __EVENTRPC_EVENT_POLLER_H_

#include "base.h"

EVENTRPC_NAMESPACE_BEGIN

class Event;

class EventPoller {
 public:
  EventPoller();

  ~EventPoller();

  void Loop();

  void Stop();

  void AddEvent(short event, Event *ev);

  void DelEvent(short event, Event *ev);

 private:
  struct Impl;
  Impl *impl_;
};

EVENTRPC_NAMESPACE_END

#endif  // __EVENTRPC_EVENT_POLLER_H_
