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

bool FileUtility::WriteFileContents(const string &file,
                                    const string &content) {
  mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
  int fd = open(file.c_str(), O_WRONLY | O_CREAT, mode);
  if (fd < 0) {
    LOG_ERROR() << "open file " << file << " for write error: "
      << strerror(errno);
    return false;
  }
  ssize_t pos = 0, length;
  const char *ptr = content.c_str();
  while (true) {
    length = write(fd, ptr, content.length() - pos);
    if (pos + length == content.length()) {
      return true;
    }
    if (length == -1) {
      return false;
    }
    pos += length;
    ptr += length;
  }
}
};
