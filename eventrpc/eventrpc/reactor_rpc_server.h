#ifndef __EVENTRPC_REACTOR_RPC_SERVER_H__
#define __EVENTRPC_REACTOR_RPC_SERVER_H__

#include "rpc_server.h"

namespace eventrpc {
class ReactorRpcServer
  : public RpcServer {
 public:
  ReactorRpcServer();

  virtual ~ReactorRpcServer();

  virtual void Run();

 protected:
  virtual int HandleAccept();
};
}; // namespace eventrpc
#endif // __EVENTRPC_REACTOR_RPC_SERVER_H__
