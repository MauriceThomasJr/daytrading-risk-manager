// examples/basic_usage.cpp
//
// Demonstrates the full trade pipeline end-to-end:
//   1. Set up an account, instrument, risk rules, and a checklist
//   2. Submit a few trades (some pass, some fail)
//   3. Show what was journaled
//
// Run from build/ after `cmake --build .`:
//   ./basic_usage

#include "pipeline/TradePipeline.h"
#include "checklist/ChecklistGate.h"
#include "checklist/ChecklistTemplate.h"
#include "checklist/ChecklistResponse.h"
#include "checklist/ChecklistItem.h"
#include "domain/Account.h"
#include "domain/Instrument.h"
#include "domain/TradeIntent.h"
#include "domain/Side.h"
#include "risk/RiskManager.h"
#include "journal/SqliteTradeJournal.h"
#include "broker/MockBrokerAdapter.h"

#include <iostream>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {
    void printSection(const std::string& title) {
        std::cout << "\n=== " << title << " ===\n";
    }

    void printResult(const std::string& label, const TradeSubmissionResult& result) {
        std::cout << "\n" << label << "\n";
        if (result.accepted) {
            std::cout << "  ACCEPTED — Order #" << result.order->getId()
                      << " for " << result.order->getSize() << " contract(s)\n";
        } else {
            std::cout << "  REJECTED\n";
            for (const auto& reason : result.rejectionReasons) {
                std::cout << "    - " << reason << "\n";
            }
        }
    }
}

int main() {
    #ifdef _WIN32
      SetConsoleOutputCP(CP_UTF8);
    #endif
    std::cout << std::fixed << std::setprecision(2) << std::boolalpha;

    // -----------------------------------------------------------------
    // Set up the trader's environment
    // -----------------------------------------------------------------
    printSection("Setup");

    Account account("demo", 100000.0);
    Instrument es("ES", 50.0, 0.25);
    RiskManager rules(0.01, 0.03, 5);  // 1% per trade, 3% daily loss, 5 trades/day

    // The trader's pre-trade checklist — these would normally come from a UI
    ChecklistTemplate myChecklist;
    myChecklist.addItem(ChecklistItem("volume",   "Is there volume confirmation?"));
    myChecklist.addItem(ChecklistItem("htf",      "Did you check the higher timeframe?"));
    myChecklist.addItem(ChecklistItem("plan",     "Is this trade in your written plan?"));

    // Persistent journal. File appears in the working directory.
    SqliteTradeJournal journal("trades.db");

    MockBrokerAdapter broker;

    TradePipeline pipeline(ChecklistGate{}, rules, journal, broker);
    std::cout << "Trades sent to broker: " << broker.sentCount() << "\n";

    std::cout << "Account starting balance: $" << account.getBalance() << "\n";
    std::cout << "Instrument: " << es.getSymbol()
              << " ($" << es.getDollarPerPoint() << "/point)\n";
    std::cout << "Risk rules: 1% per trade, 3% max daily loss, 5 trades/day\n";
    std::cout << "Checklist has " << myChecklist.getItems().size() << " items\n";

    // -----------------------------------------------------------------
    // Trade attempt 1: incomplete checklist — should be rejected
    // -----------------------------------------------------------------
    printSection("Trade 1: incomplete checklist");

    TradeIntent intent1(Side::Long, es, 7000.0, 6980.0);

    ChecklistResponse responses1;
    responses1["volume"] = true;
    responses1["htf"]    = true;
    // "plan" intentionally left unanswered

    auto result1 = pipeline.submit(intent1, account, myChecklist, responses1);
    printResult("Submitting long ES @ 7000, stop 6980:", result1);

    // -----------------------------------------------------------------
    // Trade attempt 2: same trade, complete checklist — should pass
    // -----------------------------------------------------------------
    printSection("Trade 2: complete checklist");

    ChecklistResponse responses2;
    responses2["volume"] = true;
    responses2["htf"]    = true;
    responses2["plan"]   = true;

    auto result2 = pipeline.submit(intent1, account, myChecklist, responses2);
    printResult("Submitting long ES @ 7000, stop 6980:", result2);

    // Simulate the trade closing with a loss
    if (result2.accepted) {
        account.recordTradeResult(-100.0);
    }

    // -----------------------------------------------------------------
    // Trade attempt 3: short on a different leg, also passes
    // -----------------------------------------------------------------
    printSection("Trade 3: short ES");

    TradeIntent intent3(Side::Short, es, 7020.0, 7030.0, 6980.0);
    auto result3 = pipeline.submit(intent3, account, myChecklist, responses2);
    printResult("Submitting short ES @ 7020, stop 7030, target 6980:", result3);

    if (result3.accepted) {
        account.recordTradeResult(200.0);
    }

    // -----------------------------------------------------------------
    // Account state after the trades
    // -----------------------------------------------------------------
    printSection("Account state");

    std::cout << "Balance:       $" << account.getBalance() << "\n";
    std::cout << "Daily P&L:     $" << account.getDailyPnL() << "\n";
    std::cout << "Trades today:  " << account.getTradesToday() << "\n";

    // -----------------------------------------------------------------
    // What the journal recorded
    // -----------------------------------------------------------------
    printSection("Journaled trades");

    auto recent = journal.recentTrades(10);
    std::cout << recent.size() << " trade(s) on record:\n";
    for (const auto& order : recent) {
        std::cout << "  Order #" << order.getId()
                  << " — " << (order.getSide() == Side::Long ? "Long" : "Short")
                  << " " << order.getInstrument().getSymbol()
                  << " @ " << order.getEntryPrice()
                  << ", stop " << order.getStopPrice()
                  << ", size " << order.getSize() << "\n";
    }

    std::cout << "\nDatabase file: trades.db (try `sqlite3 trades.db` to inspect)\n";

    return 0;
}