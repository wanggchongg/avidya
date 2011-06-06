
#ifndef __EVENTRPC_DISPATCHER_H__
#define __EVENTRPC_DISPATCHER_H__

#include <list>
#include <vector>
#include <sys/epoll.h>
#include "eventrpc/event.h"
#include "eventrpc/thread.h"
#include "eventrpc/mutex.h"

using std::vector;

#define EPOLL_MAX_EVENTS 1024

namespace eventrpc {
class Callback;
class Dispatcher {
 public:
  Dispatcher();

  ~Dispatcher();

  void AddEvent(Event *event);

  void DeleteEvent(Event *event);

  void ModifyEvent(Event *event);

  void Start();

  void Stop();

  void PushTask(Callback *callback);
 private:
  int OperateEvents();

  int HandleTasks();

  int Poll();

  enum EventOperationType {
    EVENT_OPERATION_ADD,
    EVENT_OPERATION_DELETE,
    EVENT_OPERATION_MODIFY,
  };

  // for event information
  struct EventEntry {
    EventOperationType event_operation_type;
    Event *event;
    struct epoll_event epoll_ev;
  };

  // for Dispatcher thread
  struct DispatcherRunnable : public Runnable {
   public:
    DispatcherRunnable(Dispatcher *dispatcher)
      : dispatcher_(dispatcher) {
    }

    virtual ~DispatcherRunnable() {
    }

    void Run();

    Dispatcher *dispatcher_;
  };

  friend struct DispatcherRunnable;

  int epoll_fd_;
  typedef vector<EventEntry*> EventVector;
  EventVector retired_events_;
  EventVector operated_events_[2];
  DispatcherRunnable runnable_;
  Thread thread_;
  EventVector *current_operate_events_;
  EventVector *waiting_operate_events_;
  SpinMutex spin_mutex_;

  SpinMutex task_spin_mutex_;
  typedef std::list<Callback*> CallbackList;
  CallbackList tasks_[2];
  CallbackList *current_handle_tasks_;
  CallbackList *waiting_handle_tasks_;

  epoll_event epoll_event_buf_[EPOLL_MAX_EVENTS];
};
};

#endif  //  __EVENTRPC_DISPATCHER_H__
