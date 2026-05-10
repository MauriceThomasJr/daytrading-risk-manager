#include <catch2/catch_test_macros.hpp>
#include "journal/InMemoryTradeJournal.h"
#include "domain/Order.h"
#include "domain/TradeIntent.h"
#include "domain/Instrument.h"
#include "domain/Side.h"

namespace {
    // Helper: build a simple Order for tests.
    Order makeOrder() {
        Instrument es("ES", 50.0, 0.25);
        TradeIntent intent(Side::Long, es, 7000.0, 6980.0);
        return Order::fromValidatedIntent(intent, 1);
    }
}

TEST_CASE("InMemoryTradeJournal starts empty", "[journal]") {
    InMemoryTradeJournal journal;

    REQUIRE(journal.size() == 0);
    REQUIRE(journal.recentTrades(10).empty());
}

TEST_CASE("InMemoryTradeJournal records orders", "[journal]") {
    InMemoryTradeJournal journal;

    journal.record(makeOrder());
    journal.record(makeOrder());
    journal.record(makeOrder());

    REQUIRE(journal.size() == 3);
}

TEST_CASE("InMemoryTradeJournal returns recent trades newest first", "[journal]") {
    InMemoryTradeJournal journal;

    Order first  = makeOrder();
    Order second = makeOrder();
    Order third  = makeOrder();

    journal.record(first);
    journal.record(second);
    journal.record(third);

    auto recent = journal.recentTrades(3);

    REQUIRE(recent.size() == 3);
    REQUIRE(recent[0].getId() == third.getId());    // newest first
    REQUIRE(recent[1].getId() == second.getId());
    REQUIRE(recent[2].getId() == first.getId());
}

TEST_CASE("InMemoryTradeJournal honors the limit parameter", "[journal]") {
    InMemoryTradeJournal journal;

    for (int i = 0; i < 5; ++i) {
        journal.record(makeOrder());
    }

    SECTION("limit smaller than size") {
        REQUIRE(journal.recentTrades(2).size() == 2);
    }

    SECTION("limit equals size") {
        REQUIRE(journal.recentTrades(5).size() == 5);
    }

    SECTION("limit larger than size") {
        REQUIRE(journal.recentTrades(100).size() == 5);
    }

    SECTION("limit of zero returns empty") {
        REQUIRE(journal.recentTrades(0).empty());
    }

    SECTION("negative limit returns empty") {
        REQUIRE(journal.recentTrades(-1).empty());
    }
}

TEST_CASE("InMemoryTradeJournal can be used through ITradeJournal interface", "[journal]") {
    InMemoryTradeJournal concreteJournal;
    ITradeJournal& journal = concreteJournal;   // hold by interface reference

    journal.record(makeOrder());
    journal.record(makeOrder());

    auto recent = journal.recentTrades(10);
    REQUIRE(recent.size() == 2);
}