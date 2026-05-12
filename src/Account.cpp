#include "domain/Account.h"

#include <utility>

Account::Account(std::string accountId, double startingBalance)
    : accountId_(std::move(accountId)), balance_(startingBalance) {}

Account Account::fromStorage(std::string accountId,
                             double balance,
                             double dailyPnL,
                             int tradesToday) {
    Account a(std::move(accountId), balance);
    a.dailyPnL_ = dailyPnL;
    a.tradesToday_ = tradesToday;
    return a;
}    

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