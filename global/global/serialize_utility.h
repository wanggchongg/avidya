/*
 * Copyright (C) Lichuang
 */
#ifndef __GLOBAL_SERIALIZE_UTILITY_H__
#define __GLOBAL_SERIALIZE_UTILITY_H__
#include "global/transaction.h"
namespace global {
bool SerializeFileHeaderToString(
    const TransactionLogFileHeader &file_header,
    string *output);

bool ParseFileHeaderFromString(
    const string &input,
    TransactionLogFileHeader *file_header);

bool SerializeTransactionHeaderToString(
    const TransactionHeader &header,
    string *output);

bool ParseTransactionHeaderFromString(
    const string &input,
    TransactionHeader *header);
};
#endif  // __GLOBAL_SERIALIZE_UTILITY_H__
