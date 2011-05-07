/*
 * Copyright(C) lichuang
 */
#include "base.h"
namespace eventrpc {
static const uint32 kSuccess = 0;
// for decode/encode message
static const uint32 kDecodeMessageError = 11;
static const uint32 kRecvMessageNotCompleted = 12;
static const uint32 kCannotFindMethodId = 13;
static const uint32 kServiceNotRegistered = 14;
};
