
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include "net/connectionevent.h"
#include "net/socket_utility.h"
#include "net/dispatcher.h"
#include "net/rpcserverevent.h"

EVENTRPC_NAMESPACE_BEGIN

struct ConnectionEvent::Impl {
 public:
  Impl(int fd, RpcServerEvent *server_event,
       ConnectionEvent *conn_event, Dispatcher *dispatcher)
    : fd_(fd),
      rpc_methods_(server_event->rpc_methods()),
      server_event_(server_event),
      state_(INIT),
      conn_event_(conn_event),
      dispatcher_(dispatcher) {
  }

  ~Impl() {
  }

  bool OnWrite();

  bool OnRead();

  void HandleServiceDone();

  void Init(int fd, WorkerThread *worker_thread);

  void Close();

 private:
  int fd_;
  char buf_[BUFFER_LENGTH];
  const RpcMethodMap *rpc_methods_;
  Meta meta_;
  RpcServerEvent *server_event_;
  gpb::Message *request_;
  gpb::Message *response_;
  const gpb::MethodDescriptor *method_;
  RpcMethod *rpc_method_;
  string message_;
  Request_State state_;
  ssize_t count_;
  ssize_t sent_count_;
  ConnectionEvent *conn_event_;
  Dispatcher *dispatcher_;
  WorkerThread *worker_thread_;
};

bool ConnectionEvent::Impl::OnWrite() {
  int len;
  bool ret;
  while (true) {
    ret = Send(fd_, message_.c_str() + sent_count_, count_, &len);
    if (!ret) {
      Close();
      return false;
    } else if (len < count_) {
      count_ -= len;
      sent_count_ += len;
      return true;
    } else if (len == count_) {
      // waiting for the next request
      if (!conn_event_->UpdateEvent(READ_EVENT)) {
        return false;
      }
      return true;
    }
  }

  return true;
}

bool ConnectionEvent::Impl::OnRead() {
  int len;
  bool ret;
  ssize_t recv_count;
  while (true) {
    recv_count = count_ > BUFFER_LENGTH ? BUFFER_LENGTH : count_;
    ret = Recv(fd_, buf_, recv_count, &len);
    if (!ret) {
      Close();
      return false;
    } else if (len < recv_count) {
      count_ -= len;
      message_.append(buf_, len);
      return true;
    } else if (len == recv_count) {
      message_.append(buf_, len);
      if (state_ == READ_META) {
        meta_.Encode(message_.c_str());
        state_ = READ_MESSAGE;
        RpcMethodMap::const_iterator iter;
        if ((iter = rpc_methods_->find(meta_.method_id()))
            != rpc_methods_->end()) {
          rpc_method_ = iter->second;
          count_ = meta_.message_len();
          state_ = READ_MESSAGE;
          message_ = "";
        } else {
          Close();
          return false;
        }
      } else if (state_ == READ_MESSAGE) {
        method_ = rpc_method_->method_;;
        request_ = rpc_method_->request_->New();
        response_ = rpc_method_->response_->New();
        request_->ParseFromString(message_);
        gpb::Closure *done = gpb::NewCallback(
            this,
            &ConnectionEvent::Impl::HandleServiceDone);
        rpc_method_->service_->CallMethod(method_,
                                          NULL,
                                          request_, response_, done);
        return true;
      }
    }
  }

  return false;
}

void ConnectionEvent::Impl::HandleServiceDone() {
  message_ = "";
  meta_.EncodeWithMessage(method_->full_name(), response_, &message_);
  sent_count_ = 0;
  count_ = message_.length();
  delete request_;
  delete response_;
  if (!conn_event_->UpdateEvent(WRITE_EVENT)) {
    return;
  }
  OnWrite();
}

void ConnectionEvent::Impl::Init(int fd, WorkerThread *worker_thread) {
  state_ = READ_META;
  count_ = META_LEN;
  fd_ = fd;
  conn_event_->set_fd(fd);
  worker_thread_ = worker_thread;
}

void ConnectionEvent::Impl::Close() {
  (static_cast<Event*>(conn_event_))->Close();
  worker_thread_->PushUnusedConnection(conn_event_);
}

ConnectionEvent::ConnectionEvent(int fd, RpcServerEvent *server_event,
                                 Dispatcher *dispatcher)
  : impl_(new Impl(fd, server_event, this, dispatcher)) {
  fd_ = fd;
  dispatcher_ = dispatcher;
}

ConnectionEvent::~ConnectionEvent() {
  delete impl_;
}

bool ConnectionEvent::OnWrite() {
  return impl_->OnWrite();
}

bool ConnectionEvent::OnRead() {
  return impl_->OnRead();
}

void ConnectionEvent::Init(int fd, WorkerThread *worker_thread) {
  impl_->Init(fd, worker_thread);
}

void ConnectionEvent::Close() {
  impl_->Close();
}

EVENTRPC_NAMESPACE_END
