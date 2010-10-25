
#ifndef __GLOBAL_FAST_LEADER_ELECTIONER_H__
#define __GLOBAL_FAST_LEADER_ELECTIONER_H__

#include <string>
#include <list>
#include "server/common.h"
#include "proto/server_config.pb.h"
#include "net/dispatcher.h"

using std::string;
using std::list;

GLOBAL_NAMESPACE_BEGIN

class FastLeaderElectioner {
 public:
  FastLeaderElectioner(uint32 server_id,
                       const list<server_config::ServerInfo> &addr_list);

  ~FastLeaderElectioner();

  void Start();

  void Stop();

 private:
  class Impl;
  Impl *impl_;
};

GLOBAL_NAMESPACE_END
#endif  //  __GLOBAL_FAST_LEADER_ELECTIONER_H__
