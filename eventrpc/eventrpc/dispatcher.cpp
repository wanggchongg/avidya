#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "log.h"
#include "callback.h"
#include "dispatcher.h"
#include "net_utility.h"
namespace {
static const uint32 kMaxPollWaitTime = 1000;
static const uint32 kEpollFdCount = 1024;
};
namespace eventrpc {
Dispatcher::Dispatcher()
  : runnable_(this),
    thread_(&runnable_),
    current_operate_events_(&operated_events_[0]),
    waiting_operate_events_(&operated_events_[1]),
    current_handle_tasks_(&tasks_[0]),
    waiting_handle_tasks_(&tasks_[1]) {
}

Dispatcher::~Dispatcher() {
  close(epoll_fd_);
}

void Dispatcher::Start() {
  epoll_fd_ = epoll_create(kEpollFdCount);
  ASSERT_NE(-1, epoll_fd_);
  thread_.Start();
}

void Dispatcher::Stop() {
}

void Dispatcher::PushTask(Callback *callback) {
  SpinMutexLock lock(&task_spin_mutex_);
  waiting_handle_tasks_->push_back(callback);
}

void Dispatcher::AddEvent(Event *event) {
  EventEntry *event_entry = new EventEntry();
  ASSERT_NE(static_cast<EventEntry*>(NULL), event_entry);
  event_entry->event = event;
  event->entry_ = static_cast<Event::entry_t>(event_entry);
  if (event->event_flags_ & EVENT_READ) {
    event_entry->epoll_ev.events |= EPOLLIN;
  }
  if (event->event_flags_ & EVENT_WRITE) {
    event_entry->epoll_ev.events |= EPOLLOUT;
  }

  event_entry->epoll_ev.events |= EPOLLET;
  event_entry->epoll_ev.data.ptr = event_entry;
  event_entry->event_operation_type = EVENT_OPERATION_ADD;
  SpinMutexLock lock(&spin_mutex_);
  waiting_operate_events_->push_back(event_entry);
}

void Dispatcher::DeleteEvent(Event *event) {
  EventEntry *event_entry = static_cast<EventEntry*>(event->entry_);
  ASSERT_NE(-1, epoll_ctl(epoll_fd_, EPOLL_CTL_DEL,
                          event->fd_, &(event_entry->epoll_ev)));
  close(event->fd_);
  event->fd_ = -1;
  event_entry->event = NULL;
  event_entry->event_operation_type = EVENT_OPERATION_DELETE;
  SpinMutexLock lock(&spin_mutex_);
  waiting_operate_events_->push_back(event_entry);
}

void Dispatcher::ModifyEvent(Event *event) {
  EventEntry *event_entry = static_cast<EventEntry*>(event->entry_);
  event_entry->epoll_ev.events = 0;
  if (event->event_flags_ & EVENT_READ) {
    event_entry->epoll_ev.events |= EPOLLIN;
  }
  if (event->event_flags_ & EVENT_WRITE) {
    event_entry->epoll_ev.events |= EPOLLOUT;
  }
  event_entry->event_operation_type = EVENT_OPERATION_MODIFY;
  SpinMutexLock lock(&spin_mutex_);
  waiting_operate_events_->push_back(event_entry);
}

int Dispatcher::Poll() {
  while (true) {
    // no need to lock
    if (!waiting_operate_events_->empty()) {
      OperateEvents();
    }
    // no need to lock
    if (!waiting_handle_tasks_->empty()) {
      HandleTasks();
    }
    int number = epoll_wait(epoll_fd_, &epoll_event_buf_[0],
                            EPOLL_MAX_EVENTS, kMaxPollWaitTime);
    if (number == -1) {
      if (errno == EINTR) {
        continue;
      }
      VLOG_ERROR() << "epoll_wait return -1, errno: "
        << strerror(errno);
      return -1;
    }
    EventEntry *event_entry;
    Event *event;
    for (int i = 0; i < number; ++i) {
      event_entry = static_cast<EventEntry*>(epoll_event_buf_[i].data.ptr);
      event = event_entry->event;
      if (event == NULL) {
        continue;
      }
      if (event_entry->event_operation_type == EVENT_OPERATION_DELETE) {
        continue;
      }
      if (epoll_event_buf_[i].events & EPOLLIN) {
        event->HandleRead();
      }
      if (epoll_event_buf_[i].events & EPOLLOUT) {
        event->HandleWrite();
      }
    }
    for (EventVector::iterator iter = retired_events_.begin();
         iter != retired_events_.end(); ++iter) {
      delete (*iter);
    }
    retired_events_.clear();
    break;
  }
  return 0;
}

void Dispatcher::DispatcherRunnable::Run() {
  while (dispatcher_->Poll() == 0) {
    ;
  }
}

int Dispatcher::HandleTasks() {
  {
    SpinMutexLock lock(&task_spin_mutex_);
    CallbackList *tmp_task_list = current_handle_tasks_;
    current_handle_tasks_ = waiting_handle_tasks_;
    waiting_handle_tasks_ = tmp_task_list;
  }

  for (CallbackList::iterator iter = current_handle_tasks_->begin();
       iter != current_handle_tasks_->end(); ++iter) {
    VLOG_INFO() << "handle task request id ";
    (*iter)->Run();
  }
  current_handle_tasks_->clear();
  return 0;
}

int Dispatcher::OperateEvents() {
  {
    SpinMutexLock lock(&spin_mutex_);
    EventVector *tmp_event_vector = current_operate_events_;
    current_operate_events_ = waiting_operate_events_;
    waiting_operate_events_ = tmp_event_vector;
  }

  EventEntry *event_entry;
  Event *event;
  for (EventVector::iterator iter = current_operate_events_->begin();
       iter != current_operate_events_->end(); ++iter) {
    event_entry = *iter;
    event = event_entry->event;
    switch (event_entry->event_operation_type) {
      case EVENT_OPERATION_ADD:
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD,
                      event->fd_, &(event_entry->epoll_ev)) != 0) {
          VLOG_ERROR() << "epoll_ctl for fd " << event->fd_ << " error";
        }
        break;
      case EVENT_OPERATION_DELETE:
        retired_events_.push_back(event_entry);
        break;
      case EVENT_OPERATION_MODIFY:
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD,
                      event->fd_, &(event_entry->epoll_ev)) != 0) {
          VLOG_ERROR() << "epoll_ctl for fd " << event->fd_ << " error";
        }
        break;
      default:
        break;
    }
  }
  current_operate_events_->clear();
  return 0;
}
};
