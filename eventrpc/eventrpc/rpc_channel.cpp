#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "rpc_channel.h"
#include "net_utility.h"
#include "dispatcher.h"
#include "log.h"

namespace eventrpc {
RpcChannel::RpcChannel(const char *host, int port,
                       Dispatcher *dispatcher)
  : host_(host),
    port_(port),
    dispatcher_(dispatcher),
    event_(-1, this),
    response_(NULL),
    done_(NULL) {
}

RpcChannel::~RpcChannel() {
  Close();
}

bool RpcChannel::Connect() {
  event_.fd_ = NetUtility::Connect(host_, port_);
  return (event_.fd_ > 0);
}

void RpcChannel::Close() {
  if (event_.fd_ > 0) {
    dispatcher_->DeleteEvent(&event_);
  }
}

void RpcChannel::CallMethod(const gpb::MethodDescriptor* method,
                            gpb::RpcController* controller,
                            const gpb::Message* request,
                            gpb::Message* response,
                            gpb::Closure* done) {
  meta_.EncodeWithMessage(method->full_name(), request, &message_);
  state_ = SEND_REQUEST;
  count_ = message_.length();
  sent_count_ = 0;
  method_id_ = meta_.method_id();
  response_ = response;
  done_ = done;
  dispatcher_->AddEvent(&event_);
}

int RpcChannel::HandleRead() {
  if (state_ < SEND_REQUEST) {
    return false;
  } else if (state_ == SEND_REQUEST) {
    state_ = READ_META;
    count_ = META_LEN;
  }

  int len;
  bool ret;
  ssize_t recv_count;
  while (true) {
    recv_count = count_ > BUFFER_LENGTH ? BUFFER_LENGTH : count_;
    ret = NetUtility::Recv(event_.fd_, buf_, recv_count, &len);
    if (!ret) {
      Close();
      return -1;
    } else if (len < recv_count) {
      count_ -= len;
      message_.append(buf_, len);
      return 0;
    } else if (len == recv_count) {
      if (state_ == READ_META) {
          meta_.Encode(message_.c_str());
          count_ = meta_.message_len();
          state_ = READ_MESSAGE;
          if (meta_.method_id() != method_id_) {
            Close();
            return -1;
          }
          message_ = "";
      } else if (state_ == READ_MESSAGE) {
          message_.append(buf_, len);
          response_->ParseFromString(message_);
          done_->Run();
          // whether close depends on user
          // Close();
          return 0;
      }
    }
  }

  return -1;
}

int RpcChannel::RpcChannelEvent::HandleRead() {
  return channel_->HandleRead();
}

int RpcChannel::HandleWrite() {
  int len;
  bool ret;
  while (true) {
    ret = NetUtility::Send(event_.fd_, message_.c_str() + sent_count_,
                           count_, &len);
    if (!ret) {
      Close();
      return -1;
    } else if (len < count_) {
      count_ -= len;
      sent_count_ += len;
      return 0;
    } else if (len == count_) {
      event_.event_flags_ = EVENT_READ;
      dispatcher_->ModifyEvent(&event_);
      return 0;
    }
  }

  return 0;
}

int RpcChannel::RpcChannelEvent::HandleWrite() {
  return channel_->HandleWrite();
}
};
