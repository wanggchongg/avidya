// Copyright (C) 2013, avidya
// Created by lichuang1982@gmail.com
// Last modified: anqin.qin@gmail.com

#include <unistd.h>
#include <errno.h>
#include <string.h>

#define USE_SOCKETPAIR

#ifndef USE_SOCKETPAIR
#include <sys/eventfd.h>
#else
#include <sys/socket.h>
#include <sys/types.h>
#endif

#include <sys/epoll.h>
#include <list>
#include "eventrpc/thread.h"
#include "eventrpc/mutex.h"
#include "eventrpc/monitor.h"
#include "eventrpc/log.h"
#include "eventrpc/task.h"
#include "eventrpc/dispatcher.h"
#include "eventrpc/net_utility.h"
namespace eventrpc {
struct EpollEvent {
  int fd;
  uint32 flags;
  struct epoll_event epoll_ev;
  EventHandler *handler;
};

struct TaskNotifyHandler : public EventHandler {
  bool HandleRead();

  bool HandleWrite() {
    return true;
  }

  TaskNotifyHandler(int event_fd, string name)
    : event_fd_(event_fd),
      name_(name) {
  }
  virtual ~TaskNotifyHandler() {
  };
  int event_fd_;
  string name_;
};

struct AddEventTask : public Task {
  AddEventTask(Dispatcher::Impl *impl, EpollEvent *event)
    : impl_(impl),
      event_(event) {
  }
  virtual ~AddEventTask() {
  }

  void Handle();

  std::string TaskName() {
      return "AddEventTask";
  }

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

  void CleanUp();

  void PushTask(Task *task);

  void NewTaskNotify();

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
#ifndef USE_SOCKETPAIR
  int event_fd_;
#else
  int event_fd_[2];
#endif
  TaskNotifyHandler *task_notify_handler_;
  SpinMutex task_list_spin_mutex_;
  epoll_event epoll_event_array_[kEpollFdCount];
  EpollEvent* event_array_[kEpollFdCount];
  bool is_running_;
  bool shut_down_;
  Monitor cleanup_monitor_;
  string name_;
};

Dispatcher::Impl::Impl()
  : running_task_list_(&task_list_[0]),
    free_task_list_(&task_list_[1]),
    thread_(this),
    epoll_fd_(-1),
#ifndef USE_SOCKETPAIR
    event_fd_(-1),
#endif
    task_notify_handler_(NULL),
    is_running_(false),
    shut_down_(false),
    name_("dispatcher") {
#ifdef USE_SOCKETPAIR
    event_fd_[0] = -1;
    event_fd_[1] = -1;
#endif
  for (uint32 i = 0; i < kEpollFdCount; ++i) {
    event_array_[i] = NULL;
  }
}

Dispatcher::Impl::~Impl() {
  Stop();
}

void Dispatcher::Impl::Run() {
  VLOG_INFO() << "dispatcher thread start....";
  is_running_ = true;
  while (!shut_down_) {
    int number = epoll_wait(epoll_fd_, &epoll_event_array_[0],
                            kEpollFdCount, kMaxPollWaitTime);
    if (number == -1) {
      if (errno == EINTR) {
        continue;
      }
      VLOG_ERROR() << "epoll_wait return -1, errno: "
        << strerror(errno);
      CleanUp();
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
    // no need to lock
    if (!free_task_list_->empty()) {
      HandleTasks();
    }
  }
  CleanUp();
}

void Dispatcher::Impl::HandleTasks() {
  {
    TaskList *tmp_task_list = running_task_list_;
    running_task_list_ = free_task_list_;
    SpinMutexLock lock(&task_list_spin_mutex_);
    free_task_list_ = tmp_task_list;
  }
  for (TaskList::iterator iter = running_task_list_->begin();
       iter != running_task_list_->end(); ) {
    Task *task = *iter;
    VLOG_INFO() << "HandleTasks: " << task->TaskName()
        << ": " << task;
    task->Handle();
    ++iter;
    delete task;
  }
  running_task_list_->clear();
}

void Dispatcher::Impl::AddEvent(int fd, uint32 flags, EventHandler *handler) {
  EpollEvent *event = new EpollEvent();
  EASSERT_TRUE(event != NULL);
  event->fd = fd;
  event->flags = flags;
  event->handler = handler;
  AddEventTask *task = new AddEventTask(this, event);
  PushTask(task);
}

void Dispatcher::Impl::InternalAddEvent(EpollEvent *event) {
  EASSERT_NE(static_cast<EpollEvent*>(NULL), event);
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
  EASSERT_TRUE(event_array_[event->fd] == NULL);
  VLOG_INFO() << "add event, fd: " << event->fd;
  event_array_[event->fd] = event;
}

void Dispatcher::Impl::InternalDeleteEvent(EpollEvent *event) {
  EASSERT_TRUE(event_array_[event->fd] == event);
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL,
                event->fd, &(event->epoll_ev)) == -1) {
    VLOG_ERROR() << "epoll_ctl error: " << strerror(errno);
  }
  close(event->fd);
  event_array_[event->fd] = NULL;
}

