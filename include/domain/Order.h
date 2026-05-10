#ifndef ORDER_H
#define ORDER_H

#include "domain/TradeIntent.h"
#include <optional>
#include <chrono>
#include <cstdint>

class Order {
public:
    // Factory: only way to construct an Order is from a validated intent.
    static Order fromValidatedIntent(const TradeIntent& intent, int size);

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