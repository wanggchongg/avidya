
#ifndef __EVENTRPC_BASE_H__
#define __EVENTRPC_BASE_H__

#include <inttypes.h>
#include <sys/types.h>

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef u_int8_t uint8;
typedef u_int16_t uint16;
typedef u_int32_t uint32;
typedef u_int64_t uint64;

namespace google {
namespace protobuf {
}
}

namespace eventrpc {
namespace gpb = ::google::protobuf;
};

using namespace std;
#endif  // __EVENTRPC_BASE_H__
