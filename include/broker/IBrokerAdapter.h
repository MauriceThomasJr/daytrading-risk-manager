#ifndef IBROKER_ADAPTER_H
#define IBROKER_ADAPTER_H

#include "broker/BrokerResponse.h"
#include "domain/Order.h"

class IBrokerAdapter {
public:
    virtual ~IBrokerAdapter() = default;

    // Send an order to the broker for execution.
    // Returns the broker's response — accepted with an ID, or rejected with a reason.
    virtual BrokerResponse send(const Order& order) = 0;
};

#endif