#ifndef ACCOUNT_H
#define ACCOUNT_H

class Account {
public:
    explicit Account(double startingBalance);
    void recordTradeResult(double pnl);
    double getBalance() const;
    double getDailyPnL() const;
    int getTradesToday() const;
    void resetDay();

private:
    double balance_;
    double dailyPnL_ = 0.0;
    int tradesToday_ = 0;
};
#endif