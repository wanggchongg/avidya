
#include <string.h>
#include "log/log.h"

using std::string;

EVENTRPC_NAMESPACE_BEGIN

#define COLOR_RED            "\033[0;31m"
#define COLOR_GREEN          "\033[0;32m"
#define COLOR_YELLOW         "\033[0;33m"
#define COLOR_BLUE           "\033[0;34m"
#define COLOR_PURPLE         "\033[0;35m"
#define COLOR_NONE           "\033[0m"
#define END_OF_COLOR         COLOR_NONE"\n"

LogLevel kLogLevel = INFO;
static const int kLogPathLength = 400;
char kLogPath[200];

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
  FileLogger() {
  }

  void Write();
 private:
  LogLevel log_level_;
};

class LoggerManager {
 public:
  LoggerManager() {
  }

  void Log(LogLevel log_level, const string &content);

  FileLogger file_logger_;
  static LoggerManager *logger_manager_[NUM_OF_LOG_LEVEL];
};

static LoggerManager kLogManager;

Log::~Log() {
  (this->*(log_func_))();
}

void Log::LogToStderr() {
  std::cout << kLogColor[log_level_] << file_ << ":" << line_
    << "] " << content_ << END_OF_COLOR;
}

void Log::LogToFile() {
  std::cout << kLogColor[log_level_] << "logtofile" << content_
    << END_OF_COLOR;
}

EVENTRPC_NAMESPACE_END
