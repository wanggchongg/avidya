/*
 * Copyright (C) Lichuang
 */
#ifndef __GLOBAL_SERIALIZE_UTILITY_H__
#define __GLOBAL_SERIALIZE_UTILITY_H__
#include <map>
#include <eventrpc/base.h>
#include "global/transaction.pb.h"
using namespace std;
namespace global {
class DataTree;
bool SerializeSessionList(
    const map<uint64, uint64> &session_timeouts,
    global::SessionList *session_list);
bool DeserializeSessionList(
    const global::SessionList &session_list,
    map<uint64, uint64> *session_timeouts);
bool DeserializeSnapLog(const string &input,
                        DataTree *data_tree,
                        map<uint64, uint64> *session_timeouts);
};
#endif  // __GLOBAL_SERIALIZE_UTILITY_H__
