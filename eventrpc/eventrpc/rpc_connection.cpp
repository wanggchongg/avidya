/*
 * Copyright(C) lichuang
 */
#include <map>
#include <list>
#include <string>
#include "error_code.h"
#include "log.h"
#include "event.h"
#include "meta.h"
#include "callback.h"
#include "dispatcher.h"
#include "net_utility.h"
#include "rpc_method_manager.h"
#include "rpc_connection.h"

using std::string;
namespace {
static const int kBufferLength = 100;
}
namespace eventrpc {
enum RequestState {
  READ_META,
  READ_MESSAGE,
};

struct RpcConnectionEvent : public Event {
  RpcConnectionEvent(int fd, RpcConnection::Impl *impl)
    : Event(fd, EVENT_READ | EVENT_WRITE),
      impl_(impl) {
  }

  virtual ~RpcConnectionEvent() {
  }

  int HandleRead();

  int HandleWrite();

  RpcConnection::Impl *impl_;
};

struct RpcConnectionCallback : public Callback {
 public:
  RpcConnectionCallback(RpcConnection::Impl *impl)
    : impl_(impl) {
  }

  virtual ~RpcConnectionCallback() {
  }

  void Run();

  void Clear() {
    recv_message = "";
    send_message = "";
    sent_count = 0;
  }

  RpcConnection::Impl *impl_;
  Meta meta;
  string recv_message;
  string send_message;
  ssize_t sent_count;
};

struct RpcConnection::Impl {
  Impl();

  ~Impl();

  void set_fd(int fd);

  void set_client_address(struct sockaddr_in address);

  void set_rpc_method_manager(RpcMethodManager *rpc_method_manager);

  void set_rpc_connection_manager(
      RpcConnectionManager *rpc_connection_manager);

  void set_dispacher(Dispatcher *dispatcher);

  Event* event();

  void Close();

  RpcConnectionCallback *get_callback();

  int HandleRead();

  int HandleWrite();

  int StateMachine();

  int HandleReadMetaState();

  int HandleReadMessageState();

  void HandleServiceDone();

