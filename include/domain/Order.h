#ifndef ORDER_H
#define ORDER_H

#include "domain/TradeIntent.h"
#include <optional>
#include <chrono>
#include <cstdint>

class Order {
public:
    // Factory for newly-created orders. Generates a fresh ID and captures
    // the current time as createdAt.
    static Order fromValidatedIntent(const TradeIntent& intent, int size);

    // Factory for orders loaded from persistent storage. Caller supplies
    // the original ID and creation timestamp; nothing is generated.
    static Order fromStorage(const TradeIntent& intent, int size,
                             std::int64_t id,
                             std::chrono::system_clock::time_point createdAt);

    Side getSide() const;
    const Instrument& getInstrument() const;
    double getEntryPrice() const;
    double getStopPrice() const;
    std::optional<double> getTargetPrice() const;
    int getSize() const;
    std::int64_t getId() const;
    std::chrono::system_clock::time_point getCreatedAt() const;

private:
    Order(const TradeIntent& intent, int size,
          std::int64_t id, std::chrono::system_clock::time_point createdAt);

    TradeIntent intent_;
    int size_;
    std::int64_t id_;
    std::chrono::system_clock::time_point createdAt_;
};

#endif