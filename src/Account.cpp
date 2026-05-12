#include "domain/Account.h"

#include <utility>

Account::Account(std::string accountId, double startingBalance)
    : accountId_(std::move(accountId)), balance_(startingBalance) {}

const std::string& Account::getAccountId() const {
    return accountId_;
}

double Account::getBalance() const {
    return balance_;
}

double Account::getDailyPnL() const {
    return dailyPnL_;
}

int Account::getTradesToday() const {
    return tradesToday_;
}

void Account::recordTradeResult(double pnl) {
    balance_ += pnl;
    dailyPnL_ += pnl;
    ++tradesToday_;
}

void Account::resetDay() {
    dailyPnL_ = 0.0;
    tradesToday_ = 0;
}