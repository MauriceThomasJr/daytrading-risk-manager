#include <catch2/catch_test_macros.hpp>
#include "storage/InMemoryAccountStore.h"

TEST_CASE("InMemoryAccountStore starts empty", "[account_store]") {
    InMemoryAccountStore store;

    REQUIRE(store.size() == 0);
    REQUIRE(store.exists("alice") == false);
    REQUIRE(store.load("alice").has_value() == false);
}

TEST_CASE("InMemoryAccountStore saves and loads an account", "[account_store]") {
    InMemoryAccountStore store;

    Account original("alice", 50000.0);
    original.recordTradeOpened();
    original.recordTradeResult(-100.0);
    store.save(original);

    auto loaded = store.load("alice");

    REQUIRE(loaded.has_value());
    REQUIRE(loaded->getAccountId() == "alice");
    REQUIRE(loaded->getBalance() == 49900.0);
    REQUIRE(loaded->getDailyPnL() == -100.0);
    REQUIRE(loaded->getTradesToday() == 1);
}

TEST_CASE("InMemoryAccountStore tracks multiple accounts", "[account_store]") {
    InMemoryAccountStore store;

    store.save(Account("alice", 50000.0));
    store.save(Account("bob",   100000.0));
    store.save(Account("carol", 25000.0));

    REQUIRE(store.size() == 3);
    REQUIRE(store.exists("alice"));
    REQUIRE(store.exists("bob"));
    REQUIRE(store.exists("carol"));
    REQUIRE(store.exists("dave") == false);
}

TEST_CASE("InMemoryAccountStore returns nullopt for missing IDs", "[account_store]") {
    InMemoryAccountStore store;
    store.save(Account("alice", 50000.0));

    REQUIRE(store.load("not-a-real-id").has_value() == false);
    REQUIRE(store.load("").has_value() == false);
}

TEST_CASE("InMemoryAccountStore save overwrites existing accounts", "[account_store]") {
    InMemoryAccountStore store;

    Account original("alice", 50000.0);
    store.save(original);

    Account updated("alice", 75000.0);
    updated.recordTradeResult(-500.0);
    store.save(updated);

    auto loaded = store.load("alice");

    REQUIRE(loaded.has_value());
    REQUIRE(loaded->getBalance() == 74500.0);  // 75000 - 500
    REQUIRE(loaded->getDailyPnL() == -500.0);
    REQUIRE(store.size() == 1);  // still only one account
}

TEST_CASE("InMemoryAccountStore loaded account is independent of original", "[account_store]") {
    InMemoryAccountStore store;

    Account original("alice", 50000.0);
    store.save(original);

    // Mutating the original after save should not affect what's stored
    original.recordTradeResult(-1000.0);

    auto loaded = store.load("alice");
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->getBalance() == 50000.0);  // unchanged
    REQUIRE(loaded->getDailyPnL() == 0.0);
}

TEST_CASE("InMemoryAccountStore works through IAccountStore interface", "[account_store]") {
    InMemoryAccountStore concrete;
    IAccountStore& store = concrete;

    store.save(Account("alice", 50000.0));

    REQUIRE(store.exists("alice"));
    REQUIRE(store.load("alice")->getBalance() == 50000.0);
}