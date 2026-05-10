#ifndef ITRADE_JOURNAL_H
#define ITRADE_JOURNAL_H

#include "domain/Order.h"
#include <vector>

class ITradeJournal {
public:
    virtual ~ITradeJournal() = default;

    // Append an order to the journal.
    virtual void record(const Order& order) = 0;

    // Read back up to `limit` of the most recent orders, newest first.
    virtual std::vector<Order> recentTrades(int limit) const = 0;
};

#endif