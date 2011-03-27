/*
 * Copyright (C) Lichuang
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "base.h"
#include "file_utility.h"
#include "log.h"

namespace eventrpc {
bool FileUtility::ReadFileContents(const string &file,
                                   string *content) {
  ASSERT(content);
  int fd = open(file.c_str(), O_RDONLY);
  if (fd < 0) {
    LOG_ERROR() << "open file " << file << " for read error: "
      << strerror(errno);
    return false;
  }
  char buffer[100];
  int32 size = 0;
  while ((size = read(fd, buffer, sizeof(buffer))) > 0) {
    content->append(buffer, size);
  }
  return true;
}
};
