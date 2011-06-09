/*
 * Copyright(C) lichuang
 */
#include "base.h"
namespace eventrpc {
static const uint32 kSuccess = 0;
// for decode/encode message
static const uint32 kDecodeMessageError = 100;
static const uint32 kRecvMessageNotCompleted = 101;
static const uint32 kCannotFindMethodId = 102;
static const uint32 kServiceNotRegistered = 103;
static const uint32 kSendMessageNotCompleted = 104;
static const uint32 kCannotFindRequestId = 105;
static const uint32 kHandlePacketError = 106;
// for network send/recv message
static const uint32 kSendMessageError = 200;
static const uint32 kRecvMessageError = 201;
};
