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
static void thread_libevent_process(int fd, short which, void *arg);

struct WorkerThreadImpl {
  friend void thread_libevent_process(int fd, short which, void *arg);
 public:
  WorkerThreadImpl(RpcServerEvent *server_event)
    : server_event_(server_event)
      , event_base_(NULL) {
  }

  ~WorkerThreadImpl();

  int Start();

  void PushNewConnection(int fd);

 private:
  typedef vector<ConnectionEvent*> ConnVec;
  ConnVec conn_vec_[2];
  ConnVec *get_vec_;
  ConnVec *put_vec_;

  int notify_recv_fd_;
  int notify_send_fd_;
  ConnVec conn_;
  RpcServerEvent *server_event_;
  EventPoller event_poller_;
  struct event_base *event_base_;
  struct event notify_event_;
};

struct WorkerThread::Impl {
 public:
  Impl(RpcServerEvent *server_event)
    : impl_(new WorkerThreadImpl(server_event)) {
  }

  WorkerThreadImpl *impl_;
};

void WorkerThreadImpl::PushNewConnection(int fd) {
  SetNonBlocking(fd);

  ConnectionEvent *conn_event = new ConnectionEvent(
      fd,
      server_event_->rpc_methods(),
      server_event_,
      &event_poller_);
  conn_event->Init();
  event_poller_.AddEvent(READ_EVENT, conn_event);
  conn_.push_back(conn_event);
}

int WorkerThreadImpl::Start() {
  int fds[2];
  if (pipe(fds)) {
    return -1;
  }

  notify_recv_fd_ = fds[0];
  notify_send_fd_ = fds[1];

  event_base_ = event_init();
  if (!event_base_) {
    return -1;
  }

  /* Listen for notifications from other threads */
  event_set(&notify_event_, notify_recv_fd_,
            EV_READ | EV_PERSIST, thread_libevent_process, this);
  event_base_set(event_base_, &notify_event_);

  if (event_add(&notify_event_, 0) == -1) {
    return -1;
  }

  pthread_t       thread;
  pthread_attr_t  attr;
  int             ret;

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, 1);

  if ((ret = pthread_create(&thread, &attr, worker_thread, event_base_)) != 0) {
    fprintf(stderr, "Can't create thread: %s\n",
            strerror(ret));
    return -1;
  }

  return 0;
}

WorkerThread::WorkerThread(RpcServerEvent *server_event)
  : impl_(new Impl(server_event)) {
}

WorkerThread::~WorkerThread() {
  delete impl_;
}

int WorkerThread::Start() {
  return impl_->impl_->Start();
}

void WorkerThread::PushNewConnection(int fd) {
  impl_->impl_->PushNewConnection(fd);
}

void thread_libevent_process(int fd, short which, void *arg) {
  //WorkerThreadImpl *impl = (WorkerThreadImpl*)(arg);
  char buf[1];

  if (read(fd, buf, 1) != 1) {
      return;
  }
  fd = fd;
  which = which;
  arg = arg;
}

void *worker_thread(void *arg) {
  event_base *base = (event_base*)arg;

  event_base_loop(base, 0);
  return NULL;
}

EVENTRPC_NAMESPACE_END
