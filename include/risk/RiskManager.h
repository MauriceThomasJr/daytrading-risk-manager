#ifndef RISK_MANAGER_H
#define RISK_MANAGER_H

#include "domain/Account.h"
#include "domain/Instrument.h"


class RiskManager {
public:
    RiskManager(double riskPerTradePct, double maxDailyLossPct, int maxTrades);
    int calculatePositionSize (const Account& account, const Instrument& instrument, double entryPrice, double stopPrice) const;
    bool canTakeNewTrade(const Account& account) const;

private:
    double riskPerTradePct_;
    double maxDailyLossPct_;
    int maxTrades_;
};
#endif 
