#include "journal/InMemoryTradeJournal.h"

#include <algorithm>
#include <stdexcept>

void InMemoryTradeJournal::record(const Order& order) {
    trades_.push_back(order);
}

void InMemoryTradeJournal::closeTrade(std::int64_t orderId,
                                      double exitPrice,
                                      double realizedPnL,
                                      std::chrono::system_clock::time_point closedAt) {
    auto it = std::find_if(trades_.begin(), trades_.end(),
                           [orderId](const Order& o) { return o.getId() == orderId; });
    if (it == trades_.end()) {
        throw std::runtime_error("No trade with ID " + std::to_string(orderId));
    }
    it->close(closedAt, exitPrice, realizedPnL);
}

std::vector<Order> InMemoryTradeJournal::recentTrades(int limit) const {
    std::vector<Order> result;
    int start = std::max(0, static_cast<int>(trades_.size()) - limit);
    for (int i = static_cast<int>(trades_.size()) - 1; i >= start; --i) {
        result.push_back(trades_[i]);
    }
    return result;
}

int InMemoryTradeJournal::size() const {
    return static_cast<int>(trades_.size());
}
std::optional<Order> InMemoryTradeJournal::findById(std::int64_t orderId) const {
    auto it = std::find_if(trades_.begin(), trades_.end(),
                           [orderId](const Order& o) { return o.getId() == orderId; });
    if (it == trades_.end()) {
        return std::nullopt;
    }
    return *it;
}