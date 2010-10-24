
#ifndef __EVENTRPC_TIME_UTILITY_H__
#define __EVENTRPC_TIME_UTILITY_H__

#include <base/base.h>

EVENTRPC_NAMESPACE_BEGIN

// this implementation copy from glog source
typedef double WallTime;
WallTime WallTime_Now();

EVENTRPC_NAMESPACE_END

#endif  //  __EVENTRPC_TIME_UTILITY_H__
