#include "broker/MockBrokerAdapter.h"

BrokerResponse MockBrokerAdapter::send(const Order& order) {
    BrokerResponse response;

    if (rejectionToSimulate_.has_value()) {
        response.accepted = false;
        response.rejectionReason = rejectionToSimulate_;
        return response;
    }

    sent_.push_back(order);

    response.accepted = true;
    response.brokerOrderId = "MOCK-" + std::to_string(nextBrokerId_++);
    return response;
}

int MockBrokerAdapter::sentCount() const {
    return static_cast<int>(sent_.size());
}

const std::vector<Order>& MockBrokerAdapter::sentOrders() const {
    return sent_;
}

void MockBrokerAdapter::simulateRejection(const std::string& reason) {
    rejectionToSimulate_ = reason;
}

void MockBrokerAdapter::clearSimulation() {
    rejectionToSimulate_.reset();
}