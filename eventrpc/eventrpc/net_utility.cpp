#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <strings.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "net_utility.h"

namespace eventrpc {

int Connect(const char *host, int port) {
  int fd = ::socket(AF_INET, SOCK_STREAM, 0);

  if (fd < 0) {
    return -1;
  }

  struct sockaddr_in servaddr;

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  inet_pton(AF_INET, host, &servaddr.sin_addr);

  if (::connect(fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    ::close(fd);
    return -1;
  }

  if (!NetUtility::SetNonBlocking(fd)) {
    ::close(fd);
    return -1;
  }

  return fd;
}

int Listen(const char *ip, int port) {
  int fd = ::socket(AF_INET, SOCK_STREAM, 0);

  if (fd < 0) {
    return -1;
  }

  int one = 1;
  if (::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0) {
    return -1;
  }
  struct sockaddr_in servaddr;

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  servaddr.sin_addr.s_addr = inet_addr(ip);

  if (!NetUtility::SetNonBlocking(fd)) {
    return -1;
  }

  if (::bind(fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    return -1;
  }

  if (::listen(fd, 10000) < 0) {
    return -1;
  }

  return fd;
}

bool Send(int fd, const void *buf, size_t count, int *length) {
  int ret = 0, errno_copy;

  *length = 0;
  do {
    ret = ::send(fd, ((char*)buf + (*length)),
                 count, MSG_NOSIGNAL | MSG_DONTWAIT);
    errno_copy = errno;
    if (ret > 0) {
      count -= ret;
      *length += ret;
    } else if (errno_copy == EINTR) {
      continue;
    } else if (errno_copy == EAGAIN) {
      return true;
    } else {
      return false;
    }
  } while (ret > 0 && count > 0);

  return true;
}

bool Recv(int fd, void *buf, size_t count, int *length) {
  int ret = 0, errno_copy;

  *length = 0;
  do {
    ret = ::recv(fd, (char*)buf + (*length), count, MSG_DONTWAIT);
    errno_copy = errno;
    if (ret > 0) {
      count -= ret;
      *length += ret;
    } else if (errno_copy == EINTR) {
      continue;
    } else if (errno_copy == EAGAIN) {
      return true;
    } else {
      return false;
    }
  } while (ret > 0 && count > 0);

  return true;
}

bool SetNonBlocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) {
    return false;
  }

  if (-1 == fcntl(fd, F_SETFL, flags | O_NONBLOCK)) {
    return false;
  }

  return true;
}

};
