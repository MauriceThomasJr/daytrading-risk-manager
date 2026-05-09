#include "risk/RiskManager.h"
#include <cmath>

RiskManager::RiskManager(double accountBalance, double riskPerTradePct, double maxDailyLossPct) {
    accountBalance_ = accountBalance;
    riskPerTradePct_ = riskPerTradePct; 
    maxDailyLossPct_ = maxDailyLossPct; 
}
int RiskManager::calculatePositionSize (double entryPrice, double stopPrice) const{

}
bool RiskManager::canTakeNewTrade() const{

}
void RiskManager::recordTradeResults(double pnl){

}
double RiskManager::GetAccBal() const{

}
double RiskManager::GetPnL() const{

}
int RiskManager::GetTradesToday() const{

}
