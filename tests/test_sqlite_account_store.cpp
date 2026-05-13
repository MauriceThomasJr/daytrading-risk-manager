#include <catch2/catch_test_macros.hpp>
#include "storage/SqliteAccountStore.h"

#include <filesystem>

namespace {
    struct TempDbFile {
        std::string path;
        explicit TempDbFile(const std::string& filename) : path(filename) {
            std::filesystem::remove(path);
        }
        ~TempDbFile() {
            std::filesystem::remove(path);
        }
    };
}

TEST_CASE("SqliteAccountStore starts empty on a new database", "[sqlite_account_store]") {
    SqliteAccountStore store(":memory:");

    REQUIRE(store.exists("alice") == false);
    REQUIRE(store.load("alice").has_value() == false);
}

TEST_CASE("SqliteAccountStore saves and loads an account", "[sqlite_account_store]") {
    SqliteAccountStore store(":memory:");

    Account alice("alice", 50000.0);
    alice.recordTradeOpened();
    alice.recordTradeResult(-100.0);
    store.save(alice);

    auto loaded = store.load("alice");

    REQUIRE(loaded.has_value());
    REQUIRE(loaded->getAccountId() == "alice");
    REQUIRE(loaded->getBalance() == 49900.0);
    REQUIRE(loaded->getDailyPnL() == -100.0);
    REQUIRE(loaded->getTradesToday() == 1);
}

TEST_CASE("SqliteAccountStore handles multiple accounts", "[sqlite_account_store]") {
    SqliteAccountStore store(":memory:");

    store.save(Account("alice", 50000.0));
    store.save(Account("bob",   100000.0));
    store.save(Account("carol", 25000.0));

    REQUIRE(store.exists("alice"));
    REQUIRE(store.exists("bob"));
    REQUIRE(store.exists("carol"));
    REQUIRE(store.exists("dave") == false);

    REQUIRE(store.load("bob")->getBalance() == 100000.0);
}

TEST_CASE("SqliteAccountStore save overwrites existing accounts", "[sqlite_account_store]") {
    SqliteAccountStore store(":memory:");

    store.save(Account("alice", 50000.0));

    Account updated("alice", 75000.0);
    updated.recordTradeOpened();
    updated.recordTradeResult(-500.0);
    store.save(updated);

    auto loaded = store.load("alice");

    REQUIRE(loaded.has_value());
    REQUIRE(loaded->getBalance() == 74500.0);
    REQUIRE(loaded->getDailyPnL() == -500.0);
    REQUIRE(loaded->getTradesToday() == 1);
}

TEST_CASE("SqliteAccountStore persists across reopens", "[sqlite_account_store]") {
    TempDbFile temp("test_account_persist.db");

    {
        SqliteAccountStore writer(temp.path);
        Account alice("alice", 50000.0);
        alice.recordTradeOpened();
        alice.recordTradeOpened();
        alice.recordTradeResult(-200.0);
        alice.recordTradeResult(50.0);
        writer.save(alice);
    }   // writer destructs, DB closed

    {
        SqliteAccountStore reader(temp.path);
        auto loaded = reader.load("alice");

        REQUIRE(loaded.has_value());
        REQUIRE(loaded->getBalance() == 49850.0);
        REQUIRE(loaded->getDailyPnL() == -150.0);
        REQUIRE(loaded->getTradesToday() == 2);
    }
}

TEST_CASE("SqliteAccountStore round-trips all fields", "[sqlite_account_store]") {
    SqliteAccountStore store(":memory:");

    // Use fromStorage directly to set arbitrary values
    Account weird = Account::fromStorage("weird-account", 12345.67, -89.10, 42);
    store.save(weird);

    auto loaded = store.load("weird-account");

    REQUIRE(loaded.has_value());
    REQUIRE(loaded->getAccountId() == "weird-account");
    REQUIRE(loaded->getBalance() == 12345.67);
    REQUIRE(loaded->getDailyPnL() == -89.10);
    REQUIRE(loaded->getTradesToday() == 42);
}

TEST_CASE("SqliteAccountStore works through IAccountStore interface", "[sqlite_account_store]") {
    SqliteAccountStore concrete(":memory:");
    IAccountStore& store = concrete;

    store.save(Account("alice", 50000.0));

    REQUIRE(store.exists("alice"));
    REQUIRE(store.load("alice")->getBalance() == 50000.0);
}