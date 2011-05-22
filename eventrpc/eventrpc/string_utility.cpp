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
static string ConvertValueToString(Type value) {
  std::stringstream stream;
  stream << value;
  return stream.str();
}

template <class Type>
static Type ConvertStringToValue(const string &input) {
  std::stringstream stream(input);
  Type result;
  stream >> result;
  return result;
}

string StringUtility::ConvertFloatToString(float value) {
  return ConvertValueToString(value);
}

string StringUtility::ConvertBoolToString(bool value) {
  return ConvertValueToString(value);
}

string StringUtility::ConvertUint64ToString(uint64 value) {
  return ConvertValueToString(value);
}

string StringUtility::ConvertInt64ToString(int64 value) {
  return ConvertValueToString(value);
}

string StringUtility::ConvertUint32ToString(uint32 value) {
  return ConvertValueToString(value);
}

string StringUtility::ConvertInt32ToString(int32 value) {
  return ConvertValueToString(value);
}

float StringUtility::ConvertStringToFloat(const string &input) {
  return ConvertStringToValue<float>(input);
}

bool StringUtility::ConvertStringToBool(const string &input) {
  return ConvertStringToValue<bool>(input);
}

uint64 StringUtility::ConvertStringToUint64(const string &input) {
  return ConvertStringToValue<uint64>(input);
}

int64 StringUtility::ConvertStringToInt64(const string &input) {
  return ConvertStringToValue<int64>(input);
}

uint32 StringUtility::ConvertStringToUint32(const string &input) {
  return ConvertStringToValue<uint32>(input);
}

int32 StringUtility::ConvertStringToInt32(const string &input) {
  return ConvertStringToValue<int32>(input);
}
};
