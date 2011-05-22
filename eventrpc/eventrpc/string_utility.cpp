/*
 * Copyright (C) Lichuang
 *
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <iostream>
#include "eventrpc/string_utility.h"
namespace {
static const uint32 kBufferLength = 20;
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

template <class Type>
static string SerializeValueToString(const Type &value) {
  char buffer[kBufferLength] = {'\0'};
  memcpy(buffer, &value, sizeof(Type));
  return string(buffer, sizeof(Type));
}

template <class Type>
Type DeserializeStringToValue(const string &input) {
  Type result;
  memcpy((char*)(&result), input.c_str(), sizeof(Type));
  return result;
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

string StringUtility::SerializeBoolToString(bool value) {
  return SerializeValueToString(value);
}

string StringUtility::SerializeUint64ToString(uint64 value) {
  return SerializeValueToString(value);
}

string StringUtility::SerializeInt64ToString(int64 value) {
  return SerializeValueToString(value);
}

string StringUtility::SerializeUint32ToString(uint32 value) {
  return SerializeValueToString(value);
}

string StringUtility::SerializeInt32ToString(int32 value) {
  return SerializeValueToString(value);
}

bool StringUtility::DeserializeStringToBool(const string &input){
  return DeserializeStringToValue<bool>(input);
}

uint64 StringUtility::DeserializeStringToUint64(const string &input) {
  return DeserializeStringToValue<uint64>(input);
}

int64 StringUtility::DeserializeStringToInt64(const string &input) {
  return DeserializeStringToValue<int64>(input);
}

uint32 StringUtility::DeserializeStringToUint32(const string &input) {
  return DeserializeStringToValue<uint32>(input);
}

int32 StringUtility::DeserializeStringToInt32(const string &input) {
  return DeserializeStringToValue<int32>(input);
}
};
