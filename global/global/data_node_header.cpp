/*
 * Copyright (C) Lichuang
 */
#include <eventrpc/log.h>
#include <eventrpc/string_utility.h>
#include "global/data_node_header.h"
using namespace eventrpc;
namespace global {
bool DataNodeHeader::Serialize(string *output) {
  ASSERT(output != NULL);
  output->append(StringUtility::SerializeUint32ToString(node_size));
  return true;
}

bool DataNodeHeader::Deserialize(const string &input) {
  if (input.size() < DATA_NODE_HEADER_SIZE) {
    return false;
  }
  uint32 size = sizeof(uint32);
  node_size = StringUtility::DeserializeStringToUint32(input.substr(0, size));
  return true;
}
};
