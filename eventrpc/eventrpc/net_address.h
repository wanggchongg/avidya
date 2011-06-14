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
  NetAddress();
  NetAddress(const string &host, int port);
  NetAddress(struct sockaddr_in &address);

  const struct sockaddr_in* address() const {
    return &address_;
  }

  const string& DebugString() const;
 private:
  void Init();

 private:
  struct sockaddr_in address_;
  string debug_string_;
};
};
#endif  // __EVENTRPC_NET_ADDRESS_H__

