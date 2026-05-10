#ifndef RISK_MANAGER_H
#define RISK_MANAGER_H


class RiskManager {
    public:
    RiskManager(double accountBalance, double riskPerTradePct, double maxDailyLossPct, int maxTrades, int dollarPerPoint);
    int calculatePositionSize (double entryPrice, double stopPrice) const;
    bool canTakeNewTrade() const;
    void recordTradeResult(double pnl);
    double getAccBal() const;
    double getPnL() const;
    int getTradesToday() const;
   
    
    private:
    double accountBalance_;
    double riskPerTradePct_;
    double maxDailyLossPct_;
    double dailyPnL_ = 0.0;
    int tradesToday_ = 0;
    int maxTrades_;
    int dollarPerPoint_;

};
#endif 
