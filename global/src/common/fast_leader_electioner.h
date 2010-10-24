
#ifndef __GLOBAL_FAST_LEADER_ELECTIONER_H__
#define __GLOBAL_FAST_LEADER_ELECTIONER_H__

#include <string>
#include <list>
#include "common/common.h"

using std::string;
using std::list;
using std::pair;

GLOBAL_NAMESPACE_BEGIN

typedef pair<string, uint32> ServerAddrPair;

class FastLeaderElectioner {
 public:
  FastLeaderElectioner(string host, uint32 port,
                       list<ServerAddrPair> addr_pair_list);

  void Start();

  void Stop();

 private:
  string host_;
  uint32 port_;
  list<ServerAddrPair> addr_pair_list_;
};

GLOBAL_NAMESPACE_END
#endif  //  __GLOBAL_FAST_LEADER_ELECTIONER_H__
