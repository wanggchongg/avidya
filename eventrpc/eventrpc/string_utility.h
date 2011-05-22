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
  static string ConvertBoolToString(bool value);
  static string ConvertUint64ToString(uint64 value);
  static string ConvertInt64ToString(int64 value);
  static string ConvertUint32ToString(uint32 value);
  static string ConvertInt32ToString(int32 value);

  static bool   ConvertStringToBool(const string &input);
  static uint64 ConvertStringToUint64(const string &input);
  static int64  ConvertStringToInt64(const string &input);
  static uint32 ConvertStringToUint32(const string &input);
  static int32  ConvertStringToInt32(const string &input);

  static string SerializeBoolToString(bool value);
  static string SerializeUint64ToString(uint64 value);
  static string SerializeInt64ToString(int64 value);
  static string SerializeUint32ToString(uint32 value);
  static string SerializeInt32ToString(int32 value);

  static bool   DeserializeStringToBool(const string &input);
  static uint64 DeserializeStringToUint64(const string &input);
  static int64  DeserializeStringToInt64(const string &input);
  static uint32 DeserializeStringToUint32(const string &input);
  static int32  DeserializeStringToInt32(const string &input);
};
};
#endif  // __EVENTRPC_STRING_UTILITY_H__
