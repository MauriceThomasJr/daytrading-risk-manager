#ifndef ITRADE_JOURNAL_H
#define ITRADE_JOURNAL_H

#include "domain/Order.h"
#include <vector>

class ITradeJournal {
public:
    virtual ~ITradeJournal() = default;

    // Record a newly-opened order in the journal.
    virtual void record(const Order& order) = 0;

    // Mark an existing order as closed. Updates the journal entry with
    // the exit price, realized P&L, and closed-at timestamp.
    // Throws std::runtime_error if no order with the given ID exists.
    virtual void closeTrade(std::int64_t orderId,
                            double exitPrice,
                            double realizedPnL,
                            std::chrono::system_clock::time_point closedAt) = 0;

    // Retrieve the N most recently recorded trades, newest first.
    virtual std::vector<Order> recentTrades(int limit) const = 0;
};

#endif