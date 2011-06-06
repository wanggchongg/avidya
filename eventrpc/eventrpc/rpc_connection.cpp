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
    : impl_(impl),
      handle_state(HANDLE_SERVICE) {
  }

  virtual ~RpcConnectionCallback() {
  }

  void Run();

  void Clear() {
    message_buffer = "";
    sent_count = 0;
    handle_state = HANDLE_SERVICE;
  }

  enum HandleState {
    HANDLE_SERVICE,
    HANDLE_SERVICE_DONE,
  };
  RpcConnection::Impl *impl_;
  Meta meta;
  string message_buffer;
  uint32 sent_count;
  HandleState handle_state;
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

  int SendServiceResponse(RpcConnectionCallback *callback);

  void HandleServiceDone(RpcConnectionCallback *callback);

  void HandleService(RpcConnectionCallback *callback);

  RpcConnectionEvent event_;
  struct sockaddr_in client_address_;
  RequestState state_;
  char recv_buffer_[kBufferLength];
  string recv_messsage_buffer_;
  string send_message_;
  RpcMethodManager *rpc_method_manager_;
  RpcConnectionManager *rpc_connection_manager_;
  Dispatcher *dispatcher_;
  typedef std::list<RpcConnectionCallback*> RpcCallbackList;
  RpcCallbackList free_callback_list_;
  RpcCallbackList handledone_callback_list_;
  RpcConnectionCallback* current_callback_;
};

RpcConnection::Impl::Impl()
  : event_(-1, this),
    state_(READ_META),
    recv_messsage_buffer_(""),
    send_message_(""),
    rpc_method_manager_(NULL) {
}

