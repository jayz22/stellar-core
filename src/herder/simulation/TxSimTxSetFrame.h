// Copyright 2019 Stellar Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "herder/TxSetFrame.h"

namespace stellar
{
namespace txsimulation
{

TxSetFrameConstPtr
makeSimTxSetFrame(Hash const& networkID, Hash const& previousLedgerHash,
                  std::vector<TransactionEnvelope> const& transactions,
                  std::vector<TransactionResultPair> const& results,
                  uint32_t multiplier);

}
}
