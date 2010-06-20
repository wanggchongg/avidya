#include <pthread.h>
#include <unistd.h>
#include <sched.h>
#include <event.h>
#include <vector>
#include "workerthread.h"
#include "mutex.h"
#include "eventpoller.h"
#include "socket_utility.h"
#include "rpcserverevent.h"

using namespace std;

EVENTRPC_NAMESPACE_BEGIN

static void *worker_thread(void *arg);

class WorkerThreadImpl;

struct NotifyEvent : public Event {
 public:
  NotifyEvent(int fd, EventPoller *event_poller,
              WorkerThreadImpl *worker_thread_impl)
    : worker_thread_impl_ (worker_thread_impl){
    fd_ = fd;
    event_poller_ = event_poller;
  }

  virtual ~NotifyEvent() {
  }

  virtual bool OnWrite() {
    return true;
  }

  virtual bool OnRead();

 private:
  WorkerThreadImpl *worker_thread_impl_;
};

struct WorkerThreadImpl {
 public:
  WorkerThreadImpl(RpcServerEvent *server_event, WorkerThread *worker_thread)
    : server_event_(server_event)
      , worker_thread_(worker_thread)
      , notify_event_(NULL) {
  }

  ~WorkerThreadImpl() {
    delete notify_event_;
  }

  bool Start(int cpuno);

  bool PushNewConnection(int fd);

  void PushUnusedConnection(ConnectionEvent *conn_event);

  bool HandleNewEvent();

 private:
  ConnectionEvent* GetConnectionEvent(int fd);

 private:
  int notify_recv_fd_;
  int notify_send_fd_;
  vector<ConnectionEvent*> active_conn_;
  vector<ConnectionEvent*> unused_conn_;
  vector<int> fd_vec_;
  RpcServerEvent *server_event_;
  Mutex mutex_;
  EventPoller event_poller_;
  WorkerThread *worker_thread_;
  NotifyEvent *notify_event_;
};

struct WorkerThread::Impl {
 public:
  Impl(RpcServerEvent *server_event, WorkerThread *worker_thread)
    : impl_(new WorkerThreadImpl(server_event, worker_thread)) {
  }

  WorkerThreadImpl *impl_;
};

ConnectionEvent* WorkerThreadImpl::GetConnectionEvent(int fd) {
  ConnectionEvent *conn_event;

  if (unused_conn_.empty()) {
    conn_event = new ConnectionEvent(fd, server_event_, &event_poller_);
  } else {
    conn_event = unused_conn_.front();
    unused_conn_.erase(unused_conn_.begin());
  }

  return conn_event;
}

bool WorkerThreadImpl::PushNewConnection(int fd) {
  MutexLock lock(&mutex_);
  fd_vec_.push_back(fd);
  write(notify_send_fd_, "a", 1);
  return true;
}

void WorkerThreadImpl::PushUnusedConnection(ConnectionEvent *conn_event) {
  unused_conn_.push_back(conn_event);
}

bool WorkerThreadImpl::HandleNewEvent() {
  int fd;
  MutexLock lock(&mutex_);
  while (!fd_vec_.empty()) {
    fd = fd_vec_.back();
    fd_vec_.pop_back();
    SetNonBlocking(fd);
    ConnectionEvent *conn_event = GetConnectionEvent(fd);
    conn_event->Init(fd, worker_thread_);
    if (!conn_event->UpdateEvent(READ_EVENT)) {
      return false;
    }
    active_conn_.push_back(conn_event);
  }

  return true;
}

bool WorkerThreadImpl::Start(int cpuno) {
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(cpuno, &mask);
  sched_setaffinity(0, sizeof(mask), &mask);
  int fds[2];
  if (pipe(fds)) {
    return -1;
  }

  notify_recv_fd_ = fds[0];
  notify_send_fd_ = fds[1];

  notify_event_ = new NotifyEvent(notify_recv_fd_, &event_poller_, this);
  if (!notify_event_->UpdateEvent(READ_EVENT)) {
    return -1;
  }

  pthread_t       thread;
  pthread_attr_t  attr;
  int             ret;

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, 1);

  ret = pthread_create(&thread, &attr, worker_thread, &event_poller_);
  if (ret != 0) {
    return -1;
  }

  return 0;
}

bool NotifyEvent::OnRead() {
  char buf[1];
  read(fd_, buf, 1);
  worker_thread_impl_->HandleNewEvent();
  return true;
}

WorkerThread::WorkerThread(RpcServerEvent *server_event)
  : impl_(new Impl(server_event, this)) {
}

WorkerThread::~WorkerThread() {
  delete impl_;
}

bool WorkerThread::Start(int cpuno) {
  return impl_->impl_->Start(cpuno);
}

bool WorkerThread::PushNewConnection(int fd) {
  return impl_->impl_->PushNewConnection(fd);
}

void WorkerThread::PushUnusedConnection(ConnectionEvent *conn_event) {
  impl_->impl_->PushUnusedConnection(conn_event);
}

void *worker_thread(void *arg) {
  EventPoller *event_poller = static_cast<EventPoller*>(arg);

  event_poller->Loop();

  return NULL;
}

EVENTRPC_NAMESPACE_END