void Dispatcher::Impl::PushTask(Task *task) {
  if (shut_down_) {
      return;
  }
  VLOG_INFO() << "PushTask(): " << task->TaskName() << ": " << task;
  {
    SpinMutexLock lock(&task_list_spin_mutex_);
    free_task_list_->push_back(task);
  }
  // no need to lock
  if (running_task_list_->empty()) {
    NewTaskNotify();
  }
}

void Dispatcher::Impl::NewTaskNotify() {
  if (shut_down_) {
      return;
  }
  uint64 a = 1;
#ifndef USE_SOCKETPAIR
  if (::write(event_fd_, &a, sizeof(a)) != sizeof(a)) {
    VLOG_ERROR() << "write to event fd error: " << strerror(errno);
  }
#else
  if (::write(event_fd_[0], &a, sizeof(a)) != sizeof(a)) {
    VLOG_ERROR() << "write to event fd error: " << strerror(errno);
  }
#endif
}

void Dispatcher::Impl::Start() {
  epoll_fd_ = ::epoll_create(kEpollFdCount);
  EASSERT_TRUE(epoll_fd_ != -1);
#ifndef USE_SOCKETPAIR
  event_fd_ = ::eventfd(0, 0);
  EASSERT_TRUE(event_fd_ != -1);
  EASSERT_TRUE(NetUtility::SetNonBlocking(event_fd_));
  task_notify_handler_ = new TaskNotifyHandler(event_fd_, name_);
  AddEvent(event_fd_, EVENT_READ, task_notify_handler_);
#else
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, event_fd_)) {
      LOG_ERROR() << "fail to open socketpair";
      return;
  }
  EASSERT_TRUE(NetUtility::SetNonBlocking(event_fd_[0]));
  EASSERT_TRUE(NetUtility::SetNonBlocking(event_fd_[1]));
  task_notify_handler_ = new TaskNotifyHandler(event_fd_[1], name_);
  AddEvent(event_fd_[0], EVENT_READ, task_notify_handler_);
#endif
  thread_.Start();
}

void Dispatcher::Impl::Stop() {
  if (!is_running_) {
    return;
  }
  shut_down_ = true;
  // send a message to wake up the epoll thread do cleanup
  NewTaskNotify();
  // waiting cleanup work done
  while (is_running_) {
    cleanup_monitor_.Wait();
  }
  close(epoll_fd_);
#ifndef USE_SOCKETPAIR
  close(event_fd_);
#else
  close(event_fd_[0]);
  close(event_fd_[1]);
#endif
}

void Dispatcher::Impl::CleanUp() {
  for (uint32 i = 0; i < kEpollFdCount; ++i) {
    EpollEvent *event = event_array_[i];
    if (event == NULL) {
      continue;
    }
    close(event->fd);
    delete event;
  }
  list<Task*>::iterator iter;
  for (iter = running_task_list_->begin();
       iter != running_task_list_->end(); ++iter) {
    Task *task = *iter;
    task->Handle();
    delete task;
  }
  running_task_list_->clear();
  for (iter = free_task_list_->begin();
       iter != free_task_list_->end(); ++iter) {
    Task *task = *iter;

    VLOG_INFO() << "HandleTasks: " << task->TaskName() << ": " << task;
    task->Handle();
    delete task;
  }
  free_task_list_->clear();
  delete task_notify_handler_;
  is_running_ = false;
  // notify the other threads that cleanup work done
  cleanup_monitor_.NotifyAll();
}

bool TaskNotifyHandler::HandleRead() {
  uint64 a = 0;
  if (read(event_fd_, &a, sizeof(a)) != sizeof(a)) {
    VLOG_ERROR() << "Dispatcher " << name_
      << " read eventfd: " << event_fd_
      << " error:" << strerror(errno);
  }
  return true;
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
