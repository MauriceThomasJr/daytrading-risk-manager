#include "risk/RiskManager.h"
#include <cmath>

RiskManager::RiskManager(double accountBalance, double riskPerTradePct, double maxDailyLossPct, int maxTrades, int dollarPerPoint) {
    accountBalance_ = accountBalance;
    riskPerTradePct_ = riskPerTradePct; 
    maxDailyLossPct_ = maxDailyLossPct;
    maxTrades_ = maxTrades;
    dollarPerPoint_ = dollarPerPoint;
}
int RiskManager::calculatePositionSize (double entryPrice, double stopPrice) const{
    double riskPerContract = std::abs((entryPrice - stopPrice) * dollarPerPoint_);
    if (riskPerContract <= 0.0) return 0;
    double maxDollarRisk = accountBalance_ * riskPerTradePct_;
    return static_cast<int>(maxDollarRisk / riskPerContract);
}

bool RiskManager::canTakeNewTrade() const{
    if (tradesToday_ >= maxTrades_) return false;

    double maxDollarLoss = accountBalance_ * maxDailyLossPct_;
    if (dailyPnL_ <= -maxDollarLoss) return false;

    return true;
}
void RiskManager::recordTradeResult(double pnl){
    dailyPnL_ += pnl;
    accountBalance_ += pnl;
    ++tradesToday_;
}
double RiskManager::getAccBal() const{
    return accountBalance_;
}
double RiskManager::getPnL() const{
    return dailyPnL_;
}
int RiskManager::getTradesToday() const{
    return tradesToday_;
}
