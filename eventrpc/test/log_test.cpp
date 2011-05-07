
#include "log.h"

int main() {
  eventrpc::SetProgramName("test");
  VLOG_INFO() << "test vlog";
  LOG_INFO() << "test log";

  return 0;
}
