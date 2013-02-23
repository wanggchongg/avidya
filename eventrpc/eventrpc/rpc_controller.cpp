// Copyright (C) 2013, avidya
// Created by anqin.qin@gmail.com

#include "rpc_controller.h"

namespace eventrpc {

RpcController::RpcController() {}

RpcController::~RpcController() {}

// client-end methods

void RpcController::Reset() {}

bool RpcController::Failed() const {
    return !m_fail_reason.empty();
}

std::string RpcController::ErrorText() const {
    return m_fail_reason;
}

void RpcController::StartCancel() {}

// server-end methods

void RpcController::SetFailed(const string& reason) {
    m_fail_reason = reason;
}

bool RpcController::IsCanceled() const {
    return false;
}

void RpcController::NotifyOnCancel(gpb::Closure* callback) {}

} // namespace eventrpc
