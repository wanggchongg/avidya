/*
 * Copyright (C) Lichuang
 */
#include <map>
#include <errno.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <eventrpc/log.h>
#include "global/utility.h"
namespace global {
bool SortFiles(const string &directory,
               const string &prefix, bool ascending,
               list<string> *files) {
  DIR  *dir = NULL;
  struct dirent *dirp = NULL;
  dir = opendir(directory.c_str());
  if (dir == NULL) {
    LOG_ERROR() << "opendir " << directory
      << " error:" << strerror(errno);
    return false;
  }
  uint64 save_gxid = 0, gxid = 0;
  map<uint64, string, greater<uint64> > descending_map;
  map<uint64, string, less<uint64> > ascending_map;
  while ((dirp = readdir(dir)) != NULL) {
    if (strncmp(dirp->d_name, prefix.c_str(), prefix.length()))
    {
      continue;
    }
    // remember to skip '.'
    gxid = atol(dirp->d_name + prefix.length() + 1);
    if (ascending) {
      ascending_map[gxid] = dirp->d_name;
    } else {
      descending_map[gxid] = dirp->d_name;
    }
  }
  closedir(dir);
  if (ascending) {
    map<uint64, string, greater<uint64> >::iterator iter;
    for (iter = ascending_map.begin();
         iter != ascending_map.end(); ++iter) {
      files->push_back(iter->second);
    }
  } else {
    map<uint64, string, less<uint64> >::iterator iter;
    for (iter = descending_map.begin();
         iter != descending_map.end(); ++iter) {
      files->push_back(iter->second);
    }
  }
  return !files->empty();
}

uint64 GetGxidOfFileName(const string &file_name,
                         const string &prefix) {
  // remember to skip '.'
  return atol(file_name.c_str() + prefix.length() + 1);
}
};
