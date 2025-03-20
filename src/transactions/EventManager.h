#pragma once

#include "xdr/Stellar-ledger-entries.h"
#include "xdr/Stellar-ledger.h"
#include "xdr/Stellar-transaction.h"
#include "main/Config.h"

// Copyright 2025 Stellar Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

namespace stellar
{

class EventManager 
{

    private:
        
        // function to handle memo logic

        // TransactionFrame const& mParentTx;
        // reference to operation frame
        xdr::xvector<ContractEvent> mContractEvents;
        xdr::xvector<DiagnosticEvent> mDiagnosticEvents;

    public:

        // contract: asset, topics: ["transfer", from:Address, to:Address, sep0011_asset:String], data: { amount:i128 }
        void newTransferEvent(Hash const& networkID, Asset const& asset, MuxedAccount const& from, MuxedAccount const& to, int64 amount, Memo const& memo);

        // other classic events such as mint, burn, clawback

        // flush into meta, handle both classic and soroban

        // handle retroactive fixing of soroban events

        // a free static method for handling fees

        void pushContractEvents(xdr::xvector<ContractEvent> const& evts);

        void pushDiagnosticEvents(xdr::xvector<DiagnosticEvent> const& evts);

        void pushSimpleDiagnosticError(Config const& cfg, SCErrorType ty,
                                         SCErrorCode code,
                                         std::string&& message,
                                         xdr::xvector<SCVal>&& args);
        
        void pushApplyTimeDiagnosticError(Config const& cfg, SCErrorType ty,
                                        SCErrorCode code, std::string&& message,
                                        xdr::xvector<SCVal>&& args = {});
        
        void pushValidationTimeDiagnosticError(Config const& cfg, SCErrorType ty,
                                            SCErrorCode code,
                                            std::string&& message,
                                            xdr::xvector<SCVal>&& args = {});                                         

        xdr::xvector<DiagnosticEvent> const&
        getDiagnosticEvents() const
        {
            return mDiagnosticEvents;
        }

        xdr::xvector<ContractEvent>&& flushContractEvents() {
            return std::move(mContractEvents);
        };

        xdr::xvector<DiagnosticEvent>&& flushDiagnosticEvents() {
            return std::move(mDiagnosticEvents);
        };

};

}