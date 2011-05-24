/*
 * Copyright (C) Lichuang
 */
#include <eventrpc/log.h>
#include "global/data_tree.h"
#include "global/data_node_header.h"
using namespace eventrpc;
namespace global {
struct DataTree::Impl {
 public:
  Impl();
  ~Impl();

  bool Deserialize(const string &input);
  bool Serialize(string *output);

  bool AddNode(const global::NodeData &node);
  TreeNode* GetParentNode(const string &path);
  string GetChildPath(TreeNode *parent_node, const string &path);
  TreeNode* GetNode(const string &path);
  bool DeleteNode(const string &path, uint64 gxid);
  TreeNode* SetData(const string &path, const string &data,
               uint32 version, uint32 gxid, uint64 time);
  bool GetData(const string &path, const string *data) const;
  bool GetChildren(const string &path, list<string> *children) const;
 private:
  typedef map<string, TreeNode*> NodeMap;
  NodeMap nodes_;
  TreeNode *root_node_;
};

DataTree::Impl::Impl() {
  global::NodeData node;
  node.set_path("/");
  node.set_permission(1);
  node.set_owner(2);
  global::NodeStat *stat = node.mutable_stat();
  stat->set_cgxid(1);
  stat->set_mgxid(1);
  stat->set_ctime(1);
  stat->set_mtime(1);
  stat->set_version(1);
  stat->set_cversion(1);
  stat->set_aversion(1);
  stat->set_ephemeral_owner(1);
  stat->set_pgxid(1);
  root_node_ = new TreeNode(node);
  ASSERT(root_node_ != NULL);
  nodes_["/"] = root_node_;
}

DataTree::Impl::~Impl() {
  NodeMap::iterator iter;
  for (iter = nodes_.begin(); iter != nodes_.end();) {
    TreeNode *node = iter->second;
    ++iter;
    delete node;
  }
}

bool DataTree::Impl::Deserialize(const string &input) {
  DataNodeHeader header;
  uint32 pos = 0, size = 0;
  while (true) {
    if (!header.Deserialize(input.substr(pos))) {
      break;
    }
    pos += DATA_NODE_HEADER_SIZE;
    size = header.node_size;
    global::NodeData node_data;
    if (!node_data.ParseFromString(input.substr(pos, size))) {
      return false;
    }
    AddNode(node_data);
    pos += size;
  }
  return true;
}

bool DataTree::Impl::Serialize(string *output) {
  NodeMap::iterator iter;
  for (iter = nodes_.begin(); iter != nodes_.end(); ++iter) {
    TreeNode *node = iter->second;
    DataNodeHeader header;
    header.node_size = node->node_data.ByteSize();
    string result;
    header.Serialize(&result);
    output->append(result);
    result = "";
    node->node_data.SerializeToString(&result);
    output->append(result);
  }
  return true;
}

TreeNode* DataTree::Impl::GetParentNode(const string &path) {
  string::size_type pos = path.find_last_of("/");
  if (pos == string::npos) {
    LOG_ERROR() << "path " << path << " has no parent";
    return NULL;
  }
  TreeNode *parent_node;
  if (pos == 0) {
    if (path == "/") {
      LOG_ERROR() << "root path not allow modify";
      return NULL;
    }
    parent_node = root_node_;
  } else {
    string parent_path = path.substr(0, pos);
    NodeMap::iterator iter = nodes_.find(parent_path);
    if (iter == nodes_.end()) {
      LOG_ERROR() << "path " << path << " has no parent"
        << ", parent path: " << parent_path;
      return NULL;
    }
    parent_node = iter->second;
  }
  return parent_node;
}

string DataTree::Impl::GetChildPath(TreeNode *parent_node,
                                    const string &path) {
  uint32 parent_path_length = parent_node->node_data.path().length();
  if (parent_path_length != 1) {
    ++parent_path_length;
  }
  return path.substr(parent_path_length);
}

bool DataTree::Impl::AddNode(const global::NodeData &node) {
  string path = node.path();
  NodeMap::iterator iter = nodes_.find(path);
  if (iter != nodes_.end()) {
    LOG_ERROR() << "path " << path << " node existed";
    return false;
  }
  TreeNode *parent_node = GetParentNode(path);
  if (parent_node == NULL) {
    return false;
  }
  TreeNode *tree_node = new TreeNode(node);
  ASSERT(tree_node != NULL);
  nodes_[node.path()] = tree_node;
  string child_path = GetChildPath(parent_node, path);
  tree_node->parent = parent_node;
  parent_node->children.insert(child_path);
  return true;
}

TreeNode* DataTree::Impl::GetNode(const string &path) {
  NodeMap::iterator iter = nodes_.find(path);
  if (iter == nodes_.end()) {
    LOG_ERROR() << "path " << path << " not exist";
    return NULL;
  }
  return iter->second;
}

bool DataTree::Impl::DeleteNode(const string &path, uint64 gxid) {
  TreeNode *parent_node = GetParentNode(path);
  if (NULL == parent_node) {
    return false;
  }
  NodeMap::iterator iter = nodes_.find(path);
  if (iter == nodes_.end()) {
    LOG_ERROR() << "path " << path << " not exist";
    return false;
  }
  string child_path = GetChildPath(parent_node, path);
  parent_node->children.erase(child_path);
  parent_node->node_data.mutable_stat()->set_pgxid(gxid);
  parent_node->node_data.mutable_stat()->set_cversion(
      parent_node->node_data.stat().cversion() + 1);
  nodes_.erase(iter);
  return true;
}

TreeNode* DataTree::Impl::SetData(const string &path,
                                  const string &data,
                                  uint32 version, uint32 gxid,
                                  uint64 time) {
  TreeNode *node = GetNode(path);
  if (node == NULL) {
    return NULL;
  }
  node->node_data.set_data(data);
  node->node_data.mutable_stat()->set_mtime(time);
  node->node_data.mutable_stat()->set_version(version);
  node->node_data.mutable_stat()->set_mgxid(gxid);
  return node;
}

bool DataTree::Impl::GetChildren(const string &path,
                                 list<string> *children) const {
  ASSERT(children != NULL);
  children->clear();
  NodeMap::const_iterator iter = nodes_.find(path);
  if (iter == nodes_.end()) {
    LOG_ERROR() << "path " << path << " not exist";
    return false;
  }
  const TreeNode *tree_node = iter->second;
  set<string>::iterator child_iter = tree_node->children.begin();
  for (; child_iter != tree_node->children.end(); ++child_iter) {
    children->push_back(*child_iter);
  }
  return true;
}

DataTree::DataTree()
  : impl_(new Impl) {
}

DataTree::~DataTree() {
  delete impl_;
}

bool DataTree::Deserialize(const string &input) {
  return impl_->Deserialize(input);
}

bool DataTree::Serialize(string *output) {
  return impl_->Serialize(output);
}

bool DataTree::AddNode(const global::NodeData &node) {
  return impl_->AddNode(node);
}

TreeNode* DataTree::GetNode(const string &path) {
  return impl_->GetNode(path);
}

bool DataTree::DeleteNode(const string &path, uint64 gxid) {
  return impl_->DeleteNode(path, gxid);
}

TreeNode* DataTree::SetData(const string &path, const string &data,
                       uint32 version, uint32 gxid, uint64 time) {
  return impl_->SetData(path, data, version, gxid, time);
}

bool DataTree::GetChildren(const string &path,
                           list<string> *children) const {
  return impl_->GetChildren(path, children);
}
};
