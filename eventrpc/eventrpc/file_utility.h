/*
 * Copyright (C) Lichuang
 *
 */
#ifndef __EVENTRPC_FILE_UTILITY_H__
#define __EVENTRPC_FILE_UTILITY_H__
#include <string>
using std::string;
namespace eventrpc {
class FileUtility {
 public:
  static bool ReadFileContents(const string &file, string *content);
  static bool WriteFileContents(const string &file,
                                const string &content);
};
}
#endif // __EVENTRPC_FILE_UTILITY_H__
