#include "log/log.h"

int main() {
  VLOG_INFO() << "this is a info vlog " << 1;
  VLOG_WARN() << "this is a warn vlog" << 2;
  VLOG_ERROR() << "this is a error vlog";
  VLOG_FATAL() << "this is a fatal vlog";

  LOG_INFO() << "this is a info log";
  LOG_WARN() << "this is a warn log";
  LOG_ERROR() << "this is a error log";
  LOG_FATAL() << "this is a fatal log";

  return 0;
}
