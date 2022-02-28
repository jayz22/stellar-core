// Copyright 2022 Stellar Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#include "ledger/LedgerTxnImpl.h"
#include "util/GlobalChecks.h"

namespace stellar
{
std::shared_ptr<InternalLedgerEntry const>
LedgerTxnRoot::Impl::loadAmountIssued(InternalLedgerKey const& key) const
{
    auto asset = toOpaqueBase64(key.amountIssuedKey().asset);
    
    std::string sql = "SELECT amountlow, amounthigh "
                      "FROM amountissued "
                      "WHERE asset= :asset";
    auto prep = mDatabase.getPreparedStatement(sql);
    auto& st = prep.statement();
    st.exchange(soci::use(asset));

    int64_t amountLo;
    int64_t amountHi;
    st.exchange(soci::into(amountLo));
    st.exchange(soci::into(amountHi));
    st.define_and_bind();
    // TODO: put in timer?
    st.execute(true);
    if (!st.got_data())
    {
        return nullptr;
    }

    InternalLedgerEntry ile(InternalLedgerEntryType::AMOUNT_ISSUED);
    ile.amountIssuedEntry().asset = key.amountIssuedKey().asset;
    // The BIGINT type in PostgresSQL and SQLite are both signed 8-byte integer. The uint128_t is implemeted with two unsigned 64 bit integer, thus we need to perform the conversion here.
    ile.amountIssuedEntry().amount.low_ = static_cast<uint64_t>(amountLo);
    ile.amountIssuedEntry().amount.high_ = static_cast<uint64_t>(amountHi);

    return std::make_shared<InternalLedgerEntry const>(std::move(ile));
} 

class BulkLoadAmountIssuedOperation
    : public DatabaseTypeSpecificOperation<std::vector<InternalLedgerEntry>>
{
    // TODO
};

UnorderedMap<InternalLedgerKey, std::shared_ptr<InternalLedgerEntry const>>
LedgerTxnRoot::Impl::bulkLoadAmountIssued(
    UnorderedSet<InternalLedgerKey> const& keys) const
{
    // TODO
}

class BulkDeleteAmountIssuedOperation
    : public DatabaseTypeSpecificOperation<void>
{
    Database& mDb;
    LedgerTxnConsistency mCons;
    std::vector<std::string> mAssets;

  public:
    BulkDeleteAmountIssuedOperation(
        Database& db, LedgerTxnConsistency cons,
        std::vector<EntryIterator> const& entries)
        : mDb(db), mCons(cons)
    {
        for (auto const& e : entries)
        {
            releaseAssert(!e.entryExists());
            releaseAssert(e.key().type() == InternalLedgerEntryType::AMOUNT_ISSUED);
            mAssets.emplace_back(toOpaqueBase64(e.key().amountIssuedKey().asset));
        }
    }

    void
    doSociGenericOperation()
    {
        std::string sql = "DELETE FROM amountissued WHERE asset= :asset";
        auto prep = mDb.getPreparedStatement(sql);
        auto& st = prep.statement();
        st.exchange(soci::use(mAssets));
        st.define_and_bind();
        // TODO: put in timer?
        st.execute(true);
        if (static_cast<size_t>(st.get_affected_rows()) != mAssets.size() &&
            mCons == LedgerTxnConsistency::EXACT)
        {
            throw std::runtime_error("Could not update data in SQL");
        }        
    }

    void
    doSqliteSpecificOperation(soci::sqlite3_session_backend* sq) override
    {
        doSociGenericOperation();
    }
#ifdef USE_POSTGRES
    void
    doPostgresSpecificOperation(soci::postgresql_session_backend* pg) override
    {
        // TODO
    }
#endif    
};

void
LedgerTxnRoot::Impl::bulkDeleteAmountIssued(
    std::vector<EntryIterator> const& entries,LedgerTxnConsistency cons)
{
    BulkDeleteAmountIssuedOperation op(mDatabase, cons, entries);
    mDatabase.doDatabaseTypeSpecificOperation(op);
}

class BulkUpsertAmountIssuedOperation
    : public DatabaseTypeSpecificOperation<void>
{
    Database& mDb;
    std::vector<std::string> mAssets;
    std::vector<int64_t> mAmountLo;
    std::vector<int64_t> mAmountHi;
    
    void
    accumulateEntry(AmountIssuedEntry const& entry)
    {
        mAssets.emplace_back(toOpaqueBase64(entry.asset));
        // The BIGINT type in PostgresSQL and SQLite are both signed 8-byte integer. The uint128_t is implemeted with two unsigned 64 bit integer, thus we need to perform the conversion here.
        mAmountLo.emplace_back(static_cast<int64_t>(entry.amount.low_));
        mAmountHi.emplace_back(static_cast<int64_t>(entry.amount.high_));
    }
  
  public:
    BulkUpsertAmountIssuedOperation(
        Database& Db, std::vector<EntryIterator> const& entryIter)
        : mDb(Db)
    {
        for (auto const& e : entryIter)
        {
            releaseAssert(e.entryExists());
            releaseAssert(e.entry().type() == InternalLedgerEntryType::AMOUNT_ISSUED);
            accumulateEntry(e.entry().amountIssuedEntry());
        }
    }

    void
    doSociGenericOperation()
    {
        std::string sql = "INSERT INTO amountissued "
                          "(asset, amountlow, amounthigh) "
                          "VALUES "
                          "( :id, :v1, :v2 ) "
                          "ON CONFLICT (asset) DO UPDATE SET "
                          "asset = excluded.asset, amountlow = "
                          "excluded.amountlow, amounthigh = "
                          "excluded.amounthigh";

        auto prep = mDb.getPreparedStatement(sql);
        soci::statement& st = prep.statement();
        st.exchange(soci::use(mAssets));
        st.exchange(soci::use(mAmountLo));
        st.exchange(soci::use(mAmountHi));
        st.define_and_bind();
        // TODO timer?
        st.execute(true);
        if (static_cast<size_t>(st.get_affected_rows()) != mAssets.size())
        {
            throw std::runtime_error("Could not update data in SQL");
        }
    }

    void
    doSqliteSpecificOperation(soci::sqlite3_session_backend* sq) override
    {
        doSociGenericOperation();
    }

#ifdef USE_POSTGRES
    void
    doPostgresSpecificOperation(soci::postgresql_session_backend* pg) override
    {
        // TODO
    }    
#endif
};

void 
LedgerTxnRoot::Impl::bulkUpsertAmountIssued(
    std::vector<EntryIterator> const& entries)
{
    BulkUpsertAmountIssuedOperation op(mDatabase, entries);
    mDatabase.doDatabaseTypeSpecificOperation(op);
}

void
LedgerTxnRoot::Impl::dropAmountIssued()
{
    throwIfChild();
    mEntryCache.clear();
    mBestOffers.clear();

    std::string coll = mDatabase.getSimpleCollationClause();
    
    mDatabase.getSession() << "DROP TABLE IF EXISTS amountissued;";
    mDatabase.getSession() << "CREATE TABLE amountissued ("
                           << "asset        TEXT " << coll << " PRIMARY KEY, "
                           << "amountlow    BIGINT      NOT NULL, "
                           << "amounthigh   BIGINT      NOT NULL);";
}

}
