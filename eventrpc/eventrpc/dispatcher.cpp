/*
 * Copyright(C) lichuang
 */
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <list>
#include "eventrpc/thread.h"
#include "eventrpc/mutex.h"
#include "eventrpc/log.h"
#include "eventrpc/assert_log.h"
#include "eventrpc/task.h"
#include "eventrpc/dispatcher.h"
#include "eventrpc/net_utility.h"
namespace {
static const uint32 kMaxPollWaitTime = 1000;
static const uint32 kEpollFdCount = 1024;
};
namespace eventrpc {
struct EpollEvent {
  int fd;
  uint32 flags;
  struct epoll_event epoll_ev;
  EventHandler *handler;
};
struct AddEventTask : public Task {
  AddEventTask(Dispatcher::Impl *impl, EpollEvent *event)
    : impl_(impl),
      event_(event) {
  }
  virtual ~AddEventTask() {
  }

  void Handle();

  Dispatcher::Impl *impl_;
  EpollEvent *event_;
};

struct Dispatcher::Impl : public ThreadWorker {
  Impl();

  ~Impl();

  void Run();

  void AddEvent(int fd, uint32 flags, EventHandler *handler);

  void Start();

  void Stop();

  void PushTask(Task *task);

  void HandleTasks();

  void InternalAddEvent(EpollEvent *event);

  void InternalDeleteEvent(EpollEvent *event);
 private:
  typedef list<Task*> TaskList;
  TaskList task_list_[2];
  TaskList *running_task_list_;
  TaskList *free_task_list_;
  Thread thread_;
  int epoll_fd_;
  SpinMutex task_list_spin_mutex_;
  epoll_event epoll_event_array_[kEpollFdCount];
  EpollEvent* event_array_[kEpollFdCount];
  typedef list<EpollEvent*> EventList;
};

Dispatcher::Impl::Impl()
  : running_task_list_(&task_list_[0]),
    free_task_list_(&task_list_[1]),
    thread_(this),
    epoll_fd_(-1) {
  for (uint32 i = 0; i < kEpollFdCount; ++i) {
    event_array_[i] = NULL;
  }
}

Dispatcher::Impl::~Impl() {
}

void Dispatcher::Impl::Run() {
  VLOG_INFO() << "dispatcher thread start....";
  while (true) {
    // no need to lock
    if (!free_task_list_->empty()) {
      HandleTasks();
    }
    int number = epoll_wait(epoll_fd_, &epoll_event_array_[0],
                            kEpollFdCount, kMaxPollWaitTime);
    if (number == -1) {
      if (errno == EINTR) {
        continue;
      }
      VLOG_ERROR() << "epoll_wait return -1, errno: "
        << strerror(errno);
      Stop();
      return;
    }
    EpollEvent *event;
    for (int i = 0; i < number; ++i) {
      event = static_cast<EpollEvent*>(epoll_event_array_[i].data.ptr);
      if (event == NULL) {
        continue;
      }
      if (epoll_event_array_[i].events & EPOLLIN) {
        if (event->handler->HandleRead() == false) {
          InternalDeleteEvent(event);
        }
      }
      if (epoll_event_array_[i].events & EPOLLOUT) {
        if (event->handler->HandleWrite() == false) {
          InternalDeleteEvent(event);
        }
      }
    }
  }
}

void Dispatcher::Impl::HandleTasks() {
  {
    SpinMutexLock lock(&task_list_spin_mutex_);
    TaskList *tmp_task_list = running_task_list_;
    running_task_list_ = free_task_list_;
    free_task_list_ = tmp_task_list;
  }
  for (TaskList::iterator iter = running_task_list_->begin();
       iter != running_task_list_->end(); ) {
    Task *task = *iter;
    task->Handle();
    ++iter;
    delete task;
  }
  running_task_list_->clear();
}

void Dispatcher::Impl::AddEvent(int fd, uint32 flags, EventHandler *handler) {
  EpollEvent *event = new EpollEvent();
  ASSERT_TRUE(event != NULL);
  event->fd = fd;
  event->flags = flags;
  event->handler = handler;
  AddEventTask *task = new AddEventTask(this, event);
  PushTask(task);
}

void Dispatcher::Impl::InternalAddEvent(EpollEvent *event) {
  ASSERT_NE(static_cast<EpollEvent*>(NULL), event);
  if (event->flags & EVENT_READ) {
    event->epoll_ev.events |= EPOLLIN;
  }
  if (event->flags & EVENT_WRITE) {
    event->epoll_ev.events |= EPOLLOUT;
  }

  event->epoll_ev.events |= EPOLLET;
  event->epoll_ev.data.ptr = event;
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD,
                event->fd, &(event->epoll_ev)) != 0) {
    delete event;
    VLOG_ERROR() << "epoll_ctl for fd " << event->fd
      << " error: " << strerror(errno);
  }
  ASSERT_TRUE(event_array_[event->fd] == NULL);
  VLOG_INFO() << "add event, fd: " << event->fd;
  event_array_[event->fd] = event;
}

void Dispatcher::Impl::InternalDeleteEvent(EpollEvent *event) {
  ASSERT_TRUE(event_array_[event->fd] == event);
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL,
                event->fd, &(event->epoll_ev)) == -1) {
    VLOG_ERROR() << "epoll_ctl error: " << strerror(errno);
  }
  close(event->fd);
  event_array_[event->fd] = NULL;
}

void Dispatcher::Impl::PushTask(Task *task) {
  SpinMutexLock lock(&task_list_spin_mutex_);
  free_task_list_->push_back(task);
}

void Dispatcher::Impl::Start() {
  epoll_fd_ = ::epoll_create(kEpollFdCount);
  ASSERT_TRUE(epoll_fd_ != -1);
  thread_.Start();
}

void Dispatcher::Impl::Stop() {
}

void AddEventTask::Handle() {
  impl_->InternalAddEvent(event_);
}

Dispatcher::Dispatcher()
  : impl_(new Impl) {
}

Dispatcher::~Dispatcher() {
  delete impl_;
}

void Dispatcher::AddEvent(int fd, uint32 flags, EventHandler *handler) {
  impl_->AddEvent(fd, flags, handler);
}

void Dispatcher::Start() {
  impl_->Start();
}

void Dispatcher::Stop() {
  impl_->Stop();
}

void Dispatcher::PushTask(Task *task) {
  impl_->PushTask(task);
}
};
