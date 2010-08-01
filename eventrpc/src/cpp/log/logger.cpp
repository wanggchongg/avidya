
#include "log/logger.h"

EVENTRPC_NAMESPACE_BEGIN

LogLevel kLogLevel = INFO;

const char *kLogColor[] = {
  COLOR_NONE,   /* none for info */
  COLOR_GREEN,  /* green for warn*/
  COLOR_YELLOW, /* yellow for error */
  COLOR_RED,    /* red for fatal */
};

void SetLogLevel(LogLevel log_level) {
  kLogLevel = log_level;
}

EVENTRPC_NAMESPACE_END
