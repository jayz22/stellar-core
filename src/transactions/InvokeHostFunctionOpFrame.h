#pragma once

// Copyright 2022 Stellar Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#include "xdr/Stellar-transaction.h"
#include <medida/metrics_registry.h>
#ifdef ENABLE_NEXT_PROTOCOL_VERSION_UNSAFE_FOR_PRODUCTION
#include "rust/RustBridge.h"
#include "transactions/OperationFrame.h"

namespace stellar
{
class AbstractLedgerTxn;

static constexpr ContractDataType CONTRACT_INSTANCE_CONTRACT_DATA_TYPE =
    EXCLUSIVE;

class InvokeHostFunctionOpFrame : public OperationFrame
{
    InvokeHostFunctionResult&
    innerResult()
    {
        return mResult.tr().invokeHostFunctionResult();
    }

    void maybePopulateDiagnosticEvents(Config const& cfg,
                                       InvokeHostFunctionOutput const& output);

    InvokeHostFunctionOp const& mInvokeHostFunction;

  public:
    InvokeHostFunctionOpFrame(Operation const& op, OperationResult& res,
                              TransactionFrame& parentTx);

    ThresholdLevel getThresholdLevel() const override;

    bool isOpSupported(LedgerHeader const& header) const override;

    bool doApply(AbstractLedgerTxn& ltx) override;
    bool doApply(Application& app, AbstractLedgerTxn& ltx,
                 Hash const& sorobanBasePrngSeed) override;

    bool doCheckValid(SorobanNetworkConfig const& config,
                      uint32_t ledgerVersion) override;
    bool doCheckValid(uint32_t ledgerVersion) override;

    void
    insertLedgerKeysToPrefetch(UnorderedSet<LedgerKey>& keys) const override;

    static InvokeHostFunctionResultCode
    getInnerCode(OperationResult const& res)
    {
        return res.tr().invokeHostFunctionResult().code();
    }

    virtual bool isSoroban() const override;

    struct HostFunctionMetrics
    {
        medida::MetricsRegistry& mMetrics;

        size_t mReadEntry{0};
        size_t mWriteEntry{0};

        size_t mLedgerReadByte{0};
        size_t mLedgerWriteByte{0};

        size_t mReadKeyByte{0};
        size_t mWriteKeyByte{0};

        size_t mReadDataByte{0};
        size_t mWriteDataByte{0};

        size_t mReadCodeByte{0};
        size_t mWriteCodeByte{0};

        size_t mEmitEvent{0};
        size_t mEmitEventByte{0};

        size_t mCpuInsn{0};
        size_t mMemByte{0};

        size_t mMetadataSizeByte{0};

        bool mSuccess{false};

        HostFunctionMetrics(medida::MetricsRegistry& metrics);

        bool isCodeKey(LedgerKey const& lk);

        void noteReadEntry(LedgerKey const& lk, size_t n);

        void noteWriteEntry(LedgerKey const& lk, size_t n);

        medida::TimerContext getExecTimer();
    };
};
}
#endif // ENABLE_NEXT_PROTOCOL_VERSION_UNSAFE_FOR_PRODUCTION
