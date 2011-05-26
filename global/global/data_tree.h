/*
 * Copyright (C) Lichuang
 */
#ifndef __GLOBAL_DATA_TREE_H__
#define __GLOBAL_DATA_TREE_H__
#include <string>
#include <list>
#include <set>
#include <eventrpc/base.h>
#include "global/transaction.pb.h"
using namespace std;
namespace global {
struct TreeNode {
  struct TreeNode *parent;
  global::NodeData node_data;
  set<string> children;
  TreeNode(global::NodeData data)
    : parent(NULL),
      node_data(data) {
  }
};
class DataTree {
 public:
  DataTree();
  ~DataTree();

  bool Deserialize(const string &input);
  bool Serialize(string *output) const;

  bool AddNode(const global::NodeData &node);
  TreeNode* GetNode(const string &path);
  bool DeleteNode(const string &path, uint64 gxid);
  TreeNode* SetData(const string &path, const string &data,
                    uint32 version, uint32 gxid, uint64 time);
  bool GetChildren(const string &path, list<string> *children) const;

 private:
  struct Impl;
  Impl *impl_;
};
};
#endif  // __GLOBAL_DATA_TREE_H__
