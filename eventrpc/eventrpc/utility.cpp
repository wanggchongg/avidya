#include <stdio.h>
#include <string.h>
#include "utility.h"
#include "global_init.h"

extern uint32_t hashword(
    const uint32_t *k,
    size_t          length,
    uint32_t        initval);

namespace eventrpc {

#define HASH_INITVAL 0x31415926

uint32_t hash_string(const string &str) {
  return static_cast<uint32_t>(hashword(
      reinterpret_cast<const uint32_t*>(str.c_str()),
      str.size() / 4, HASH_INITVAL));
}

int GetCpuNum() {
  FILE *file;
  char buf[50];
  if ((file = fopen("/proc/cpuinfo", "r")) == NULL) {
    return -1;
  }
  int count = 0;
  while (fgets(buf, sizeof(buf), file) != NULL) {
    if (strstr(buf, "processor") != NULL) {
      count++;
    }
  }

  return count;
}

static string kUserName;
const string& GetMyUserName() {
  return kUserName;
}

static void MyUserNameInitlizer() {
  const char *user = getenv("USER");

  if (user != NULL) {
    kUserName = user;
  }
}
REGISTER_MODULE_INITIALIZER(utility, MyUserNameInitlizer())

};
