
#ifndef __EVENTRPC_UTILITY_H__
#define __EVENTRPC_UTILITY_H__

#include <stdlib.h>
#include <string>
#include "base.h"

using std::string;

namespace eventrpc {

uint32 hash_string(const string &str);

int GetCpuNum();

const string& GetMyUserName();

};

#endif  //  __EVENTRPC_UTILITY_H__
