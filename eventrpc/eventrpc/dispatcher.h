
#ifndef __EVENTRPC_DISPATCHER_H__
#define __EVENTRPC_DISPATCHER_H__

#include <vector>
#include <sys/epoll.h>
#include "event.h"

using std::vector;

#define EPOLL_MAX_EVENTS 1024

namespace eventrpc {
class Dispatcher {
 public:
  Dispatcher();

  ~Dispatcher();

  void AddEvent(Event *event);

  void DeleteEvent(Event *event);

  void ModifyEvent(Event *event);

  int Poll();

 private:
  struct EventEntry {
    Event *event;
    struct epoll_event epoll_ev;
  };

  int epoll_fd_;
  typedef vector<EventEntry*> RetiredEventVector;
  RetiredEventVector retired_events_;
  epoll_event epoll_event_buf_[EPOLL_MAX_EVENTS];
};
};

#endif  //  __EVENTRPC_DISPATCHER_H__
