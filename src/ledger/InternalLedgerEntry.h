// Copyright 2020 Stellar Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "lib/util/uint128_t.h"
#include "xdr/Stellar-ledger-entries.h"
#include "xdr/Stellar-ledger.h"

namespace stellar
{

enum class InternalLedgerEntryType
{
    LEDGER_ENTRY,
    SPONSORSHIP,
    SPONSORSHIP_COUNTER,
    AMOUNT_ISSUED,
};

struct SponsorshipKey
{
    AccountID sponsoredID;
};

struct SponsorshipCounterKey
{
    AccountID sponsoringID;
};

struct AmountIssuedKey
{
    Asset asset;
};

class InternalLedgerKey
{
  private:
    size_t mutable mHash;
    InternalLedgerEntryType mType;
    union
    {
        LedgerKey mLedgerKey;
        SponsorshipKey mSponsorshipKey;
        SponsorshipCounterKey mSponsorshipCounterKey;
        AmountIssuedKey mAmountIssuedKey;
    };

    void assign(InternalLedgerKey const& glk);
    void assign(InternalLedgerKey&& glk);
    void construct();
    void destruct();

    void checkDiscriminant(InternalLedgerEntryType expected) const;

    void type(InternalLedgerEntryType t);
    LedgerKey& ledgerKeyRef();
    SponsorshipKey& sponsorshipKeyRef();
    SponsorshipCounterKey& sponsorshipCounterKeyRef();
    AmountIssuedKey& amountIssuedKeyRef();

  public:
    static InternalLedgerKey makeSponsorshipKey(AccountID const& sponsoredId);
    static InternalLedgerKey
    makeSponsorshipCounterKey(AccountID const& sponsoringId);
    static InternalLedgerKey makeAmountIssuedKey(Asset const& asset);

    InternalLedgerKey();
    explicit InternalLedgerKey(InternalLedgerEntryType t);

    InternalLedgerKey(LedgerKey const& lk);
    explicit InternalLedgerKey(SponsorshipKey const& sk);
    explicit InternalLedgerKey(SponsorshipCounterKey const& sck);
    explicit InternalLedgerKey(AmountIssuedKey const& aik);

    InternalLedgerKey(InternalLedgerKey const& glk);
    InternalLedgerKey(InternalLedgerKey&& glk);

    InternalLedgerKey& operator=(InternalLedgerKey const& glk);
    InternalLedgerKey& operator=(InternalLedgerKey&& glk);

    ~InternalLedgerKey();

    InternalLedgerEntryType type() const;

    LedgerKey const& ledgerKey() const;

    SponsorshipKey const& sponsorshipKey() const;

    SponsorshipCounterKey const& sponsorshipCounterKey() const;

    AmountIssuedKey const& amountIssuedKey() const;

    std::string toString() const;

    size_t hash() const;
};

struct SponsorshipEntry
{
    AccountID sponsoredID;
    AccountID sponsoringID;
};

struct SponsorshipCounterEntry
{
    AccountID sponsoringID;
    int64_t numSponsoring;
};

struct AmountIssuedEntry
{
    Asset asset;
    uint128_t amount;
};

class InternalLedgerEntry
{
  private:
    InternalLedgerEntryType mType;
    union
    {
        LedgerEntry mLedgerEntry;
        SponsorshipEntry mSponsorshipEntry;
        SponsorshipCounterEntry mSponsorshipCounterEntry;
        AmountIssuedEntry mAmountIssuedEntry;
    };

    void assign(InternalLedgerEntry const& gle);
    void assign(InternalLedgerEntry&& gle);
    void construct();
    void destruct();

    void checkDiscriminant(InternalLedgerEntryType expected) const;

  public:
    InternalLedgerEntry();
    explicit InternalLedgerEntry(InternalLedgerEntryType t);

    InternalLedgerEntry(LedgerEntry const& le);
    explicit InternalLedgerEntry(SponsorshipEntry const& se);
    explicit InternalLedgerEntry(SponsorshipCounterEntry const& sce);
    explicit InternalLedgerEntry(AmountIssuedEntry const& aie);

    InternalLedgerEntry(InternalLedgerEntry const& gle);
    InternalLedgerEntry(InternalLedgerEntry&& gle);

    InternalLedgerEntry& operator=(InternalLedgerEntry const& gle);
    InternalLedgerEntry& operator=(InternalLedgerEntry&& gle);

    ~InternalLedgerEntry();

    void type(InternalLedgerEntryType t);
    InternalLedgerEntryType type() const;

    LedgerEntry& ledgerEntry();
    LedgerEntry const& ledgerEntry() const;

    SponsorshipEntry& sponsorshipEntry();
    SponsorshipEntry const& sponsorshipEntry() const;

    SponsorshipCounterEntry& sponsorshipCounterEntry();
    SponsorshipCounterEntry const& sponsorshipCounterEntry() const;

    AmountIssuedEntry& amountIssuedEntry();
    AmountIssuedEntry const& amountIssuedEntry() const;

    InternalLedgerKey toKey() const;

    std::string toString() const;
};

bool operator==(SponsorshipKey const& lhs, SponsorshipKey const& rhs);
bool operator!=(SponsorshipKey const& lhs, SponsorshipKey const& rhs);
bool operator==(SponsorshipEntry const& lhs, SponsorshipEntry const& rhs);
bool operator!=(SponsorshipEntry const& lhs, SponsorshipEntry const& rhs);

bool operator==(SponsorshipCounterKey const& lhs,
                SponsorshipCounterKey const& rhs);
bool operator!=(SponsorshipCounterKey const& lhs,
                SponsorshipCounterKey const& rhs);
bool operator==(SponsorshipCounterEntry const& lhs,
                SponsorshipCounterEntry const& rhs);
bool operator!=(SponsorshipCounterEntry const& lhs,
                SponsorshipCounterEntry const& rhs);

bool operator==(AmountIssuedKey const& lhs, AmountIssuedKey const& rhs);
bool operator!=(AmountIssuedKey const& lhs, AmountIssuedKey const& rhs);
bool operator==(AmountIssuedEntry const& lhs, AmountIssuedEntry const& rhs);
bool operator!=(AmountIssuedEntry const& lhs, AmountIssuedEntry const& rhs);

bool operator==(InternalLedgerKey const& lhs, InternalLedgerKey const& rhs);
bool operator!=(InternalLedgerKey const& lhs, InternalLedgerKey const& rhs);
bool operator==(InternalLedgerEntry const& lhs, InternalLedgerEntry const& rhs);
bool operator!=(InternalLedgerEntry const& lhs, InternalLedgerEntry const& rhs);
}
