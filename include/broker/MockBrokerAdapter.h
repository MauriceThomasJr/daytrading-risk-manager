#ifndef MOCK_BROKER_ADAPTER_H
#define MOCK_BROKER_ADAPTER_H

#include "broker/IBrokerAdapter.h"
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

class MockBrokerAdapter : public IBrokerAdapter {
public:
    BrokerResponse send(const Order& order) override;

    // Test helpers: what has this mock been asked to send?
    int sentCount() const;
    const std::vector<Order>& sentOrders() const;

    // Configure the mock to reject the next send (and all sends after)
    // until clearSimulation() is called.
    void simulateRejection(const std::string& reason);
    void clearSimulation();

private:
    std::vector<Order> sent_;
    std::optional<std::string> rejectionToSimulate_;
    std::int64_t nextBrokerId_ = 1;
};

#endif