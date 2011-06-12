/*
 * Copyright (C) Lichuang
 */
#include <strings.h>
#include "eventrpc/net_address.h"
#include "eventrpc/assert_log.h"
namespace eventrpc {
NetAddress::NetAddress(const string &host, int port) {
  bzero(&address_, sizeof(address_));
  address_.sin_family = AF_INET;
  ASSERT_EQ(1, inet_pton(AF_INET, host.c_str(), &(address_.sin_addr)))
    << "host: " << host;
  address_.sin_port = htons(port);
}

NetAddress::NetAddress(struct sockaddr_in &address)
  : address_(address) {
}

string NetAddress::DebugString() const {
  char buffer[30];
  inet_ntop(AF_INET, &address_.sin_addr,
            buffer, sizeof(buffer));
  return string(buffer);
}
};
