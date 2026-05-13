#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <string>

class Account {
public:
    Account(std::string accountId, double startingBalance);

    static Account fromStorage(std::string accountId,
                               double balance,
                               double dailyPnL,
                               int tradesToday);

    const std::string& getAccountId() const;
    double getBalance() const;
    double getDailyPnL() const;
    int getTradesToday() const;

    // Called when a position is opened. Only increments trade count.
    void recordTradeOpened();

    // Called when a position closes with a realized P&L.
    // Updates balance and daily P&L; does NOT touch trade count.
    void recordTradeResult(double pnl);

    void resetDay();

private:
    std::string accountId_;
    double balance_;
    double dailyPnL_ = 0.0;
    int tradesToday_ = 0;
};

#endif