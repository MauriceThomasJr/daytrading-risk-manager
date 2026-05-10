#include <catch2/catch_test_macros.hpp>
#include "journal/SqliteTradeJournal.h"
#include "domain/Order.h"
#include "domain/TradeIntent.h"
#include "domain/Instrument.h"
#include "domain/Side.h"

#include <chrono>
#include <cstdio>
#include <filesystem>

namespace {
    Order makeOrder(double entry = 7000.0, double stop = 6980.0,
                    std::optional<double> target = std::nullopt) {
        Instrument es("ES", 50.0, 0.25);
        TradeIntent intent(Side::Long, es, entry, stop, target);
        return Order::fromValidatedIntent(intent, 1);
    }

    // RAII helper: ensures a test DB file is deleted when the test ends.
    struct TempDbFile {
        std::string path;
        explicit TempDbFile(const std::string& filename) : path(filename) {
            std::filesystem::remove(path);  // start clean
        }
        ~TempDbFile() {
            std::filesystem::remove(path);
        }
    };
}

TEST_CASE("SqliteTradeJournal starts empty on a new database", "[sqlite_journal]") {
    SqliteTradeJournal journal(":memory:");
    REQUIRE(journal.recentTrades(10).empty());
}

TEST_CASE("SqliteTradeJournal records and retrieves orders", "[sqlite_journal]") {
    SqliteTradeJournal journal(":memory:");

    Order original = makeOrder();
    journal.record(original);

    auto recent = journal.recentTrades(10);

    REQUIRE(recent.size() == 1);
    REQUIRE(recent[0].getId() == original.getId());
    REQUIRE(recent[0].getSize() == original.getSize());
}

TEST_CASE("SqliteTradeJournal round-trips every field correctly", "[sqlite_journal]") {
    SqliteTradeJournal journal(":memory:");

    // Build an order with a known set of values
    Instrument nq("NQ", 20.0, 0.25);
    TradeIntent intent(Side::Short, nq, 18000.0, 18050.0, 17950.0);
    Order original = Order::fromValidatedIntent(intent, 3);

    journal.record(original);
    auto loaded = journal.recentTrades(1);

    REQUIRE(loaded.size() == 1);

    const Order& roundTripped = loaded[0];
    REQUIRE(roundTripped.getId() == original.getId());
    REQUIRE(roundTripped.getSide() == Side::Short);
    REQUIRE(roundTripped.getInstrument().getSymbol() == "NQ");
    REQUIRE(roundTripped.getInstrument().getDollarPerPoint() == 20.0);
    REQUIRE(roundTripped.getInstrument().getTickSize() == 0.25);
    REQUIRE(roundTripped.getEntryPrice() == 18000.0);
    REQUIRE(roundTripped.getStopPrice() == 18050.0);
    REQUIRE(roundTripped.getTargetPrice().has_value());
    REQUIRE(roundTripped.getTargetPrice().value() == 17950.0);
    REQUIRE(roundTripped.getSize() == 3);
    REQUIRE(roundTripped.getCreatedAt() == original.getCreatedAt());
}

TEST_CASE("SqliteTradeJournal handles NULL target prices", "[sqlite_journal]") {
    SqliteTradeJournal journal(":memory:");

    Order withoutTarget = makeOrder(7000.0, 6980.0, std::nullopt);
    journal.record(withoutTarget);

    auto loaded = journal.recentTrades(1);

    REQUIRE(loaded.size() == 1);
    REQUIRE(loaded[0].getTargetPrice().has_value() == false);
}

TEST_CASE("SqliteTradeJournal returns trades newest first", "[sqlite_journal]") {
    SqliteTradeJournal journal(":memory:");

    Order first = makeOrder();
    Order second = makeOrder();
    Order third = makeOrder();

    journal.record(first);
    journal.record(second);
    journal.record(third);

    auto recent = journal.recentTrades(3);

    REQUIRE(recent.size() == 3);
    REQUIRE(recent[0].getId() == third.getId());
    REQUIRE(recent[1].getId() == second.getId());
    REQUIRE(recent[2].getId() == first.getId());
}

TEST_CASE("SqliteTradeJournal honors the limit parameter", "[sqlite_journal]") {
    SqliteTradeJournal journal(":memory:");

    for (int i = 0; i < 5; ++i) {
        journal.record(makeOrder());
    }

    REQUIRE(journal.recentTrades(2).size() == 2);
    REQUIRE(journal.recentTrades(5).size() == 5);
    REQUIRE(journal.recentTrades(100).size() == 5);
    REQUIRE(journal.recentTrades(0).empty());
    REQUIRE(journal.recentTrades(-1).empty());
}

TEST_CASE("SqliteTradeJournal persists across reopens", "[sqlite_journal]") {
    TempDbFile temp("test_persist.db");

    Order original = makeOrder();

    {
        // Write in the first journal instance
        SqliteTradeJournal writer(temp.path);
        writer.record(original);
    }   // writer goes out of scope, db file is closed

    {
        // Open a fresh instance against the same file
        SqliteTradeJournal reader(temp.path);
        auto recent = reader.recentTrades(10);

        REQUIRE(recent.size() == 1);
        REQUIRE(recent[0].getId() == original.getId());
    }
}

TEST_CASE("SqliteTradeJournal opening an existing DB is idempotent", "[sqlite_journal]") {
    TempDbFile temp("test_idempotent.db");

    // Create, write, close
    {
        SqliteTradeJournal journal(temp.path);
        journal.record(makeOrder());
    }

    // Reopen the same file multiple times — should not crash, should not lose data
    {
        SqliteTradeJournal journal(temp.path);
        REQUIRE(journal.recentTrades(10).size() == 1);
    }

    {
        SqliteTradeJournal journal(temp.path);
        journal.record(makeOrder());  // add another
        REQUIRE(journal.recentTrades(10).size() == 2);
    }
}

TEST_CASE("SqliteTradeJournal works through ITradeJournal interface", "[sqlite_journal]") {
    SqliteTradeJournal concrete(":memory:");
    ITradeJournal& journal = concrete;

    journal.record(makeOrder());
    journal.record(makeOrder());

    auto recent = journal.recentTrades(10);
    REQUIRE(recent.size() == 2);
}