  RpcConnectionCallback callback_;
  RpcConnectionEvent event_;
  struct sockaddr_in client_address_;
  RequestState state_;
  char recv_buffer_[kBufferLength];
  string recv_messsage_buffer_;
  string send_message_;
  ssize_t expect_count_;
  ssize_t sent_count_;
  RpcMethodManager *rpc_method_manager_;
  RpcConnectionManager *rpc_connection_manager_;
  Dispatcher *dispatcher_;
  std::list<RpcConnectionCallback*> callback_list_;
  RpcConnectionCallback* current_callback_;
};

RpcConnection::Impl::Impl()
  : event_(-1, this),
    callback_(this),
    state_(READ_META),
    recv_messsage_buffer_(""),
    send_message_(""),
    rpc_method_manager_(NULL) {
}

RpcConnection::Impl::~Impl() {
  Close();
  std::list<RpcConnectionCallback*>::iterator iter;
  for (iter = callback_list_.begin();
       iter != callback_list_.end(); ) {
    RpcConnectionCallback *callback = *iter;
    delete callback;
    ++iter;
  }
}

void RpcConnection::Impl::set_fd(int fd) {
  event_.fd_ = fd;
}

void RpcConnection::Impl::set_client_address(struct sockaddr_in address) {
  client_address_ = address;
}

void RpcConnection::Impl::set_rpc_method_manager(
    RpcMethodManager *rpc_method_manager) {
  rpc_method_manager_ = rpc_method_manager;
}

void RpcConnection::Impl::set_rpc_connection_manager(
    RpcConnectionManager *rpc_connection_manager) {
  rpc_connection_manager_ = rpc_connection_manager;
}

void RpcConnection::Impl::set_dispacher(Dispatcher *dispatcher) {
  dispatcher_ = dispatcher;
}

RpcConnectionCallback *RpcConnection::Impl::get_callback() {
  if (callback_list_.empty()) {
    return new RpcConnectionCallback(this);
  } else {
    RpcConnectionCallback *callback = callback_list_.front();
    callback_list_.pop_front();
    return callback;
  }
}

Event* RpcConnection::Impl::event() {
  return &event_;
}

void RpcConnection::Impl::Close() {
  if (event_.fd_ > 0) {
    VLOG_INFO() << "close connection from "
      << inet_ntop(AF_INET, &client_address_.sin_addr,
                   recv_buffer_, sizeof(recv_buffer_))
      << ":" << ntohs(client_address_.sin_port);
    dispatcher_->DeleteEvent(&event_);
  }
}

int RpcConnection::Impl::HandleWrite() {
  return 0;
  int send_length = 0;
  bool ret = false;
  while (true) {
    ret = NetUtility::Send(event_.fd_,
                           send_message_.c_str() + sent_count_,
                           expect_count_, &send_length);
    if (!ret) {
      Close();
      return -1;
    } else if (send_length < expect_count_) {
      sent_count_ += send_length;
      expect_count_ -= send_length;
      return 0;
    } else if (send_length == expect_count_) {
      // waiting for the next request
      //event_.event_flags_ = EVENT_READ;
      //dispatcher_->ModifyEvent(&event_);
      return 0;
    }
  }
  return 0;
}


void RpcConnectionCallback::Run() {
  impl_->HandleServiceDone();
}

int RpcConnection::Impl::HandleRead() {
  int recv_length = 0;;
  bool ret = false;
  int result = 0;
  while (true) {
    ret = NetUtility::Recv(event_.fd_, recv_buffer_,
                           kBufferLength, &recv_length);
    if (!ret) {
      VLOG_ERROR() << "recv message from "
        << inet_ntop(AF_INET, &client_address_.sin_addr,
                     recv_buffer_, sizeof(recv_buffer_))
        << ":" << ntohs(client_address_.sin_port) << " error";
      Close();
      return -1;
    }
    recv_messsage_buffer_.append(recv_buffer_, recv_length);
    result = StateMachine();
    if (result != kSuccess && result != kRecvMessageNotCompleted) {
      return result;
    }
  }
  return kSuccess;
}

int RpcConnection::Impl::HandleReadMetaState() {
  if (recv_messsage_buffer_.length() < META_LEN) {
    return kRecvMessageNotCompleted;
  }
  Meta meta;
  meta.Encode(recv_messsage_buffer_.c_str());
  if (!rpc_method_manager_->IsServiceRegistered(meta.method_id())) {
    VLOG_ERROR() << "method id " << meta.method_id()
      << " not registered";
    Close();
    return kServiceNotRegistered;
  }
  current_callback_ = get_callback();
  ASSERT(current_callback_ != NULL);
  current_callback_->Clear();
  current_callback_->meta = meta;
  VLOG_INFO() << "method id " << meta.method_id();
  state_ = READ_MESSAGE;
  recv_messsage_buffer_ = recv_messsage_buffer_.substr(META_LEN);
  return kSuccess;
}

int RpcConnection::Impl::HandleReadMessageState() {
  ASSERT(current_callback_ != NULL);
  uint32 message_length = current_callback_->meta.message_length();
  uint32 need_message_length = message_length -
    current_callback_->recv_message.length();
  if (recv_messsage_buffer_.length() >= need_message_length) {
    current_callback_->recv_message.append(recv_messsage_buffer_,
                                           0, need_message_length);
    state_ = READ_META;
    recv_messsage_buffer_ = recv_messsage_buffer_.substr(need_message_length);
    VLOG_INFO() << "recv message: " << current_callback_->recv_message;
    return kSuccess;
  }
  current_callback_->recv_message.append(recv_messsage_buffer_);
  recv_messsage_buffer_ = "";
  return kRecvMessageNotCompleted;
}

int RpcConnection::Impl::StateMachine() {
  int32 ret;
  while (true) {
    if (state_ == READ_META) {
      ret = HandleReadMetaState();
      if (ret != kSuccess) {
        return ret;
      }
      continue;
    }
    if (state_ == READ_MESSAGE) {
      ret = HandleReadMessageState();
      if (ret != kSuccess) {
        return ret;
      }
      continue;
    }
  }
  return 0;
}

void RpcConnection::Impl::HandleServiceDone() {
  //event_.event_flags_ = EVENT_WRITE;
  expect_count_ = send_message_.length();
  sent_count_ = 0;
  //dispatcher_->ModifyEvent(&event_);
}

RpcConnection::RpcConnection() {
  impl_ = new Impl();
}

RpcConnection::~RpcConnection() {
  delete impl_;
}

void RpcConnection::set_fd(int fd) {
  impl_->set_fd(fd);
}

void RpcConnection::set_client_address(struct sockaddr_in address) {
  impl_->set_client_address(address);
}

void RpcConnection::set_rpc_method_manager(
    RpcMethodManager *rpc_method_manager) {
  impl_->set_rpc_method_manager(rpc_method_manager);
}

void RpcConnection::set_rpc_connection_manager(
    RpcConnectionManager *rpc_connection_manager) {
  impl_->set_rpc_connection_manager(rpc_connection_manager);
}

void RpcConnection::set_dispacher(Dispatcher *dispatcher) {
  impl_->set_dispacher(dispatcher);
}

int RpcConnectionEvent::HandleRead() {
  return impl_->HandleRead();
}

int RpcConnectionEvent::HandleWrite() {
  return impl_->HandleWrite();
}

Event* RpcConnection::event() {
  return impl_->event();
}

void RpcConnection::Close() {
  impl_->Close();
}
};
