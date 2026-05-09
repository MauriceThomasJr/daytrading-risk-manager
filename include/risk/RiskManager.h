#ifndef RISK_MANAGER_H
#define RISK_MANAGER_H


class RiskManager {
    public:
    RiskManager(double accountBalance, double riskPerTradePct, double maxDailyLossPct);
    int calculatePositionSize (double entryPrice, double stopPrice) const;
    bool canTakeNewTrade() const;
    void recordTradeResults(double pnl);
    double GetAccBal() const;
    double GetPnL() const;
    int GetTradesToday() const;
   
    
    private:
    double accountBalance_;
    double riskPerTradePct_;
    double maxDailyLossPct_;
    double dailyPnL_;
    int tradesToday_ = 0;
};
#endif 
