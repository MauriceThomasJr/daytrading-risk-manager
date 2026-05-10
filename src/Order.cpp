#include "domain/Order.h"

#include <atomic>

namespace {
    std::atomic<std::int64_t> nextOrderId{1};
}

Order::Order(const TradeIntent& intent, int size,
             std::int64_t id, std::chrono::system_clock::time_point createdAt)
    : intent_(intent), size_(size), id_(id), createdAt_(createdAt) {}

Order Order::fromValidatedIntent(const TradeIntent& intent, int size) {
    return Order(intent, size,
                 nextOrderId.fetch_add(1),
                 std::chrono::system_clock::now());
}

Order Order::fromStorage(const TradeIntent& intent, int size,
                         std::int64_t id,
                         std::chrono::system_clock::time_point createdAt) {
    return Order(intent, size, id, createdAt);
}

void Order::seedNextOrderId(std::int64_t lastSeenId) {
    // Raise the counter so the next generated ID is past lastSeenId.
    std::int64_t desired = lastSeenId + 1;
    std::int64_t current = nextOrderId.load();
    while (current < desired) {
        // compare_exchange_weak updates `current` to the latest value if
        // the swap fails, so the loop converges.
        if (nextOrderId.compare_exchange_weak(current, desired)) {
            break;
        }
    }
}

Side Order::getSide() const {
    return intent_.getSide();
}

const Instrument& Order::getInstrument() const {
    return intent_.getInstrument();
}

double Order::getEntryPrice() const {
    return intent_.getEntryPrice();
}

double Order::getStopPrice() const {
    return intent_.getStopPrice();
}

std::optional<double> Order::getTargetPrice() const {
    return intent_.getTargetPrice();
}

int Order::getSize() const {
    return size_;
}

std::int64_t Order::getId() const {
    return id_;
}

std::chrono::system_clock::time_point Order::getCreatedAt() const {
    return createdAt_;
}