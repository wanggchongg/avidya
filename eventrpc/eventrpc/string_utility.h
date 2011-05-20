/*
 * Copyright (C) Lichuang
 *
 */
#ifndef __EVENTRPC_STRING_UTILITY_H__
#define __EVENTRPC_STRING_UTILITY_H__
#include <string>
#include "base.h"
using std::string;
namespace eventrpc {
class StringUtility {
 public:
  static string ConvertFloatToString(float value);
  static string ConvertBoolToString(bool value);
  static string ConvertUint64ToString(uint64 value);
  static string Convertint64ToString(int64 value);
  static string ConvertUint32ToString(uint32 value);
  static string Convertint32ToString(int32 value);
};
};
#endif  // __EVENTRPC_STRING_UTILITY_H__
