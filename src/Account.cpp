#include "domain/Account.h"

Account::Account(double startingBalance) {
    balance_ = startingBalance;
}
void Account::recordTradeResult(double pnl) {
    balance_ += pnl;
    dailyPnL_ += pnl;
    ++tradesToday_;
}
double Account::getBalance() const { 
    return balance_; 
}
double Account::getDailyPnL() const { 
    return dailyPnL_ ;
}
int Account::getTradesToday() const {
    return tradesToday_;
}
void Account::resetDay() {
    dailyPnL_ = 0.0;
    tradesToday_ = 0;
}
