
#include "util/global_init.h"

void init() {
}
REGISTER_MODULE_INITIALIZER(test_global_init, init())

int main() {
  return 0;
}
