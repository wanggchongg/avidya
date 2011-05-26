/*
 * Copyright (C) Lichuang
 */
#ifndef __GLOBAL_DATA_NODE_HEADER_H__
#define __GLOBAL_DATA_NODE_HEADER_H__
#include <string>
#include <eventrpc/base.h>
using namespace std;
namespace global {
struct DataNodeHeader {
  uint32 node_size;
  DataNodeHeader()
    : node_size(0) {
  }
  bool Serialize(string *output) const;
  bool Deserialize(const string &input);
};
#define DATA_NODE_HEADER_SIZE sizeof(DataNodeHeader)
};
#endif  // __GLOBAL_DATA_NODE_HEADER_H__
