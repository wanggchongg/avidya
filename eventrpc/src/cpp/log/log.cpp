
#include <string.h>
#include <stdio.h>
#include <iomanip>    // for setw
#include "log/log.h"
#include "concurrency/mutex.h"

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
             const ostringstream &time_pid_stream,
             const string &content);

 private:
  void CreateLogFile();

 private:
  LogLevel log_level_;
  FILE *file_;
  Mutex mutex_;
  string log_filename_;
};

void FileLogger::CreateLogFile() {
  file_ = fopen(log_filename_.c_str(), "w");
}

void FileLogger::Write(LogLevel loglevel,
                       const ostringstream &time_pid_stream,
                       const string &content) {
  MutexLock lock(&mutex_);

  if (log_filename_.empty()) {
    log_filename_ = string(kLogPath) + kLogLevelStr[loglevel];
  }
  if (file_ == NULL) {
    CreateLogFile();
  }

  fwrite(time_pid_stream.str().c_str(), 1,
         time_pid_stream.str().length(), file_);
  fwrite(content.c_str(), 1,
         content.length(), file_);
}

static FileLogger kFileLogger[NUM_OF_LOG_LEVEL];

Log::~Log() {
  (this->*(log_func_))();
}

void Log::Init() {
  now_ = WallTime_Now();
  timestamp_ = static_cast<time_t>(now_);
  localtime_r(&timestamp_, &tm_time);
  int usecs = static_cast<int>((now_ - timestamp_) * 1000000);
  time_pid_stream_.fill('0');
  time_pid_stream_ << setw(2) << 1+tm_time.tm_mon
    << setw(2) << tm_time.tm_mday
    << ' '
    << setw(2) << tm_time.tm_hour
    << ':'
    << setw(2) << tm_time.tm_min
    << ':'
    << setw(2) << tm_time.tm_sec
    << '.'
    << setw(2)  << usecs
    << ' '
    << file_
    << ":"
    << line_
    << "]"
    << '\0';
}

void Log::LogToStderr() {
  std::cout << kLogColor[log_level_] << time_pid_stream_.str()
    << content_ << END_OF_COLOR;
}

void Log::LogToFile() {
  kFileLogger[log_level_].Write(log_level_, time_pid_stream_,
                                content_);
}

EVENTRPC_NAMESPACE_END
