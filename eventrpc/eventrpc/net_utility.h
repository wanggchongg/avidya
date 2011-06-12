/*
 * Copyright (C) Lichuang
 *
 */
#ifndef __EVENTRPC_NET_UTILITY_H__
#define __EVENTRPC_NET_UTILITY_H__
#include <string>
using std::string;
struct sockaddr_in;
namespace eventrpc {
class NetUtility {
 public:
  static int Connect(const string &host, int port);

  static int Listen(const char *ip, int port);

  static bool Accept(int listen_fd,
                     struct sockaddr_in *addr,
                     int *fd);

  static bool SetNonBlocking(int fd);

  static bool Send(int fd, const void *buf,
                   size_t count, int *length);

  static bool Recv(int fd, void *buf,
                   size_t count, int *length);
};
};
#endif  //  __EVENTRPC_NET_UTILITY_H__