RpcConnection::Impl::~Impl() {
  Close();
  std::list<RpcConnectionCallback*>::iterator iter;
  for (iter = free_callback_list_.begin();
       iter != free_callback_list_.end(); ) {
    RpcConnectionCallback *callback = *iter;
    ++iter;
    delete callback;
  }
  for (iter = handledone_callback_list_.begin();
       iter != handledone_callback_list_.end(); ) {
    RpcConnectionCallback *callback = *iter;
    ++iter;
    delete callback;
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
  if (free_callback_list_.empty()) {
    return new RpcConnectionCallback(this);
  } else {
    RpcConnectionCallback *callback = free_callback_list_.front();
    free_callback_list_.pop_front();
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

int RpcConnection::Impl::SendServiceResponse(RpcConnectionCallback *callback) {
  // is no need send response?
  if (callback->message_buffer.empty()) {
    free_callback_list_.push_back(callback);
    return kSuccess;
  }
  int length = 0;
  bool ret = false;
  while (true) {
    ret = NetUtility::Send(
        event_.fd_,
        callback->message_buffer.c_str() + callback->sent_count,
        callback->message_buffer.length() - callback->sent_count,
        &length);
    if (!ret) {
      VLOG_ERROR() << "send message to "
        << inet_ntop(AF_INET, &client_address_.sin_addr,
                     recv_buffer_, sizeof(recv_buffer_))
        << ":" << ntohs(client_address_.sin_port) << " error";
      Close();
      return kSendMessageError;
    }
    callback->sent_count += length;
    if (callback->sent_count < callback->message_buffer.length()) {
      return kSendMessageNotCompleted;
    }
    VLOG_INFO() << "send service response "
      << callback->meta.request_id() << " done, count: "
      << callback->sent_count;
    callback->Clear();
    free_callback_list_.push_back(callback);
    return kSuccess;
  }
  LOG_FATAL() << "should never reach here!";
  return 0;
}

int RpcConnection::Impl::HandleWrite() {
  VLOG_INFO() << "HandleWrite";
  RpcCallbackList::iterator iter, tmp_iter;
  uint32 result = kSuccess;
  for (iter = handledone_callback_list_.begin();
       iter != handledone_callback_list_.end(); ) {
    RpcConnectionCallback *callback = *iter;
    VLOG_INFO() << "begin send response request id "
      << callback->meta.request_id();
    result = SendServiceResponse(callback);
    if (result == kSuccess) {
      tmp_iter = iter;
      ++iter;
      handledone_callback_list_.erase(tmp_iter);
      VLOG_INFO() << "send response request id "
        << callback->meta.request_id() << " done";
      continue;
    }
    if (result == kSendMessageError) {
      VLOG_INFO() << "send response request id "
        << callback->meta.request_id() << " error";
      Close();
      return result;
    }
    // when reach here, send not completed, return and
    // waiting the next send time
    return result;
  }
  return result;
}

void RpcConnectionCallback::Run() {
  if (handle_state == HANDLE_SERVICE) {
    handle_state = HANDLE_SERVICE_DONE;
    impl_->HandleService(this);
    return;
  }
  impl_->HandleServiceDone(this);
}

int RpcConnection::Impl::HandleRead() {
  int recv_length = 0;;
  bool ret = false;
  uint32 result = 0;
  while (true) {
    ret = NetUtility::Recv(event_.fd_, recv_buffer_,
                           kBufferLength, &recv_length);
    if (!ret) {
      VLOG_ERROR() << "recv message from "
        << inet_ntop(AF_INET, &client_address_.sin_addr,
                     recv_buffer_, sizeof(recv_buffer_))
        << ":" << ntohs(client_address_.sin_port) << " error";
      Close();
      return kRecvMessageError;
    }
    recv_messsage_buffer_.append(recv_buffer_, recv_length);
    result = StateMachine();
    if (result == kSuccess) {
      continue;
    }
    if (result == kRecvMessageNotCompleted) {
      recv_messsage_buffer_ = "";
      return result;
    }
    VLOG_ERROR() << "handle message from "
      << inet_ntop(AF_INET, &client_address_.sin_addr,
                   recv_buffer_, sizeof(recv_buffer_))
      << ":" << ntohs(client_address_.sin_port) << " error: " << result;
    Close();
    return result;
  }
  VLOG_FATAL() << "should not reach here";
  return kSuccess;
}

int RpcConnection::Impl::HandleReadMetaState() {
  if (recv_messsage_buffer_.length() < META_LEN) {
    return kRecvMessageNotCompleted;
  }
  Meta meta;
  meta.EncodeWithBuffer(recv_messsage_buffer_.c_str());
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
  VLOG_INFO() << "method id " << meta.method_id()
    << ", message length: " << meta.message_length()
    << ", request id: " << meta.request_id();
  state_ = READ_MESSAGE;
  recv_messsage_buffer_ = recv_messsage_buffer_.substr(META_LEN);
  return kSuccess;
}

int RpcConnection::Impl::HandleReadMessageState() {
  ASSERT(current_callback_ != NULL);
  uint32 message_length = current_callback_->meta.message_length();
  uint32 need_message_length = message_length -
    current_callback_->message_buffer.length();
  if (recv_messsage_buffer_.length() >= need_message_length) {
    current_callback_->message_buffer.append(recv_messsage_buffer_,
                                           0, need_message_length);
    state_ = READ_META;
    recv_messsage_buffer_ = recv_messsage_buffer_.substr(need_message_length);
    current_callback_->handle_state = RpcConnectionCallback::HANDLE_SERVICE;
    VLOG_INFO() << "push task request id " << current_callback_->meta.request_id();
    dispatcher_->PushTask(current_callback_);
    return kSuccess;
  }
  current_callback_->message_buffer.append(recv_messsage_buffer_);
  return kRecvMessageNotCompleted;
}

int RpcConnection::Impl::StateMachine() {
  uint32 ret;
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

void RpcConnection::Impl::HandleServiceDone(RpcConnectionCallback *callback) {
  VLOG_INFO() << "HandleServiceDone for response request id "
    << callback->meta.request_id();
  callback->handle_state = RpcConnectionCallback::HANDLE_SERVICE_DONE;
  uint32 result = SendServiceResponse(callback);
  if (result == kSuccess) {
    return;
  }
  if (result == kSendMessageError) {
    VLOG_INFO() << "send response request id "
      << callback->meta.request_id() << " error";
    Close();
    return;
  }
  // when reach here, send not completed, return and
  // waiting the next send time
  handledone_callback_list_.push_back(callback);
}

void RpcConnection::Impl::HandleService(RpcConnectionCallback *callback) {
  VLOG_INFO() << "HandleService for response request id "
    << callback->meta.request_id();
  rpc_method_manager_->HandleService(callback->message_buffer,
                                     &(callback->message_buffer),
                                     &(callback->meta),
                                     callback);
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
