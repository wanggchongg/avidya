
#ifndef __EVENTRPC_BASE_H__
#define __EVENTRPC_BASE_H__

#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>

#define EVENTRPC_NAMESPACE_BEGIN namespace eventrpc {
#define EVENTRPC_NAMESPACE_END  };

#define BUFFER_LENGTH 100

namespace google {
  namespace protobuf {
  }
}

namespace eventrpc {
namespace gpb = ::google::protobuf;
typedef int32_t int32;
typedef u_int32_t uint32;
typedef int64_t int64;
typedef u_int64_t uint64;
};


#endif  // __EVENTRPC_BASE_H__
