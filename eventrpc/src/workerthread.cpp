#include <pthread.h>
#include <unistd.h>
#include <event.h>
#include <vector>
#include "workerthread.h"
#include "eventpoller.h"
#include "socket_utility.h"
#include "rpcserverevent.h"

using namespace std;

EVENTRPC_NAMESPACE_BEGIN

static void *worker_thread(void *arg);

struct NotifyEvent : public Event {
 public:
  NotifyEvent(int fd, EventPoller *event_poller) {
    fd_ = fd;
    event_poller_ = event_poller;
  }

  virtual ~NotifyEvent() {
  }

  virtual int OnWrite() {
    return -1;
  }

  virtual int OnRead() {
    return -1;
  }
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

  int Start();

  bool PushNewConnection(int fd);

  void PushUnusedConnection(ConnectionEvent *conn_event);

 private:
  ConnectionEvent* GetConnectionEvent(int fd);

 private:
  int notify_recv_fd_;
  int notify_send_fd_;
  pthread_mutex_t lock_;
  vector<ConnectionEvent*> active_conn_;
  vector<ConnectionEvent*> unused_conn_;
  RpcServerEvent *server_event_;
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

  pthread_mutex_lock(&lock_);
  if (unused_conn_.empty()) {
    conn_event = new ConnectionEvent(fd, server_event_, &event_poller_);
  } else {
    conn_event = unused_conn_.front();
    unused_conn_.erase(unused_conn_.begin());
  }

  pthread_mutex_unlock(&lock_);

  return conn_event;
}

bool WorkerThreadImpl::PushNewConnection(int fd) {
  SetNonBlocking(fd);

  ConnectionEvent *conn_event = GetConnectionEvent(fd);
  conn_event->Init(fd, worker_thread_);
  if (!conn_event->UpdateEvent(READ_EVENT)) {
    return false;
  }
  active_conn_.push_back(conn_event);
  return true;
}

void WorkerThreadImpl::PushUnusedConnection(ConnectionEvent *conn_event) {
  pthread_mutex_lock(&lock_);
  unused_conn_.push_back(conn_event);
  pthread_mutex_unlock(&lock_);
}

int WorkerThreadImpl::Start() {
  int fds[2];
  if (pipe(fds)) {
    return -1;
  }

  notify_recv_fd_ = fds[0];
  notify_send_fd_ = fds[1];

  notify_event_ = new NotifyEvent(notify_recv_fd_, &event_poller_);
  if (!notify_event_->UpdateEvent(READ_EVENT)) {
    return -1;
  }

  pthread_mutex_init(&lock_, NULL);
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

WorkerThread::WorkerThread(RpcServerEvent *server_event)
  : impl_(new Impl(server_event, this)) {
}

WorkerThread::~WorkerThread() {
  delete impl_;
}

int WorkerThread::Start() {
  return impl_->impl_->Start();
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
