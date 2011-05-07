/*
 * Copyright (C) Lichuang
 *
 */
#ifndef __EVENTRPC_NET_UTILITY_H__
#define __EVENTRPC_NET_UTILITY_H__
struct sockaddr_in;
namespace eventrpc {
class NetUtility {
 public:
  static int Connect(const char *host, int port);

  static int Listen(const char *ip, int port);

  static int Accept(int listen_fd, struct sockaddr_in *addr);

  static bool SetNonBlocking(int fd);

  static bool Send(int fd, const void *buf,
                   size_t count, int *length);

  static bool Recv(int fd, void *buf,
                   size_t count, int *length);
};
};
#endif  //  __EVENTRPC_NET_UTILITY_H__
