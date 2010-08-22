
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
  DEBUG1 = 4,
  DEBUG2 = 5,
  DEBUG3 = 6,
  DEBUG4 = 7,
  NUM_OF_LOG_LEVEL
};

extern const char *kLogColor[];
extern LogLevel kLogLevel;
extern void SetLogLevel(LogLevel log_level);
extern void SetLogPath(const char *log_path);
extern void SetMaxLogFileSize(uint32 size);
extern void SetProgramName(const char *name);

class Log {
 public:
  typedef void (Log::*LogFunc)(void);

  Log(LogLevel log_level,
      LogFunc log_func, int line, const char *file)
    : log_level_(log_level),
      log_func_(log_func),
      line_(line),
      file_(file) {
      Init();
  }

  ~Log();

  std::ostringstream &stream() {
    return input_stream_;
  }

  void LogToStderr();
  void LogToFile();

 private:
  void Init();
  void NowTime();

  DISALLOW_EVIL_CONSTRUCTOR(Log);

 protected:
  LogLevel log_level_;
  LogFunc log_func_;
  int line_;
  const char *file_;
  struct timeval timeval_;
  std::ostringstream log_header_;
  time_t timestamp_;
  struct ::tm tm_time_;
  std::ostringstream input_stream_;
};

#define LOG_IS_ON(log_level) (log_level <= eventrpc::kLogLevel)

#define LOG_IF(log_level, condition, func) \
  if (condition) Log(log_level, func, __LINE__, __FILE__).stream()

#define LOG(log_level, func) LOG_IF(log_level, LOG_IS_ON(log_level), func)

#define VLOG_INFO() LOG(eventrpc::INFO, &Log::LogToStderr)
#define VLOG_WARN() LOG(eventrpc::WARN, &Log::LogToStderr)
#define VLOG_ERROR() LOG(eventrpc::ERROR, &Log::LogToStderr)
#define VLOG_FATAL() LOG(eventrpc::FATAL, &Log::LogToStderr)

#define LOG_INFO() LOG(eventrpc::INFO, &Log::LogToFile)
#define LOG_WARN() LOG(eventrpc::WARN, &Log::LogToFile)
#define LOG_ERROR() LOG(eventrpc::ERROR, &Log::LogToFile)
#define LOG_FATAL() LOG(eventrpc::FATAL, &Log::LogToFile)

#define LOG_DEBUG1() LOG(eventrpc::DEBUG1, &Log::LogToFile)

EVENTRPC_NAMESPACE_END

using eventrpc::Log;

#endif  // __EVENTRPC_LOG_H__
