#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <string>

class Account {
public:
    Account(std::string accountId, double startingBalance);

    // Factory for loading from persistent storage. Sets all fields directly,
    // bypassing the usual "balance = starting, pnl = 0, trades = 0" defaults.
    static Account fromStorage(std::string accountId,
                               double balance,
                               double dailyPnL,
                               int tradesToday);

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