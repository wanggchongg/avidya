/*
 * Copyright (C) Lichuang
 */
#ifndef __EVENTRPC_NET_ADDRESS_H__
#define __EVENTRPC_NET_ADDRESS_H__
#include <string>
#include <arpa/inet.h>
#include "eventrpc/base.h"
namespace eventrpc {
class NetAddress {
 public:
  NetAddress(const string &host, int port);
  NetAddress(struct sockaddr_in &address);

  string DebugString() const;
 private:
  struct sockaddr_in address_;
};
};
#endif  // __EVENTRPC_NET_ADDRESS_H__

