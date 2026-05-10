#include "risk/RiskManager.h"
#include "domain/Account.h"
#include "domain/Instrument.h"
#include <iostream>

int main() {
    Account account(100000.0);
    Instrument es("ES", 50.0, 0.25);
    RiskManager rules(0.01, 0.03, 5);

    std::cout << std::boolalpha;
    std::cout << "Position size: "
              << rules.calculatePositionSize(account, es, 7000.0, 6980.0)
              << " contract(s)\n";
    std::cout << "Can trade?     " << rules.canTakeNewTrade(account) << "\n";

    account.recordTradeResult(-100.0);

    std::cout << "Balance:       $" << account.getBalance() << "\n";
    std::cout << "Daily P&L:     $" << account.getDailyPnL() << "\n";
    std::cout << "Trades today:  " << account.getTradesToday() << "\n";

    return 0;
}