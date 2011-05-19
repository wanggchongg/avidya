/*
 * Copyright (C) Lichuang
 */
#ifndef __GLOBAL_UTILITY_H__
#define __GLOBAL_UTILITY_H__
#include <eventrpc/base.h>
#include <list>
#include <string>
using namespace std;
namespace global {
bool SortFiles(const string &directory,
               const string &prefix, bool ascending,
               list<string> *files);
uint64 GetGxidOfFileName(const string &file_name,
                         const string &prefix);
}
#endif  // __GLOBAL_UTILITY_H__
