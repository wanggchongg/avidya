#include "log/log.h"

int main() {
  VLOG_INFO() << "this is a info vlog " << 1;
  VLOG_WARN() << "this is a warn vlog" << 2;
  VLOG_ERROR() << "this is a error vlog";
  VLOG_FATAL() << "this is a fatal vlog";

  eventrpc::SetProgramName("log_test");
  LOG_INFO() << "this is a info log";
  LOG_WARN() << "this is a warn log";
  LOG_ERROR() << "this is a error log";

  LOG_DEBUG1_IF(1 == 1) << "this log will be print";
  LOG_DEBUG1_IF(1 == 0) << "this log will not be print";
  LOG_DEBUG1_IF_NOT(1 == 0) << "this log will be print too";

  eventrpc::SetMaxLogFileSize(10);
  sleep(2);
  LOG_INFO() << "this is another info log";

  eventrpc::SetMaxLogFileSize(1000000);
  LOG_INFO_IF(1 == 1) << "this log will be print";
  LOG_INFO_IF(1 == 0) << "this log will not be print";

  eventrpc::SetLogLevel(INFO);
  LOG_DEBUG1() << "this log will not be print";
  eventrpc::SetLogLevel(DEBUG1);
  LOG_DEBUG1() << "this log will be print";

  CHECK(1 == 0);
  CHECK_EQ(1, 0);
  CHECK_GE(1, 0);
  CHECK_LE(1, 0);

  return 0;
}
