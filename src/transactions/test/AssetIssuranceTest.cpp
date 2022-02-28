// Copyright 2022 Stellar Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#include "database/Database.h"
#include "ledger/LedgerManager.h"
#include "ledger/LedgerTxn.h"
#include "ledger/LedgerTxnEntry.h"
#include "ledger/LedgerTxnHeader.h"
#include "ledger/TrustLineWrapper.h"
#include "lib/catch.hpp"
#include "main/Application.h"
#include "main/Config.h"
#include "test/TestAccount.h"
#include "test/TestExceptions.h"
#include "test/TestMarket.h"
#include "test/TestUtils.h"
#include "test/TxTests.h"
#include "test/test.h"
#include "transactions/ChangeTrustOpFrame.h"
#include "transactions/MergeOpFrame.h"
#include "transactions/PaymentOpFrame.h"
#include "transactions/TransactionUtils.h"
#include "util/Logging.h"
#include "util/Timer.h"

using namespace stellar;
using namespace stellar::txtest;
using namespace std;

TEST_CASE("issue asset", "[tx][issueasset]")
{
    Config cfg = getTestConfig();

    VirtualClock clock;
    auto app = createTestApplication(clock, cfg);

    // set up world
    auto root = TestAccount::createRoot(*app);
    cout << "Root: " << root.getAccountId() << endl;

    auto const& lm = app->getLedgerManager();
    auto const minBalance2 = lm.getLastMinBalance(3);
    auto gateway = root.create("gw84393", minBalance2);
    cout << "Gateway: " << gateway.getAccountId() << endl;

    Asset idr = makeAsset(gateway, "IDR");

    SECTION("payment")
    {        
        {
            root.changeTrust(idr, 100);
            gateway.pay(root, idr, 90); // issue 90 units of "IDR" via payment

            InternalLedgerKey key = InternalLedgerKey::makeAmountIssuedKey(idr);
            LedgerTxn ltx(app->getLedgerTxnRoot());
            auto ile = ltx.load(key).currentGeneralized();
            std::cout << ile.amountIssuedEntry().amount << std::endl;
            REQUIRE(ile.amountIssuedEntry().amount == (uint64_t)90);
        }

        /* TODO: why does this complain about already has child? */
        {
            root.pay(gateway, idr, 40); // burn 40 units of "IDR" via payment

            InternalLedgerKey key = InternalLedgerKey::makeAmountIssuedKey(idr);
            LedgerTxn ltx(app->getLedgerTxnRoot());            
            auto ile = ltx.load(key).currentGeneralized();
            std::cout << ile.amountIssuedEntry().amount << std::endl;
            REQUIRE(ile.amountIssuedEntry().amount == (uint64_t)50);
        }
        
    }

    SECTION("issue amount greater than INT64_MAX")
    {
        auto a1 = root.create("A", minBalance2);
        auto b1 = root.create("B", minBalance2);
        root.changeTrust(idr, INT64_MAX);
        a1.changeTrust(idr, INT64_MAX);
        b1.changeTrust(idr, INT64_MAX);
        gateway.pay(root, idr, INT64_MAX);
        gateway.pay(a1, idr, INT64_MAX);
        gateway.pay(b1, idr, INT64_MAX);
        
        InternalLedgerKey key = InternalLedgerKey::makeAmountIssuedKey(idr);
        LedgerTxn ltx(app->getLedgerTxnRoot());
        auto entry = ltx.load(key);
        std::cout << entry.currentGeneralized().amountIssuedEntry().amount << std::endl;
        //REQUIRE(entry.currentGeneralized().amountIssuedEntry().amount == 27670116110564327421);
    }

    SECTION("claimable balance")
    {
        // acc2 is authorized
        root.changeTrust(idr, INT64_MAX);

        ClaimPredicate pred;
        pred.type(CLAIM_PREDICATE_BEFORE_ABSOLUTE_TIME).absBefore() = INT64_MAX;

        // TODO: refactor?
        auto makeClaimant = [](AccountID const& account, ClaimPredicate const& pred) {
            Claimant c;
            c.v0().destination = account;
            c.v0().predicate = pred;
            return c;
        };
        ClaimableBalanceID claimableBalanceID;
        { 
            xdr::xvector<Claimant, 10> validClaimants{makeClaimant(root, pred), makeClaimant(gateway, pred)};
            claimableBalanceID = gateway.createClaimableBalance(idr, 75, validClaimants); // issue asset by creating claimable balance

            InternalLedgerKey key = InternalLedgerKey::makeAmountIssuedKey(idr);
            LedgerTxn ltx(app->getLedgerTxnRoot());
            auto ile = ltx.load(key).currentGeneralized();
            std::cout << ile.amountIssuedEntry().amount << std::endl;
            REQUIRE(ile.amountIssuedEntry().amount == (uint64_t)75);
        }
        {
            gateway.claimClaimableBalance(claimableBalanceID);
            InternalLedgerKey key = InternalLedgerKey::makeAmountIssuedKey(idr);
            LedgerTxn ltx(app->getLedgerTxnRoot());            
            auto ile = ltx.load(key).currentGeneralized();
            std::cout << ile.amountIssuedEntry().amount << std::endl;
            REQUIRE(ile.amountIssuedEntry().amount == (uint64_t)0);
        }
    }
}

/*
TEST_CASE("issue asset", "[tx][manageoffer]")
{

}

TEST_CASE("issue asset", "[tx][liquiditypool]")
{

}

TEST_CASE("issue asset", "[tx][clawback]")
{

}

TEST_CASE("issue asset", "[tx][clawbackclaimablebalance]")
{

}

TEST_CASE("issue asset", "[tx][authrevokedontrustlinetoliquiditypool]")
{

}
*/