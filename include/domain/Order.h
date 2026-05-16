#ifndef ORDER_H
#define ORDER_H

#include "domain/TradeIntent.h"
#include <chrono>
#include <cstdint>
#include <optional>

class Order {
public:
    // Factory for newly-created orders. Generates a fresh ID and captures
    // the current time as createdAt.
    static Order fromValidatedIntent(const TradeIntent& intent, int size);

    // Factory for open orders loaded from persistent storage.
    static Order fromStorage(const TradeIntent& intent, int size,
                             std::int64_t id,
                             std::chrono::system_clock::time_point createdAt);

    // Factory for closed orders loaded from persistent storage.
    // Includes the close-time fields.
    static Order fromClosedStorage(const TradeIntent& intent, int size,
                                   std::int64_t id,
                                   std::chrono::system_clock::time_point createdAt,
                                   std::chrono::system_clock::time_point closedAt,
                                   double exitPrice,
                                   double realizedPnL);

    // Raise the next-ID counter to be at least lastSeenId + 1.
    static void seedNextOrderId(std::int64_t lastSeenId);

    Side getSide() const;
    const Instrument& getInstrument() const;
    double getEntryPrice() const;
    double getStopPrice() const;
    std::optional<double> getTargetPrice() const;
    int getSize() const;
    std::int64_t getId() const;
    std::chrono::system_clock::time_point getCreatedAt() const;

    // Close-state accessors.
    bool isClosed() const;
    std::optional<std::chrono::system_clock::time_point> getClosedAt() const;
    std::optional<double> getExitPrice() const;
    std::optional<double> getRealizedPnL() const;

    // Mark this order as closed. Mutates the order in place.
    void close(std::chrono::system_clock::time_point closedAt,
               double exitPrice,
               double realizedPnL);

private:
    Order(const TradeIntent& intent, int size,
          std::int64_t id, std::chrono::system_clock::time_point createdAt);

    TradeIntent intent_;
    int size_;
    std::int64_t id_;
    std::chrono::system_clock::time_point createdAt_;

    // Close-state fields. All three are set together or none are set.
    std::optional<std::chrono::system_clock::time_point> closedAt_;
    std::optional<double> exitPrice_;
    std::optional<double> realizedPnL_;
};

#endif