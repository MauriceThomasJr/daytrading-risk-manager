#include "risk/RiskManager.h"
#include <iostream>

int main() {
    RiskManager trading(100000.0, 0.01, 0.03, 5, 50);
    std::cout << std::boolalpha; 
    std::cout << "Position size: " << trading.calculatePositionSize(7000, 6980) << " contract(s)" << std::endl;
    std::cout << "Can trade? " << trading.canTakeNewTrade() << std::endl;
    trading.recordTradeResults(-100.0);
    std::cout << "Balance: $" << trading.getAccBal() << std::endl;
    std::cout << "Daily P&L: $" << trading.getPnL() << std::endl;
    std::cout << "Trades today: " << trading.getTradesToday() << std::endl;
    return 0;
}