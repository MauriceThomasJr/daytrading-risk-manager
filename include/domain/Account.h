#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <string>

class Account {
public:
    Account(std::string accountId, double startingBalance);

    const std::string& getAccountId() const;
    double getBalance() const;
    double getDailyPnL() const;
    int getTradesToday() const;

    void recordTradeResult(double pnl);
    void resetDay();

private:
    std::string accountId_;
    double balance_;
    double dailyPnL_ = 0.0;
    int tradesToday_ = 0;
};

#endif