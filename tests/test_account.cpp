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

TEST_CASE("Account recordTradeOpened increments trade count only", "[account]") {
    Account account("test-account", 10000.0);

    account.recordTradeOpened();

    REQUIRE(account.getTradesToday() == 1);
    REQUIRE(account.getBalance() == 10000.0);  // unchanged
    REQUIRE(account.getDailyPnL() == 0.0);     // unchanged
}

TEST_CASE("Account recordTradeResult updates balance and P&L only", "[account]") {
    Account account("test-account", 10000.0);

    SECTION("Winning trade increases balance and P&L") {
        account.recordTradeResult(150.0);
        REQUIRE(account.getBalance() == 10150.0);
        REQUIRE(account.getDailyPnL() == 150.0);
        REQUIRE(account.getTradesToday() == 0);  // not affected
    }

    SECTION("Losing trade decreases balance and P&L") {
        account.recordTradeResult(-100.0);
        REQUIRE(account.getBalance() == 9900.0);
        REQUIRE(account.getDailyPnL() == -100.0);
        REQUIRE(account.getTradesToday() == 0);
    }

    SECTION("Multiple results accumulate") {
        account.recordTradeResult(-50.0);
        account.recordTradeResult(100.0);
        account.recordTradeResult(-30.0);
        REQUIRE(account.getBalance() == 10020.0);
        REQUIRE(account.getDailyPnL() == 20.0);
    }
}

TEST_CASE("Account open and close together model a full trade lifecycle", "[account]") {
    Account account("test-account", 10000.0);

    // Open a trade
    account.recordTradeOpened();
    REQUIRE(account.getTradesToday() == 1);

    // Close it with a loss
    account.recordTradeResult(-100.0);

    REQUIRE(account.getBalance() == 9900.0);
    REQUIRE(account.getDailyPnL() == -100.0);
    REQUIRE(account.getTradesToday() == 1);
}

TEST_CASE("Account resetDay clears daily stats but preserves balance", "[account]") {
    Account account("test-account", 10000.0);
    account.recordTradeOpened();
    account.recordTradeOpened();
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