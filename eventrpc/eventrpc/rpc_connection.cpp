
#include "log.h"
#include "net_utility.h"
#include "rpc_method_manager.h"
#include "rpc_connection.h"

namespace eventrpc {
int RpcConnection::RpcConnectionEvent::HandleRead() {
  ASSERT_NE(0, event_flags_ & EVENT_READ);
  return connection_->HandleRead();
}

int RpcConnection::RpcConnectionEvent::HandleWrite() {
  ASSERT_NE(0, event_flags_ & EVENT_READ);
  return connection_->HandleWrite();
}

int RpcConnection::HandleRead() {
  int recv_length = 0;;
  bool ret = false;
  ssize_t recv_count = 0;
  while (true) {
    if (expect_recv_count_ > RpcConnection::BUFFER_LENGTH) {
      recv_count = RpcConnection::BUFFER_LENGTH;
    } else {
      recv_count = expect_recv_count_;
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

int RpcConnection::HandleWrite() {
  return 0;
}

int RpcConnection::StateMachine() {
  switch (state_) {
    case READ_META:
      {
        meta_.Encode(message_.c_str());
        if (!rpc_method_manager_->IsServiceRegisted(meta_.method_id())) {
          close(event_.fd_);
          return -1;
        }
          expect_recv_count_ = meta_.message_len();
          state_ = READ_MESSAGE;
          message_ = "";
      }
      break;
    case READ_MESSAGE:
      rpc_method_manager_->HandleService(&message_, &meta_);
      /*
      method_ = rpc_method_->method_;;
      request_ = rpc_method_->request_->New();
      response_ = rpc_method_->response_->New();
      request_->ParseFromString(message_);
      gpb::Closure *done = gpb::NewCallback(
          this,
          &RpcConnectionEvent::Impl::HandleServiceDone);
      rpc_method_->service_->CallMethod(method_,
                                        NULL,
                                        request_, response_, done);
                                        */
      break;
    default:
      LOG_FATAL() << "should not reach here";
      break;
  }
  return 0;
}

};
