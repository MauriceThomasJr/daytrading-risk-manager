#include <catch2/catch_test_macros.hpp>
#include "risk/RiskManager.h"

TEST_CASE("RiskManager calculates position size correctly", "[risk]") {
    // $100,000 account, 1% risk per trade, 3% max daily loss,
    // 5 max trades per day, $50/point (ES futures).
    RiskManager rm(100000.0, 0.01, 0.03, 5, 50);

    SECTION("Standard trade with reasonable stop") {
        // 20-point stop on ES = $1,000 risk per contract.
        // 1% of $100k = $1,000 max risk.
        // Expect 1 contract.
        REQUIRE(rm.calculatePositionSize(7000.0, 6980.0) == 1);
    }

    SECTION("Position size of zero when stop equals entry") {
        // No risk per share = can't size = return 0.
        REQUIRE(rm.calculatePositionSize(7000.0, 7000.0) == 0);
    }

    SECTION("Works for short trades (stop above entry)") {
        // Same 20-point distance, just inverted.
        REQUIRE(rm.calculatePositionSize(6980.0, 7000.0) == 1);
    }
}

TEST_CASE("RiskManager respects max trade count", "[risk]") {
    RiskManager rm(100000.0, 0.01, 0.03, 2, 50);  // only 2 trades allowed

    REQUIRE(rm.canTakeNewTrade() == true);
    rm.recordTradeResults(50.0);
    REQUIRE(rm.canTakeNewTrade() == true);
    rm.recordTradeResults(50.0);
    REQUIRE(rm.canTakeNewTrade() == false);  // hit the cap
}

TEST_CASE("RiskManager updates state after trade", "[risk]") {
    RiskManager rm(100000.0, 0.01, 0.03, 5, 50);

    rm.recordTradeResults(-100.0);

    REQUIRE(rm.getAccBal() == 99900.0);
    REQUIRE(rm.getPnL() == -100.0);
    REQUIRE(rm.getTradesToday() == 1);
}