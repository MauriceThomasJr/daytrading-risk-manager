#ifndef BROKER_RESPONSE_H
#define BROKER_RESPONSE_H

#include <optional>
#include <string>

struct BrokerResponse {
    bool accepted;
    std::optional<std::string> brokerOrderId;     // present when accepted
    std::optional<std::string> rejectionReason;   // present when rejected
};

#endif