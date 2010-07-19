
#ifndef __EVENTRPC_UTILITY_H__
#define __EVENTRPC_UTILITY_H__

#include <string>
#include "base.h"

using std::string;

EVENTRPC_NAMESPACE_BEGIN

uint32_t hash_string(const string &str);

int GetCpuNum();

EVENTRPC_NAMESPACE_END
#endif  //  __EVENTRPC_UTILITY_H__
