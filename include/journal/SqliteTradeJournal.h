#ifndef SQLITE_TRADE_JOURNAL_H
#define SQLITE_TRADE_JOURNAL_H

#include "journal/ITradeJournal.h"
#include <SQLiteCpp/SQLiteCpp.h>
#include <string>

class SqliteTradeJournal : public ITradeJournal {
public:
    explicit SqliteTradeJournal(const std::string& dbPath);

    void record(const Order& order) override;
    void closeTrade(std::int64_t orderId,
                    double exitPrice,
                    double realizedPnL,
                    std::chrono::system_clock::time_point closedAt) override;
    std::vector<Order> recentTrades(int limit) const override;

    std::optional<Order> findById(std::int64_t orderId) const override;

private:
    void ensureSchema();
    void addColumnIfMissing(const std::string& columnName, const std::string& columnType);

    mutable SQLite::Database db_;
};

#endif