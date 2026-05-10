#ifndef IN_MEMORY_TRADE_JOURNAL_H
#define IN_MEMORY_TRADE_JOURNAL_H

#include "journal/ITradeJournal.h"
#include <vector>

class InMemoryTradeJournal : public ITradeJournal {
public:
    void record(const Order& order) override;
    std::vector<Order> recentTrades(int limit) const override;

    int size() const;

private:
    std::vector<Order> orders_;
};

#endif