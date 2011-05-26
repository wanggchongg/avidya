/*
 * Copyright (C) Lichuang
 */
#include <eventrpc/log.h>
#include "global/data_tree.h"
#include "global/transaction_log_header.h"
#include "global/serialize_utility.h"
namespace global {
bool SerializeSessionList(
    const map<uint64, uint64> &session_timeouts,
    global::SessionList *session_list) {
  map<uint64, uint64>::const_iterator iter = session_timeouts.begin();
  for (; iter != session_timeouts.end(); ++iter) {
    global::Session *session = session_list->add_sessions();
    session->set_id(iter->first);
    session->set_timeout(iter->second);
  }
  return true;
}

bool DeserializeSessionList(
    const global::SessionList &session_list,
    map<uint64, uint64> *session_timeouts) {
  for (int i = 0; i < session_list.sessions_size(); ++i) {
    global::Session session = session_list.sessions(i);
    (*session_timeouts)[session.id()] = session.timeout();
  }
  return true;
}

bool DeserializeSnapLog(const string &input,
                        DataTree *data_tree,
                        map<uint64, uint64> *session_timeouts) {
  uint32 pos = 0, size = 0;
  SnapLogFileHeader header;
  ASSERT_TRUE(header.Deserialize(input));

  pos += SNAP_LOG_HEADER_SIZE;
  size = header.session_size;
  global::SessionList session_list;
  ASSERT_TRUE(session_list.ParseFromString(input.substr(pos, size)));
  ASSERT_TRUE(DeserializeSessionList(session_list, session_timeouts));
  pos += size;
  ASSERT_TRUE(data_tree->Deserialize(input.substr(pos)));
  return true;
}
};
