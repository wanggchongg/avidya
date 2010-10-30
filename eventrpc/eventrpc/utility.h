
#ifndef __EVENTRPC_UTILITY_H__
#define __EVENTRPC_UTILITY_H__

#include <string>
#include <eventrpc/base.h>

using std::string;

namespace eventrpc {

uint32_t hash_string(const string &str);

int GetCpuNum();

const string& GetMyUserName();

};

#endif  //  __EVENTRPC_UTILITY_H__
