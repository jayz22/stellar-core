// Copyright 2020 Stellar Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "xdr/Stellar-ledger-entries.h"
#include "xdr/Stellar-ledger.h"
#include "lib/util/uint128_t.h"

namespace stellar
{

enum class InternalLedgerEntryType
{
    LEDGER_ENTRY,
    SPONSORSHIP,
    SPONSORSHIP_COUNTER,
    ASSET_AMOUNT_ISSUED,
};

struct SponsorshipKey
{
    AccountID sponsoredID;
};

struct SponsorshipCounterKey
{
    AccountID sponsoringID;
};

struct AssetAmountIssuedKey
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
        AssetAmountIssuedKey mAssetAmountIssuedKey;
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
    AssetAmountIssuedKey& assetAmountIssuedKeyRef();

  public:
    static InternalLedgerKey makeSponsorshipKey(AccountID const& sponsoredId);
    static InternalLedgerKey
    makeSponsorshipCounterKey(AccountID const& sponsoringId);
    static InternalLedgerKey
    makeAssetAmountIssuedKey(Asset const& asset);

    InternalLedgerKey();
    explicit InternalLedgerKey(InternalLedgerEntryType t);

    InternalLedgerKey(LedgerKey const& lk);
    explicit InternalLedgerKey(SponsorshipKey const& sk);
    explicit InternalLedgerKey(SponsorshipCounterKey const& sck);
    explicit InternalLedgerKey(AssetAmountIssuedKey const& aik);

    InternalLedgerKey(InternalLedgerKey const& glk);
    InternalLedgerKey(InternalLedgerKey&& glk);

    InternalLedgerKey& operator=(InternalLedgerKey const& glk);
    InternalLedgerKey& operator=(InternalLedgerKey&& glk);

    ~InternalLedgerKey();

    InternalLedgerEntryType type() const;

    LedgerKey const& ledgerKey() const;

    SponsorshipKey const& sponsorshipKey() const;

    SponsorshipCounterKey const& sponsorshipCounterKey() const;

    AssetAmountIssuedKey const& assetAmountIssuedKey() const;

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

struct AssetAmountIssuedEntry
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
        AssetAmountIssuedEntry mAssetAmountIssuedEntry;
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
    explicit InternalLedgerEntry(AssetAmountIssuedEntry const& aie);

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

    AssetAmountIssuedEntry& assetAmountIssuedEntry();
    AssetAmountIssuedEntry const& assetAmountIssuedEntry() const;

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

bool operator==(AssetAmountIssuedKey const& lhs,
                AssetAmountIssuedKey const& rhs);
bool operator!=(AssetAmountIssuedKey const& lhs,
                AssetAmountIssuedKey const& rhs);                    
bool operator==(AssetAmountIssuedEntry const& lhs,
                AssetAmountIssuedEntry const& rhs);
bool operator!=(AssetAmountIssuedEntry const& lhs,
                AssetAmountIssuedEntry const& rhs);

bool operator==(InternalLedgerKey const& lhs, InternalLedgerKey const& rhs);
bool operator!=(InternalLedgerKey const& lhs, InternalLedgerKey const& rhs);
bool operator==(InternalLedgerEntry const& lhs, InternalLedgerEntry const& rhs);
bool operator!=(InternalLedgerEntry const& lhs, InternalLedgerEntry const& rhs);
}
