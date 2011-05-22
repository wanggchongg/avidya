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
  static string ConvertInt64ToString(int64 value);
  static string ConvertUint32ToString(uint32 value);
  static string ConvertInt32ToString(int32 value);

  static float  ConvertStringToFloat(const string &input);
  static bool   ConvertStringToBool(const string &input);
  static uint64 ConvertStringToUint64(const string &input);
  static int64  ConvertStringToInt64(const string &input);
  static uint32 ConvertStringToUint32(const string &input);
  static int32  ConvertStringToInt32(const string &input);
};
};
#endif  // __EVENTRPC_STRING_UTILITY_H__
