#include <catch2/catch_test_macros.hpp>
#include "domain/Account.h"

TEST_CASE("Account stores its ID", "[account]") {
    Account account("alice", 10000.0);
    REQUIRE(account.getAccountId() == "alice");
}

TEST_CASE("Account starts with given balance and zero stats", "[account]") {
    Account account("test-account", 10000.0);

    REQUIRE(account.getBalance() == 10000.0);
    REQUIRE(account.getDailyPnL() == 0.0);
    REQUIRE(account.getTradesToday() == 0);
}

TEST_CASE("Account updates state when trade is recorded", "[account]") {
    Account account("test-account", 10000.0);

    SECTION("Winning trade increases balance and P&L") {
        account.recordTradeResult(150.0);
        REQUIRE(account.getBalance() == 10150.0);
        REQUIRE(account.getDailyPnL() == 150.0);
        REQUIRE(account.getTradesToday() == 1);
    }

    SECTION("Losing trade decreases balance and P&L") {
        account.recordTradeResult(-100.0);
        REQUIRE(account.getBalance() == 9900.0);
        REQUIRE(account.getDailyPnL() == -100.0);
        REQUIRE(account.getTradesToday() == 1);
    }

    SECTION("Multiple trades accumulate correctly") {
        account.recordTradeResult(-50.0);
        account.recordTradeResult(100.0);
        account.recordTradeResult(-30.0);

        REQUIRE(account.getBalance() == 10020.0);
        REQUIRE(account.getDailyPnL() == 20.0);
        REQUIRE(account.getTradesToday() == 3);
    }
}

TEST_CASE("Account resetDay clears daily stats but preserves balance", "[account]") {
    Account account("test-account", 10000.0);
    account.recordTradeResult(-200.0);
    account.recordTradeResult(50.0);

    REQUIRE(account.getBalance() == 9850.0);
    REQUIRE(account.getDailyPnL() == -150.0);
    REQUIRE(account.getTradesToday() == 2);

    account.resetDay();

    REQUIRE(account.getBalance() == 9850.0);
    REQUIRE(account.getDailyPnL() == 0.0);
    REQUIRE(account.getTradesToday() == 0);
}

TEST_CASE("Account zero-pnl trade still counts as a trade", "[account]") {
    Account account("test-account", 10000.0);
    account.recordTradeResult(0.0);

    REQUIRE(account.getBalance() == 10000.0);
    REQUIRE(account.getDailyPnL() == 0.0);
    REQUIRE(account.getTradesToday() == 1);
}