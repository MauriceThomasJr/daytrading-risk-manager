#include <catch2/catch_test_macros.hpp>
#include "risk/RiskManager.h"
#include "domain/Account.h"
#include "domain/Instrument.h"

TEST_CASE("RiskManager calculates position size correctly", "[risk]") {
    Account account(100000.0);
    Instrument es("ES", 50.0, 0.25);
    RiskManager rules(0.01, 0.03, 5);  // 1% risk, 3% max loss, 5 trades

    SECTION("Standard trade with reasonable stop") {
        // 20-point stop on ES = $1,000 risk per contract.
        // 1% of $100k = $1,000 max risk.
        // Expect 1 contract.
        REQUIRE(rules.calculatePositionSize(account, es, 7000.0, 6980.0) == 1);
    }

    SECTION("Position size of zero when stop equals entry") {
        REQUIRE(rules.calculatePositionSize(account, es, 7000.0, 7000.0) == 0);
    }

    SECTION("Works for short trades (stop above entry)") {
        REQUIRE(rules.calculatePositionSize(account, es, 6980.0, 7000.0) == 1);
    }
}

TEST_CASE("RiskManager respects max trade count", "[risk]") {
    Account account(100000.0);
    RiskManager rules(0.01, 0.03, 2);  // only 2 trades allowed

    REQUIRE(rules.canTakeNewTrade(account) == true);
    account.recordTradeResult(50.0);
    REQUIRE(rules.canTakeNewTrade(account) == true);
    account.recordTradeResult(50.0);
    REQUIRE(rules.canTakeNewTrade(account) == false);  // hit the cap
}

TEST_CASE("RiskManager respects max daily loss", "[risk]") {
    Account account(100000.0);
    RiskManager rules(0.01, 0.03, 100);  // big trade cap, 3% daily loss limit

    REQUIRE(rules.canTakeNewTrade(account) == true);

    // Lose $2,000 — under the 3% ($3,000) limit, so still allowed.
    account.recordTradeResult(-2000.0);
    REQUIRE(rules.canTakeNewTrade(account) == true);

    // Lose another $2,000 — total $4,000 loss, over the limit. Now blocked.
    // Note: limit is 3% of *current* balance ($98,000 * 0.03 = $2,940).
    account.recordTradeResult(-2000.0);
    REQUIRE(rules.canTakeNewTrade(account) == false);
}

TEST_CASE("RiskManager works with different instruments", "[risk]") {
    Account account(100000.0);
    RiskManager rules(0.01, 0.03, 5);

    SECTION("ES at $50/point: 20-point stop = 1 contract") {
        Instrument es("ES", 50.0, 0.25);
        REQUIRE(rules.calculatePositionSize(account, es, 7000.0, 6980.0) == 1);
    }

    SECTION("MES at $5/point: 20-point stop = 10 contracts") {
        // $1,000 max risk / ($100 risk per contract) = 10 contracts.
        Instrument mes("MES", 5.0, 0.25);
        REQUIRE(rules.calculatePositionSize(account, mes, 7000.0, 6980.0) == 10);
    }

    SECTION("NQ at $20/point: 50-point stop = 1 contract") {
        // $1,000 max risk / ($1,000 risk per contract) = 1 contract.
        Instrument nq("NQ", 20.0, 0.25);
        REQUIRE(rules.calculatePositionSize(account, nq, 18000.0, 17950.0) == 1);
    }
}