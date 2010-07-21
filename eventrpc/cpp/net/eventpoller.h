
#ifndef __EVENTRPC_EVENT_POLLER_H_
#define __EVENTRPC_EVENT_POLLER_H_

#include "base/base.h"

EVENTRPC_NAMESPACE_BEGIN

class Event;

class EventPoller {
 public:
  EventPoller();

  ~EventPoller();

  void Loop();

  void Stop();

  bool AddEvent(Event *event);

  bool DelEvent(Event *event);

 private:
  struct Impl;
  Impl *impl_;
};

EVENTRPC_NAMESPACE_END

#endif  // __EVENTRPC_EVENT_POLLER_H_
