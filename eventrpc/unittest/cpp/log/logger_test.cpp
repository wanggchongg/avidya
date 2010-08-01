#include "log/logger.h"

int main() {
  VLOG_INFO("this is a info vlog" << "\n");
  VLOG_WARN("this is a warn vlog" << "\n");
  VLOG_ERROR("this is a error vlog" << "\n");
  VLOG_FATAL("this is a fatal vlog" << "\n");

  return 0;
}
