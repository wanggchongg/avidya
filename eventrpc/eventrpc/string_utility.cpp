/*
 * Copyright (C) Lichuang
 *
 */
#include <stdio.h>
#include <sstream>
#include <iostream>
#include "eventrpc/string_utility.h"
namespace {
static const uint32 kBufferLength = 100;
};
namespace eventrpc {
template <class Type>
static string ConvertToString(Type value) {
  std::stringstream stream;
  stream << value;
  return stream.str();
}

string StringUtility::ConvertFloatToString(float value) {
  return ConvertToString(value);
}

string StringUtility::ConvertBoolToString(bool value) {
  return ConvertToString(value);
}

string StringUtility::ConvertUint64ToString(uint64 value) {
  return ConvertToString(value);
}

string StringUtility::Convertint64ToString(int64 value) {
  return ConvertToString(value);
}

string StringUtility::ConvertUint32ToString(uint32 value) {
  return ConvertToString(value);
}

string StringUtility::Convertint32ToString(int32 value) {
  return ConvertToString(value);
}
};
