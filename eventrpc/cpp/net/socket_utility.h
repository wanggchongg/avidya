
#ifndef __EVENTRPC_SOCKET_UTILITY_H__
#define __EVENTRPC_SOCKET_UTILITY_H__

#include "base/base.h"

EVENTRPC_NAMESPACE_BEGIN

int Connect(const char *ip, int port);

int Listen(const char *ip, int port);

bool SetNonBlocking(int fd);

bool Send(int fd, const void *buf, size_t count, int *length);

bool Recv(int fd, void *buf, size_t count, int *length);

EVENTRPC_NAMESPACE_END
#endif  //  __EVENTRPC_SOCKET_UTILITY_H__
