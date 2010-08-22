
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <iomanip>    // for setw
#include "log/log.h"
#include "concurrency/mutex.h"
#include "util/utility.h"

using std::string;
using std::ostringstream;
using std::setw;

EVENTRPC_NAMESPACE_BEGIN

#define COLOR_RED            "\033[0;31m"
#define COLOR_GREEN          "\033[0;32m"
#define COLOR_YELLOW         "\033[0;33m"
#define COLOR_NONE           "\033[0m"
#define END_OF_COLOR         COLOR_NONE"\n"

LogLevel kLogLevel = INFO;
static const int kLogPathLength = 200;
char kLogPath[kLogPathLength] = "/tmp/";

static const char *kLogLevelStr[] = {
  "INFO",
  "WARN",
  "ERROR",
  "FATAL",
  NULL
};

const char *kLogColor[] = {
  COLOR_NONE,   /* none for info */
  COLOR_GREEN,  /* green for warn*/
  COLOR_YELLOW, /* yellow for error */
  COLOR_RED,    /* red for fatal */
};

void SetLogLevel(LogLevel log_level) {
  kLogLevel = log_level;
}

void SetLogPath(const char *log_path) {
  strcpy(kLogPath, log_path);
}

class FileLogger {
 public:
  FileLogger()
    : file_(NULL) {
  }

  ~FileLogger() {
    if (file_) {
      fclose(file_);
    }
  }

  void Write(LogLevel loglevel,
             const string &log_time,
             const string &content);

 private:
  void CreateLogFile(LogLevel loglovel);

 private:
  LogLevel log_level_;
  FILE *file_;
  Mutex mutex_;
  string log_filename_;
};

void FileLogger::CreateLogFile(LogLevel loglevel) {
  if (log_filename_.empty()) {
    log_filename_ = string(kLogPath) + GetMyUserName() +
      + "." + kLogLevelStr[loglevel];
  }
  file_ = fopen(log_filename_.c_str(), "w");
}

void FileLogger::Write(LogLevel loglevel,
                       const string &log_time,
                       const string &content) {
  MutexLock lock(&mutex_);

  if (file_ == NULL) {
    CreateLogFile(loglevel);
  }

  fwrite(log_time.c_str(), 1, log_time.length(), file_);
  fwrite(content.c_str(), 1, content.length(), file_);
}

static FileLogger kFileLogger[NUM_OF_LOG_LEVEL];

Log::~Log() {
  (this->*(log_func_))();
}

void Log::Init() {
  NowTime();
  log_header_.fill('0');
  log_header_ << setw(2) << 1 + tm_time_.tm_mon
    << setw(2) << tm_time_.tm_mday
    << ' '
    << setw(2) << tm_time_.tm_hour
    << ':'
    << setw(2) << tm_time_.tm_min
    << ':'
    << setw(2) << tm_time_.tm_sec
    << '.'
    << setw(2)  << timeval_.tv_usec
    << ' '
    << file_
    << ":"
    << line_
    << "]"
    << '\0';
}

void Log::NowTime() {
  ::gettimeofday(&timeval_, 0);
  ::localtime_r(&timeval_.tv_sec, &tm_time_);
}

void Log::LogToStderr() {
  std::cout << kLogColor[log_level_] << log_header_.str()
    << input_stream_.str() << END_OF_COLOR;
}

void Log::LogToFile() {
  input_stream_ << "\n";
  kFileLogger[log_level_].Write(log_level_, log_header_.str(),
                                input_stream_.str());
}

EVENTRPC_NAMESPACE_END
