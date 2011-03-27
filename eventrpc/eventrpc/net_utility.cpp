/*
 * Copyright (C) Lichuang
 *
 */
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
#include "log.h"

namespace eventrpc {

int NetUtility::Connect(const char *host, int port) {
  int fd = ::socket(AF_INET, SOCK_STREAM, 0);

  if (fd < 0) {
    return -1;
  }

  struct sockaddr_in servaddr;

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  inet_pton(AF_INET, host, &servaddr.sin_addr);

  if (::connect(fd, (struct sockaddr *)&servaddr,
                sizeof(servaddr)) < 0) {
    ::close(fd);
    return -1;
  }

  if (!NetUtility::SetNonBlocking(fd)) {
    ::close(fd);
    return -1;
  }

  return fd;
}

int NetUtility::Listen(const char *ip, int port) {
  int fd = ::socket(AF_INET, SOCK_STREAM, 0);

  if (fd < 0) {
    return -1;
  }

  int one = 1;
  if (::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                   &one, sizeof(one)) < 0) {
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

int NetUtility::Accept(int listen_fd) {
  int fd, errno_copy;
  struct sockaddr_in addr;
  socklen_t len = sizeof(addr);

  do {
    fd = ::accept(listen_fd, (struct sockaddr *)&addr, &len);
    if (fd > 0) {
      break;
    }
    errno_copy = errno;
    if (fd < 0) {
      if (errno_copy == EINTR) {
        continue;
      } else if (errno_copy == EAGAIN) {
        return -1;
      } else {
        LOG_ERROR() << "Fail to accept, "
          << " error: "
          << strerror(errno_copy);
        return -1;
      }
    }
  } while (true);

  if (!NetUtility::SetNonBlocking(fd)) {
    close(fd);
    return -1;
  }

  return fd;
}

bool NetUtility::Send(int fd, const void *buf, size_t count,
                      int *length) {
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

bool NetUtility::Recv(int fd, void *buf, size_t count, int *length) {
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

bool NetUtility::SetNonBlocking(int fd) {
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
