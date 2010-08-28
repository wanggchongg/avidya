
#ifndef __EVENTRPC_LOG_H__
#define __EVENTRPC_LOG_H__

#include <sstream>
#include <iostream>
#include <string>
#include "base/base.h"
#include "base/noncopyable.h"

EVENTRPC_NAMESPACE_BEGIN

enum LogLevel {
  DEBUG1 = 0,
  DEBUG2 = 1,
  DEBUG3 = 2,
  DEBUG4 = 3,
  INFO = 4,
  WARN = 5,
  ERROR = 6,
  FATAL = 7,
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
  void FatalLogToFile();

 private:
  void Init();
  void GetNowTime();

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

#define LOG_IS_ON(log_level) (log_level >= eventrpc::kLogLevel)

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
#define LOG_FATAL() LOG(eventrpc::FATAL, &Log::FatalLogToFile)

#define LOG_DEBUG1() LOG(eventrpc::DEBUG1, &Log::LogToFile)
#define LOG_DEBUG2() LOG(eventrpc::DEBUG2, &Log::LogToFile)
#define LOG_DEBUG3() LOG(eventrpc::DEBUG3, &Log::LogToFile)
#define LOG_DEBUG4() LOG(eventrpc::DEBUG4, &Log::LogToFile)

#define LOG_INFO_IF(condition) \
  if (condition) LOG_INFO()
#define LOG_WARN_IF(condition) \
  if (condition) LOG_WARN()
#define LOG_ERROR_IF(condition) \
  if (condition) LOG_ERROR()
#define LOG_FATAL_IF(condition) \
  if (condition) LOG_FATAL()

#define LOG_INFO_IF_NOT(condition) \
  if (!(condition)) LOG_INFO()
#define LOG_WARN_IF_NOT(condition) \
  if (!(condition)) LOG_WARN()
#define LOG_ERROR_IF_NOT(condition) \
  if (!(condition)) LOG_ERROR()
#define LOG_FATAL_IF_NOT(condition) \
  if (!(condition)) LOG_FATAL()

#define LOG_DEBUG1_IF(condition) \
  if (condition) LOG_DEBUG1()
#define LOG_DEBUG2_IF(condition) \
  if (condition) LOG_DEBUG2()
#define LOG_DEBUG3_IF(condition) \
  if (condition) LOG_DEBUG3()
#define LOG_DEBUG4_IF(condition) \
  if (condition) LOG_DEBUG4()

#define LOG_DEBUG1_IF_NOT(condition) \
  if (!(condition)) LOG_DEBUG1()
#define LOG_DEBUG2_IF_NOT(condition) \
  if (!(condition)) LOG_DEBUG2()
#define LOG_DEBUG3_IF_NOT(condition) \
  if (!(condition)) LOG_DEBUG3()
#define LOG_DEBUG4_IF_NOT(condition) \
  if (!(condition)) LOG_DEBUG4()

// check condition macros
#ifdef NDEBUG
// define a null stream that actually has no effect
// cause CHECK marco has no effect in release version
#define NULL_STREAM                             \
  if (0) std::ostringstream()

#define CHECK(condition)                NULL_STREAM

#define CHECK_EQ(expected, actual)      NULL_STREAM
#define CHECK_NE(expected, actual)      NULL_STREAM
#define CHECK_LE(expected, actual)      NULL_STREAM
#define CHECK_LT(expected, actual)      NULL_STREAM
#define CHECK_GE(expected, actual)      NULL_STREAM
#define CHECK_GT(expected, actual)      NULL_STREAM
#else   // ifdef NDEBUG
#define CHECK(condition)                                \
 if (!(condition))                                      \
   LOG_FATAL() << "Check failed: " #condition "\n"

#define DEFINE_CHECK_OP_IMPL(name, op)                  \
template<class t1, class t2>                            \
inline bool                                             \
Check##name##impl(const t1& v1, const t2 &v2) {         \
  return (v1 op v2);                                    \
}

DEFINE_CHECK_OP_IMPL(EQ, ==)
DEFINE_CHECK_OP_IMPL(NE, !=)
DEFINE_CHECK_OP_IMPL(LE, <=)
DEFINE_CHECK_OP_IMPL(LT, <)
DEFINE_CHECK_OP_IMPL(GE, >=)
DEFINE_CHECK_OP_IMPL(GT, >)
#undef DEFINE_CHECK_OP_IMPL

#define CHECK_OP(name, op, val1, val2)                  \
  if (!eventrpc::Check##name##impl(val1, val2))         \
    LOG_FATAL() << "Check failed: "                     \
      << #val1 " " #op " " #val2                        \
      << "(" #val1 << " vs. " << #val2 ")\n"

#define CHECK_EQ(expected, actual)                      \
  CHECK_OP(EQ, ==, expected, actual)
#define CHECK_NE(expected, actual)                      \
  CHECK_OP(NE, !=, expected, actual)
#define CHECK_LE(expected, actual)                      \
  CHECK_OP(LE, <=, expected, actual)
#define CHECK_LT(expected, actual)                      \
  CHECK_OP(LT, <, expected, actual)
#define CHECK_GE(expected, actual)                      \
  CHECK_OP(GE, >=, expected, actual)
#define CHECK_GT(expected, actual)                      \
  CHECK_OP(GT, >, expected, actual)
#endif // ifndef NDEBUG

EVENTRPC_NAMESPACE_END

using eventrpc::Log;
using eventrpc::INFO;
using eventrpc::WARN;
using eventrpc::ERROR;
using eventrpc::FATAL;
using eventrpc::DEBUG1;
using eventrpc::DEBUG2;
using eventrpc::DEBUG3;
using eventrpc::DEBUG4;

#endif  // __EVENTRPC_LOG_H__
