
#include "log.h"
#include "dispatcher.h"
#include "net_utility.h"
#include "rpc_method_manager.h"
#include "rpc_connection.h"

namespace eventrpc {
int RpcConnection::RpcConnectionEvent::HandleRead() {
  ASSERT_NE(0, event_flags_ & EVENT_READ);
  return connection_->HandleRead();
}

int RpcConnection::RpcConnectionEvent::HandleWrite() {
  ASSERT_NE(0, event_flags_ & EVENT_WRITE);
  return connection_->HandleWrite();
}

int RpcConnection::HandleRead() {
  int recv_length = 0;;
  bool ret = false;
  ssize_t recv_count = 0;
  while (true) {
    if (expect_count_ > RpcConnection::BUFFER_LENGTH) {
      recv_count = RpcConnection::BUFFER_LENGTH;
    } else {
      recv_count = expect_count_;
    }
    ret = NetUtility::Recv(event_.fd_, buffer_,
                           recv_count, &recv_length);
    if (!ret) {
      close(event_.fd_);
      return -1;
    } else {
      message_.append(buffer_, recv_length);
      if (recv_length < recv_count) {
        return 0;
      } else if (recv_length == recv_count) {
        return StateMachine();
      }
    }
  }
  return 0;
}

void RpcConnection::Close() {
  if (event_.fd_ > 0) {
    dispatcher_->DeleteEvent(&event_);
  }
}

int RpcConnection::HandleWrite() {
  int send_length = 0;
  bool ret = false;
  while (true) {
    ret = NetUtility::Send(event_.fd_,
                           message_.c_str() + sent_count_,
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
      event_.event_flags_ = EVENT_READ;
      dispatcher_->ModifyEvent(&event_);
      return 0;
    }
  }
  return 0;
}

int RpcConnection::StateMachine() {
  switch (state_) {
    case READ_META:
      {
        meta_.Encode(message_.c_str());
        if (!rpc_method_manager_->IsServiceRegistered(meta_.method_id())) {
          close(event_.fd_);
          return -1;
        }
          expect_count_ = meta_.message_len();
          state_ = READ_MESSAGE;
          message_ = "";
      }
      break;
    case READ_MESSAGE:
      rpc_method_manager_->HandleService(&message_, &meta_, &callback_);
      break;
    default:
      LOG_FATAL() << "should not reach here";
      break;
  }
  return 0;
}

void RpcConnection::HandleServiceDone() {
  event_.event_flags_ = EVENT_WRITE;
  expect_count_ = message_.length();
  sent_count_ = 0;
  dispatcher_->ModifyEvent(&event_);
}

void RpcConnection::RpcConnectionCallback::Run() {
  connection_->HandleServiceDone();
}
};
