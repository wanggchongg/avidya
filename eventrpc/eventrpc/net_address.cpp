/*
 * Copyright (C) Lichuang
 */
#include <strings.h>
#include "eventrpc/net_address.h"
#include "eventrpc/log.h"
#include "eventrpc/string_utility.h"
namespace eventrpc {
NetAddress::NetAddress() {
}

NetAddress::NetAddress(const string &host, int port) {
  bzero(&address_, sizeof(address_));
  address_.sin_family = AF_INET;
  EASSERT_EQ(1, inet_pton(AF_INET, host.c_str(), &(address_.sin_addr)))
    << "host: " << host;
  address_.sin_port = htons(port);
  Init();
}

NetAddress::NetAddress(struct sockaddr_in &address)
  : address_(address) {
  Init();
}

void NetAddress::Init() {
  char buffer[30];
  inet_ntop(AF_INET, &address_.sin_addr,
            buffer, sizeof(buffer));
  string port = StringUtility::ConvertInt32ToString(
      ntohs(address_.sin_port));
  debug_string_ = string(buffer) + ":" + port;
}

const string& NetAddress::DebugString() const {
  return debug_string_;
}
};
