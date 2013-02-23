// Copyright (C) 2013, avidya
// Created by anqin.qin@gmail.com

#ifndef __EVENTRPC_RPC_CONTROLLER__
#define __EVENTRPC_RPC_CONTROLLER__

#include <string>
#include <google/protobuf/service.h>
// #include <google/protobuf/stubs/common.h>

#include "eventrpc/base.h"

namespace eventrpc {

class RpcController : public gpb::RpcController {
public:
    RpcController();
    virtual ~RpcController();

    // client-end methods
    virtual void Reset();
    virtual bool Failed() const;
    virtual std::string ErrorText() const;
    virtual void StartCancel();

    // server-end methods
    virtual void SetFailed(const string& reason);
    virtual bool IsCanceled() const;
    virtual void NotifyOnCancel(gpb::Closure* callback);

private:
    std::string m_fail_reason;
};

} // namespace eventrpc

#endif // __EVENTRPC_RPC_CONTROLLER__
