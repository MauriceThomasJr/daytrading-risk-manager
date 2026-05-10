#include "risk/RiskManager.h"
#include <cmath>

RiskManager::RiskManager(double riskPerTradePct, double maxDailyLossPct, int maxTrades) {
    riskPerTradePct_ = riskPerTradePct; 
    maxDailyLossPct_ = maxDailyLossPct;
    maxTrades_ = maxTrades;
}
int RiskManager::calculatePositionSize (const Account& account, const Instrument& instrument, double entryPrice, double stopPrice) const{
    double riskPerContract = std::abs((entryPrice - stopPrice) * instrument.getDollarPerPoint());
    if (riskPerContract <= 0.0) return 0;
    double maxDollarRisk = account.getBalance() * riskPerTradePct_;
    return static_cast<int>(maxDollarRisk / riskPerContract);
}

bool RiskManager::canTakeNewTrade(const Account& account) const{
    if (account.getTradesToday() >= maxTrades_) return false;

    double maxDollarLoss = account.getBalance() * maxDailyLossPct_;
    if (account.getDailyPnL() <= -maxDollarLoss) return false;

    return true;
}
