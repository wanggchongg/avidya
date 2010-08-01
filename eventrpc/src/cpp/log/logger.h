
#ifndef __EVENTRPC_LOGGER_H__
#define __EVENTRPC_LOGGER_H__

#include <sstream>
#include <iostream>
#include <string>
#include "base/base.h"
#include "base/noncopyable.h"
#include "util/singleton.h"

EVENTRPC_NAMESPACE_BEGIN

enum LogLevel {
  INFO = 0,
  WARN = 1,
  ERROR = 2,
  FATAL = 3,
  NUM_OF_LOG_LEVEL
};

#define COLOR_RED            "\033[0;31m"
#define COLOR_GREEN          "\033[0;32m"
#define COLOR_YELLOW         "\033[0;33m"
#define COLOR_BLUE           "\033[0;34m"
#define COLOR_PURPLE         "\033[0;35m"
#define COLOR_NONE           "\033[0m"
#define END_OF_COLOR         COLOR_NONE"\n"

extern const char *kLogColor[];
extern LogLevel kLogLevel;
extern void SetLogLevel(LogLevel log_level);

class Logger {
 public:
  Logger(const std::string &content, LogLevel log_level)
    : content_(content),
      log_level_(log_level) {
  }

  virtual ~Logger() {
  }

 private:
  DISALLOW_EVIL_CONSTRUCTOR(Logger);

 protected:
  const std::string &content_;
  LogLevel log_level_;
};

class StdoutLogger : public Logger {
 public:
  StdoutLogger(const std::string &content, LogLevel log_level)
    : Logger(content, log_level) {
  }

  virtual ~StdoutLogger() {
    std::cout << kLogColor[log_level_] << content_
      << END_OF_COLOR;
  }
};

#define STDOUT_LOG(log_level, content)                       \
  do{                                                           \
    if (log_level >= eventrpc::kLogLevel) {                   \
      std::ostringstream os;                                      \
      os << "[" << __FILE__ << ":" << __LINE__ << "]";       \
      os << content;                                                \
      StdoutLogger(os.str().c_str(), log_level);               \
    } \
  } while(0)

#define VLOG_INFO(content) STDOUT_LOG(eventrpc::INFO, content)
#define VLOG_WARN(content) STDOUT_LOG(eventrpc::WARN, content)
#define VLOG_ERROR(content) STDOUT_LOG(eventrpc::ERROR, content)
#define VLOG_FATAL(content) STDOUT_LOG(eventrpc::FATAL, content)

EVENTRPC_NAMESPACE_END

using eventrpc::StdoutLogger;

#endif  // __EVENTRPC_LOGGER_H__
