#include "journal/InMemoryTradeJournal.h"

void InMemoryTradeJournal::record(const Order& order) {
    orders_.push_back(order);
}

std::vector<Order> InMemoryTradeJournal::recentTrades(int limit) const {
    if (limit <= 0) {
        return {};
    }

    std::vector<Order> result;

    // Walk backward from the end (newest first), up to `limit` items.
    int taken = 0;
    for (auto it = orders_.rbegin(); it != orders_.rend() && taken < limit; ++it, ++taken) {
        result.push_back(*it);
    }

    return result;
}

int InMemoryTradeJournal::size() const {
    return static_cast<int>(orders_.size());
}