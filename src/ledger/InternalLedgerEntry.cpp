// Copyright 2020 Stellar Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#include "ledger/InternalLedgerEntry.h"
#include "ledger/LedgerHashUtils.h"
#include "util/GlobalChecks.h"
#include "util/HashOfHash.h"
#include "util/XDRCereal.h"
#include "util/types.h"

#include <fmt/format.h>

namespace stellar
{

bool
operator==(SponsorshipKey const& lhs, SponsorshipKey const& rhs)
{
    return lhs.sponsoredID == rhs.sponsoredID;
}

bool
operator!=(SponsorshipKey const& lhs, SponsorshipKey const& rhs)
{
    return !(lhs == rhs);
}

bool
operator==(SponsorshipEntry const& lhs, SponsorshipEntry const& rhs)
{
    return lhs.sponsoredID == rhs.sponsoredID &&
           lhs.sponsoringID == rhs.sponsoringID;
}

bool
operator!=(SponsorshipEntry const& lhs, SponsorshipEntry const& rhs)
{
    return !(lhs == rhs);
}

bool
operator==(SponsorshipCounterKey const& lhs, SponsorshipCounterKey const& rhs)
{
    return lhs.sponsoringID == rhs.sponsoringID;
}

bool
operator!=(SponsorshipCounterKey const& lhs, SponsorshipCounterKey const& rhs)
{
    return !(lhs == rhs);
}

bool
operator==(SponsorshipCounterEntry const& lhs,
           SponsorshipCounterEntry const& rhs)
{
    return lhs.sponsoringID == rhs.sponsoringID &&
           lhs.numSponsoring == rhs.numSponsoring;
}

bool
operator!=(SponsorshipCounterEntry const& lhs,
           SponsorshipCounterEntry const& rhs)
{
    return !(lhs == rhs);
}

bool
operator==(AmountIssuedKey const& lhs, AmountIssuedKey const& rhs)
{
    return lhs.asset == rhs.asset;
}

bool
operator!=(AmountIssuedKey const& lhs, AmountIssuedKey const& rhs)
{
    return !(lhs == rhs);
}

bool
operator==(AmountIssuedEntry const& lhs, AmountIssuedEntry const& rhs)
{
    return lhs.asset == rhs.asset && lhs.amount == rhs.amount;
}

bool
operator!=(AmountIssuedEntry const& lhs, AmountIssuedEntry const& rhs)
{
    return !(lhs == rhs);
}

// InternalLedgerKey -------------------------------------------------------
InternalLedgerKey::InternalLedgerKey()
    : InternalLedgerKey(InternalLedgerEntryType::LEDGER_ENTRY)
{
}

InternalLedgerKey::InternalLedgerKey(InternalLedgerEntryType t) : mType(t)
{
    construct();
}

InternalLedgerKey::InternalLedgerKey(LedgerKey const& lk)
    : InternalLedgerKey(InternalLedgerEntryType::LEDGER_ENTRY)
{
    ledgerKeyRef() = lk;
}

InternalLedgerKey::InternalLedgerKey(SponsorshipKey const& sk)
    : InternalLedgerKey(InternalLedgerEntryType::SPONSORSHIP)
{
    sponsorshipKeyRef() = sk;
}

InternalLedgerKey::InternalLedgerKey(SponsorshipCounterKey const& sck)
    : InternalLedgerKey(InternalLedgerEntryType::SPONSORSHIP_COUNTER)
{
    sponsorshipCounterKeyRef() = sck;
}

InternalLedgerKey::InternalLedgerKey(AmountIssuedKey const& aik)
    : InternalLedgerKey(InternalLedgerEntryType::AMOUNT_ISSUED)
{
    amountIssuedKeyRef() = aik;
}

InternalLedgerKey::InternalLedgerKey(InternalLedgerKey const& glk)
    : InternalLedgerKey(glk.type())
{
    assign(glk);
}

InternalLedgerKey::InternalLedgerKey(InternalLedgerKey&& glk)
    : InternalLedgerKey(glk.type())
{
    assign(std::move(glk));
}

InternalLedgerKey
InternalLedgerKey::makeSponsorshipKey(AccountID const& sponsoredId)
{
    InternalLedgerKey res(InternalLedgerEntryType::SPONSORSHIP);
    res.sponsorshipKeyRef().sponsoredID = sponsoredId;
    return res;
}

InternalLedgerKey
InternalLedgerKey::makeSponsorshipCounterKey(AccountID const& sponsoringId)
{
    InternalLedgerKey res(InternalLedgerEntryType::SPONSORSHIP_COUNTER);
    res.sponsorshipCounterKeyRef().sponsoringID = sponsoringId;
    return res;
}

InternalLedgerKey
InternalLedgerKey::makeAmountIssuedKey(Asset const& asset)
{
    InternalLedgerKey res(InternalLedgerEntryType::AMOUNT_ISSUED);
    res.amountIssuedKeyRef().asset = asset;
    return res;
}

InternalLedgerKey&
InternalLedgerKey::operator=(InternalLedgerKey const& glk)
{
    type(glk.type());
    assign(glk);
    return *this;
}

InternalLedgerKey&
InternalLedgerKey::operator=(InternalLedgerKey&& glk)
{
    if (this == &glk)
    {
        return *this;
    }

    type(glk.type());
    assign(std::move(glk));
    return *this;
}

InternalLedgerKey::~InternalLedgerKey()
{
    destruct();
}

size_t
InternalLedgerKey::hash() const
{
    if (mHash != 0)
    {
        return mHash;
    }
    size_t res;
    switch (type())
    {
    case stellar::InternalLedgerEntryType::LEDGER_ENTRY:
        res = std::hash<stellar::LedgerKey>()(ledgerKey());
        break;
    case stellar::InternalLedgerEntryType::SPONSORSHIP:
        res = std::hash<stellar::uint256>()(
            sponsorshipKey().sponsoredID.ed25519());
        break;
    case stellar::InternalLedgerEntryType::SPONSORSHIP_COUNTER:
        res = std::hash<stellar::uint256>()(
            sponsorshipCounterKey().sponsoringID.ed25519());
        break;
    case stellar::InternalLedgerEntryType::AMOUNT_ISSUED:
        res = std::hash<stellar::Asset>()(amountIssuedKey().asset);
        break;
    default:
        abort();
    }
    hashMix(res, static_cast<size_t>(type()));
    mHash = res;
    return res;
}

void
InternalLedgerKey::assign(InternalLedgerKey const& glk)
{
    releaseAssert(glk.type() == mType);
    mHash = glk.mHash;
    switch (mType)
    {
    case InternalLedgerEntryType::LEDGER_ENTRY:
        ledgerKeyRef() = glk.ledgerKey();
        break;
    case InternalLedgerEntryType::SPONSORSHIP:
        sponsorshipKeyRef() = glk.sponsorshipKey();
        break;
    case InternalLedgerEntryType::SPONSORSHIP_COUNTER:
        sponsorshipCounterKeyRef() = glk.sponsorshipCounterKey();
        break;
    case InternalLedgerEntryType::AMOUNT_ISSUED:
        amountIssuedKeyRef() = glk.amountIssuedKey();
        break;
    default:
        abort();
    }
}

void
InternalLedgerKey::assign(InternalLedgerKey&& glk)
{
    releaseAssert(glk.type() == mType);
    mHash = glk.mHash;
    switch (mType)
    {
    case InternalLedgerEntryType::LEDGER_ENTRY:
        ledgerKeyRef() = std::move(glk.mLedgerKey);
        break;
    case InternalLedgerEntryType::SPONSORSHIP:
        sponsorshipKeyRef() = std::move(glk.mSponsorshipKey);
        break;
    case InternalLedgerEntryType::SPONSORSHIP_COUNTER:
        sponsorshipCounterKeyRef() = std::move(glk.mSponsorshipCounterKey);
        break;
    case InternalLedgerEntryType::AMOUNT_ISSUED:
        amountIssuedKeyRef() = std::move(glk.mAmountIssuedKey);
        break;
    default:
        abort();
    }
}

void
InternalLedgerKey::construct()
{
    switch (mType)
    {
    case InternalLedgerEntryType::LEDGER_ENTRY:
        new (&mLedgerKey) LedgerKey();
        break;
    case InternalLedgerEntryType::SPONSORSHIP:
        new (&mSponsorshipKey) SponsorshipKey();
        break;
    case InternalLedgerEntryType::SPONSORSHIP_COUNTER:
        new (&mSponsorshipCounterKey) SponsorshipCounterKey();
        break;
    case InternalLedgerEntryType::AMOUNT_ISSUED:
        new (&mAmountIssuedKey) AmountIssuedKey();
        break;
    default:
        abort();
    }
    mHash = 0;
}

void
InternalLedgerKey::destruct()
{
    switch (mType)
    {
    case InternalLedgerEntryType::LEDGER_ENTRY:
        mLedgerKey.~LedgerKey();
        break;
    case InternalLedgerEntryType::SPONSORSHIP:
        mSponsorshipKey.~SponsorshipKey();
        break;
    case InternalLedgerEntryType::SPONSORSHIP_COUNTER:
        mSponsorshipCounterKey.~SponsorshipCounterKey();
        break;
    case InternalLedgerEntryType::AMOUNT_ISSUED:
        mAmountIssuedKey.~AmountIssuedKey();
        break;
    default:
        abort();
    }
    mHash = 0;
}

void
InternalLedgerKey::type(InternalLedgerEntryType t)
{
    if (t != mType)
    {
        destruct();
        mType = t;
        construct();
    }
}

InternalLedgerEntryType
InternalLedgerKey::type() const
{
    return mType;
}

void
InternalLedgerKey::checkDiscriminant(InternalLedgerEntryType expected) const
{
    if (mType != expected)
    {
        throw std::runtime_error("invalid union access");
    }
}

LedgerKey&
InternalLedgerKey::ledgerKeyRef()
{
    checkDiscriminant(InternalLedgerEntryType::LEDGER_ENTRY);
    mHash = 0;
    return mLedgerKey;
}

LedgerKey const&
InternalLedgerKey::ledgerKey() const
{
    checkDiscriminant(InternalLedgerEntryType::LEDGER_ENTRY);
    return mLedgerKey;
}

SponsorshipKey&
InternalLedgerKey::sponsorshipKeyRef()
{
    checkDiscriminant(InternalLedgerEntryType::SPONSORSHIP);
    mHash = 0;
    return mSponsorshipKey;
}

SponsorshipKey const&
InternalLedgerKey::sponsorshipKey() const
{
    checkDiscriminant(InternalLedgerEntryType::SPONSORSHIP);
    return mSponsorshipKey;
}

SponsorshipCounterKey&
InternalLedgerKey::sponsorshipCounterKeyRef()
{
    checkDiscriminant(InternalLedgerEntryType::SPONSORSHIP_COUNTER);
    mHash = 0;
    return mSponsorshipCounterKey;
}

SponsorshipCounterKey const&
InternalLedgerKey::sponsorshipCounterKey() const
{
    checkDiscriminant(InternalLedgerEntryType::SPONSORSHIP_COUNTER);
    return mSponsorshipCounterKey;
}

AmountIssuedKey&
InternalLedgerKey::amountIssuedKeyRef()
{
    checkDiscriminant(InternalLedgerEntryType::AMOUNT_ISSUED);
    mHash = 0;
    return mAmountIssuedKey;
}

AmountIssuedKey const&
InternalLedgerKey::amountIssuedKey() const
{
    checkDiscriminant(InternalLedgerEntryType::AMOUNT_ISSUED);
    return mAmountIssuedKey;
}

std::string
InternalLedgerKey::toString() const
{
    switch (mType)
    {
    case InternalLedgerEntryType::LEDGER_ENTRY:
        return xdr_to_string(ledgerKey(), "LedgerKey");

    case InternalLedgerEntryType::SPONSORSHIP:
        return fmt::format(
            FMT_STRING("{{\n  {}\n}}\n"),
            xdr_to_string(sponsorshipKey().sponsoredID, "sponsoredID"));
    case InternalLedgerEntryType::SPONSORSHIP_COUNTER:
        return fmt::format(FMT_STRING("{{\n  {}\n}}\n"),
                           xdr_to_string(sponsorshipCounterKey().sponsoringID,
                                         "sponsoringID"));
    case InternalLedgerEntryType::AMOUNT_ISSUED:
        return fmt::format(FMT_STRING("{{\n  {}\n}}\n"),
                           xdr_to_string(amountIssuedKey().asset, "asset"));
    default:
        abort();
    }
}

bool
operator==(InternalLedgerKey const& lhs, InternalLedgerKey const& rhs)
{
    if (lhs.hash() != rhs.hash() || lhs.type() != rhs.type())
    {
        return false;
    }

    switch (lhs.type())
    {
    case InternalLedgerEntryType::LEDGER_ENTRY:
        return lhs.ledgerKey() == rhs.ledgerKey();
    case InternalLedgerEntryType::SPONSORSHIP:
        return lhs.sponsorshipKey() == rhs.sponsorshipKey();
    case InternalLedgerEntryType::SPONSORSHIP_COUNTER:
        return lhs.sponsorshipCounterKey() == rhs.sponsorshipCounterKey();
    case InternalLedgerEntryType::AMOUNT_ISSUED:
        return lhs.amountIssuedKey() == rhs.amountIssuedKey();
    default:
        abort();
    }
}

bool
operator!=(InternalLedgerKey const& lhs, InternalLedgerKey const& rhs)
{
    return !(lhs == rhs);
}

// InternalLedgerEntry -----------------------------------------------------
InternalLedgerEntry::InternalLedgerEntry()
    : InternalLedgerEntry(InternalLedgerEntryType::LEDGER_ENTRY)
{
}

InternalLedgerEntry::InternalLedgerEntry(InternalLedgerEntryType t) : mType(t)
{
    construct();
}

InternalLedgerEntry::InternalLedgerEntry(LedgerEntry const& le)
    : InternalLedgerEntry(InternalLedgerEntryType::LEDGER_ENTRY)
{
    ledgerEntry() = le;
}

InternalLedgerEntry::InternalLedgerEntry(SponsorshipEntry const& se)
    : InternalLedgerEntry(InternalLedgerEntryType::SPONSORSHIP)
{
    sponsorshipEntry() = se;
}

InternalLedgerEntry::InternalLedgerEntry(SponsorshipCounterEntry const& sce)
    : InternalLedgerEntry(InternalLedgerEntryType::SPONSORSHIP_COUNTER)
{
    sponsorshipCounterEntry() = sce;
}

InternalLedgerEntry::InternalLedgerEntry(AmountIssuedEntry const& aie)
    : InternalLedgerEntry(InternalLedgerEntryType::AMOUNT_ISSUED)
{
    amountIssuedEntry() = aie;
}

InternalLedgerEntry::InternalLedgerEntry(InternalLedgerEntry const& gle)
    : InternalLedgerEntry(gle.type())
{
    assign(gle);
}

InternalLedgerEntry::InternalLedgerEntry(InternalLedgerEntry&& gle)
    : InternalLedgerEntry(gle.type())
{
    assign(std::move(gle));
}

InternalLedgerEntry&
InternalLedgerEntry::operator=(InternalLedgerEntry const& gle)
{
    if (this == &gle)
    {
        return *this;
    }

    type(gle.type());
    assign(gle);
    return *this;
}

InternalLedgerEntry&
InternalLedgerEntry::operator=(InternalLedgerEntry&& gle)
{
    if (this == &gle)
    {
        return *this;
    }

    type(gle.type());
    assign(std::move(gle));
    return *this;
}

InternalLedgerEntry::~InternalLedgerEntry()
{
    destruct();
}

void
InternalLedgerEntry::assign(InternalLedgerEntry const& gle)
{
    releaseAssert(gle.type() == mType);
    switch (mType)
    {
    case InternalLedgerEntryType::LEDGER_ENTRY:
        ledgerEntry() = gle.ledgerEntry();
        break;
    case InternalLedgerEntryType::SPONSORSHIP:
        sponsorshipEntry() = gle.sponsorshipEntry();
        break;
    case InternalLedgerEntryType::SPONSORSHIP_COUNTER:
        sponsorshipCounterEntry() = gle.sponsorshipCounterEntry();
        break;
    case InternalLedgerEntryType::AMOUNT_ISSUED:
        amountIssuedEntry() = gle.amountIssuedEntry();
        break;
    default:
        abort();
    }
}

void
InternalLedgerEntry::assign(InternalLedgerEntry&& gle)
{
    releaseAssert(gle.type() == mType);
    switch (mType)
    {
    case InternalLedgerEntryType::LEDGER_ENTRY:
        ledgerEntry() = std::move(gle.ledgerEntry());
        break;
    case InternalLedgerEntryType::SPONSORSHIP:
        sponsorshipEntry() = std::move(gle.sponsorshipEntry());
        break;
    case InternalLedgerEntryType::SPONSORSHIP_COUNTER:
        sponsorshipCounterEntry() = std::move(gle.sponsorshipCounterEntry());
        break;
    case InternalLedgerEntryType::AMOUNT_ISSUED:
        amountIssuedEntry() = std::move(gle.amountIssuedEntry());
        break;
    default:
        abort();
    }
}

void
InternalLedgerEntry::construct()
{
    switch (mType)
    {
    case InternalLedgerEntryType::LEDGER_ENTRY:
        new (&mLedgerEntry) LedgerEntry();
        break;
    case InternalLedgerEntryType::SPONSORSHIP:
        new (&mSponsorshipEntry) SponsorshipEntry();
        break;
    case InternalLedgerEntryType::SPONSORSHIP_COUNTER:
        new (&mSponsorshipCounterEntry) SponsorshipCounterEntry();
        break;
    case InternalLedgerEntryType::AMOUNT_ISSUED:
        new (&mAmountIssuedEntry) AmountIssuedEntry();
        break;
    default:
        abort();
    }
}

void
InternalLedgerEntry::destruct()
{
    switch (mType)
    {
    case InternalLedgerEntryType::LEDGER_ENTRY:
        mLedgerEntry.~LedgerEntry();
        break;
    case InternalLedgerEntryType::SPONSORSHIP:
        mSponsorshipEntry.~SponsorshipEntry();
        break;
    case InternalLedgerEntryType::SPONSORSHIP_COUNTER:
        mSponsorshipCounterEntry.~SponsorshipCounterEntry();
        break;
    case InternalLedgerEntryType::AMOUNT_ISSUED:
        mAmountIssuedEntry.~AmountIssuedEntry();
        break;
    default:
        abort();
    }
}

void
InternalLedgerEntry::type(InternalLedgerEntryType t)
{
    if (t != mType)
    {
        destruct();
        mType = t;
        construct();
    }
}

InternalLedgerEntryType
InternalLedgerEntry::type() const
{
    return mType;
}

void
InternalLedgerEntry::checkDiscriminant(InternalLedgerEntryType expected) const
{
    if (mType != expected)
    {
        throw std::runtime_error("invalid union access");
    }
}

LedgerEntry&
InternalLedgerEntry::ledgerEntry()
{
    checkDiscriminant(InternalLedgerEntryType::LEDGER_ENTRY);
    return mLedgerEntry;
}

LedgerEntry const&
InternalLedgerEntry::ledgerEntry() const
{
    checkDiscriminant(InternalLedgerEntryType::LEDGER_ENTRY);
    return mLedgerEntry;
}

SponsorshipEntry&
InternalLedgerEntry::sponsorshipEntry()
{
    checkDiscriminant(InternalLedgerEntryType::SPONSORSHIP);
    return mSponsorshipEntry;
}

SponsorshipEntry const&
InternalLedgerEntry::sponsorshipEntry() const
{
    checkDiscriminant(InternalLedgerEntryType::SPONSORSHIP);
    return mSponsorshipEntry;
}

SponsorshipCounterEntry&
InternalLedgerEntry::sponsorshipCounterEntry()
{
    checkDiscriminant(InternalLedgerEntryType::SPONSORSHIP_COUNTER);
    return mSponsorshipCounterEntry;
}

SponsorshipCounterEntry const&
InternalLedgerEntry::sponsorshipCounterEntry() const
{
    checkDiscriminant(InternalLedgerEntryType::SPONSORSHIP_COUNTER);
    return mSponsorshipCounterEntry;
}

AmountIssuedEntry&
InternalLedgerEntry::amountIssuedEntry()
{
    checkDiscriminant(InternalLedgerEntryType::AMOUNT_ISSUED);
    return mAmountIssuedEntry;
}

AmountIssuedEntry const&
InternalLedgerEntry::amountIssuedEntry() const
{
    checkDiscriminant(InternalLedgerEntryType::AMOUNT_ISSUED);
    return mAmountIssuedEntry;
}

InternalLedgerKey
InternalLedgerEntry::toKey() const
{
    switch (mType)
    {
    case InternalLedgerEntryType::LEDGER_ENTRY:
        return LedgerEntryKey(ledgerEntry());
    case InternalLedgerEntryType::SPONSORSHIP:
        return InternalLedgerKey(
            SponsorshipKey{sponsorshipEntry().sponsoredID});
    case InternalLedgerEntryType::SPONSORSHIP_COUNTER:
        return InternalLedgerKey(
            SponsorshipCounterKey{sponsorshipCounterEntry().sponsoringID});
    case InternalLedgerEntryType::AMOUNT_ISSUED:
        return InternalLedgerKey(AmountIssuedKey{amountIssuedEntry().asset});
    default:
        abort();
    }
}

std::string
InternalLedgerEntry::toString() const
{
    switch (mType)
    {
    case InternalLedgerEntryType::LEDGER_ENTRY:
        return xdr_to_string(ledgerEntry(), "LedgerEntry");
    case InternalLedgerEntryType::SPONSORSHIP:
        return fmt::format(
            FMT_STRING("{{\n  {},\n  {}\n}}\n"),
            xdr_to_string(sponsorshipEntry().sponsoredID, "sponsoredID"),
            xdr_to_string(sponsorshipEntry().sponsoringID, "sponsoringID"));
    case InternalLedgerEntryType::SPONSORSHIP_COUNTER:
        return fmt::format(FMT_STRING("{{\n  {},\n  numSponsoring = {}\n}}\n"),
                           xdr_to_string(sponsorshipCounterEntry().sponsoringID,
                                         "sponsoringID"),
                           sponsorshipCounterEntry().numSponsoring);
    case InternalLedgerEntryType::AMOUNT_ISSUED:
        return fmt::format(FMT_STRING("{{\n  {},\n  amount = {}\n}}\n"),
                           xdr_to_string(amountIssuedEntry().asset, "asset"),
                           uint128_to_string(amountIssuedEntry().amount));
    default:
        abort();
    }
}

bool
operator==(InternalLedgerEntry const& lhs, InternalLedgerEntry const& rhs)
{
    if (lhs.type() != rhs.type())
    {
        return false;
    }

    switch (lhs.type())
    {
    case InternalLedgerEntryType::LEDGER_ENTRY:
        return lhs.ledgerEntry() == rhs.ledgerEntry();
    case InternalLedgerEntryType::SPONSORSHIP:
        return lhs.sponsorshipEntry() == rhs.sponsorshipEntry();
    case InternalLedgerEntryType::SPONSORSHIP_COUNTER:
        return lhs.sponsorshipCounterEntry() == rhs.sponsorshipCounterEntry();
    case InternalLedgerEntryType::AMOUNT_ISSUED:
        return lhs.amountIssuedEntry() == rhs.amountIssuedEntry();
    default:
        abort();
    }
}

bool
operator!=(InternalLedgerEntry const& lhs, InternalLedgerEntry const& rhs)
{
    return !(lhs == rhs);
}
}
