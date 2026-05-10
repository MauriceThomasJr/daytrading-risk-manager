#ifndef TRADE_SUBMISSION_RESULT_H
#define TRADE_SUBMISSION_RESULT_H

#include "domain/Order.h"
#include <optional>
#include <string>
#include <vector>

struct TradeSubmissionResult {
    bool accepted;
    std::optional<Order> order;
    std::vector<std::string> rejectionReasons;
};

#endif