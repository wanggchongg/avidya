
#ifndef __EVENTRPC_LOG_H__
#define __EVENTRPC_LOG_H__

#include <sstream>
#include <iostream>
#include <string>
#include "base/base.h"
#include "base/noncopyable.h"

EVENTRPC_NAMESPACE_BEGIN

enum LogLevel {
  INFO = 0,
  WARN = 1,
  ERROR = 2,
  FATAL = 3,
  NUM_OF_LOG_LEVEL
};

extern const char *kLogColor[];
extern LogLevel kLogLevel;
extern char kLogPath[];
extern void SetLogLevel(LogLevel log_level);
extern void SetLogPath(const char *log_path);

class Log {
 public:
  typedef void (Log::*LogFunc)(void);

  Log(const std::string &content, LogLevel log_level,
      LogFunc log_func, int line, const char *file)
    : content_(content),
      log_level_(log_level),
      log_func_(log_func),
      line_(line),
      file_(file) {
  }

  ~Log();

  void LogToStderr();
  void LogToFile();

 private:
  DISALLOW_EVIL_CONSTRUCTOR(Log);

 protected:
  const std::string &content_;
  LogLevel log_level_;
  LogFunc log_func_;
  int line_;
  const char *file_;
};

#define CONSTRUCT_STRINGSTREAM(content) \
  std::ostringstream os;                                      \
  os << content

#define STDOUT_LOG(log_level, content)                       \
  do{                                                           \
    if (log_level >= eventrpc::kLogLevel) {                   \
      CONSTRUCT_STRINGSTREAM(content);   \
      Log(os.str().c_str(), log_level, &Log::LogToStderr, \
          __LINE__, __FILE__);    \
    } \
  } while(0)

#define FILE_LOG(log_level, content)                       \
  do{                                                           \
    if (log_level >= eventrpc::kLogLevel) {                   \
      CONSTRUCT_STRINGSTREAM(content);   \
      Log(os.str().c_str(), log_level, &Log::LogToFile,         \
          __LINE__, __FILE__);    \
    } \
  } while(0)

#define VLOG_INFO(content) STDOUT_LOG(eventrpc::INFO, content)
#define VLOG_WARN(content) STDOUT_LOG(eventrpc::WARN, content)
#define VLOG_ERROR(content) STDOUT_LOG(eventrpc::ERROR, content)
#define VLOG_FATAL(content) STDOUT_LOG(eventrpc::FATAL, content)

#define LOG_INFO(content) FILE_LOG(eventrpc::INFO, content)
#define LOG_WARN(content) FILE_LOG(eventrpc::WARN, content)
#define LOG_ERROR(content) FILE_LOG(eventrpc::ERROR, content)
#define LOG_FATAL(content) FILE_LOG(eventrpc::FATAL, content)

EVENTRPC_NAMESPACE_END

using eventrpc::Log;

#endif  // __EVENTRPC_LOG_H__
