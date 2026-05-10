#ifndef SQLITE_TRADE_JOURNAL_H
#define SQLITE_TRADE_JOURNAL_H

#include "journal/ITradeJournal.h"
#include <SQLiteCpp/SQLiteCpp.h>
#include <string>

class SqliteTradeJournal : public ITradeJournal {
public:
    // Open or create a database at the given path.
    // Use ":memory:" for an in-memory database (tests).
    explicit SqliteTradeJournal(const std::string& dbPath);

    void record(const Order& order) override;
    std::vector<Order> recentTrades(int limit) const override;

private:
    void ensureSchema();

    mutable SQLite::Database db_;
};

#endif