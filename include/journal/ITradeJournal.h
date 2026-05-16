#ifndef ITRADE_JOURNAL_H
#define ITRADE_JOURNAL_H

#include "domain/Order.h"
#include <optional>
#include <vector>

class ITradeJournal {
public:
    virtual ~ITradeJournal() = default;

    virtual void record(const Order& order) = 0;

    virtual void closeTrade(std::int64_t orderId,
                            double exitPrice,
                            double realizedPnL,
                            std::chrono::system_clock::time_point closedAt) = 0;

    virtual std::optional<Order> findById(std::int64_t orderId) const = 0;

    virtual std::vector<Order> recentTrades(int limit) const = 0;
};

#endif