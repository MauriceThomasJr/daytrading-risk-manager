#include <catch2/catch_test_macros.hpp>
#include "domain/Order.h"
#include "domain/TradeIntent.h"
#include "domain/Instrument.h"
#include "domain/Side.h"

TEST_CASE("Order is built from a TradeIntent and a size", "[order]") {
    Instrument es("ES", 50.0, 0.25);
    TradeIntent intent(Side::Long, es, 7000.0, 6980.0, 7050.0);

    Order order = Order::fromValidatedIntent(intent, 3);

    SECTION("Size is set from the factory call") {
        REQUIRE(order.getSize() == 3);
    }

    SECTION("Order delegates trade fields to the intent") {
        REQUIRE(order.getSide() == Side::Long);
        REQUIRE(order.getEntryPrice() == 7000.0);
        REQUIRE(order.getStopPrice() == 6980.0);
        REQUIRE(order.getInstrument().getSymbol() == "ES");
    }

    SECTION("Order preserves the optional target price") {
        REQUIRE(order.getTargetPrice().has_value());
        REQUIRE(order.getTargetPrice().value() == 7050.0);
    }
}

TEST_CASE("Order works without a target price", "[order]") {
    Instrument mes("MES", 5.0, 0.25);
    TradeIntent intent(Side::Short, mes, 7000.0, 7020.0);

    Order order = Order::fromValidatedIntent(intent, 5);

    REQUIRE(order.getSide() == Side::Short);
    REQUIRE(order.getSize() == 5);
    REQUIRE(order.getTargetPrice().has_value() == false);
}

TEST_CASE("Order supports a range of valid sizes", "[order]") {
    Instrument es("ES", 50.0, 0.25);
    TradeIntent intent(Side::Long, es, 7000.0, 6980.0);

    SECTION("Single-contract order") {
        Order order = Order::fromValidatedIntent(intent, 1);
        REQUIRE(order.getSize() == 1);
    }

    SECTION("Larger order") {
        Order order = Order::fromValidatedIntent(intent, 10);
        REQUIRE(order.getSize() == 10);
    }
}
TEST_CASE("Order assigns a unique sequential ID", "[order]") {
    Instrument es("ES", 50.0, 0.25);
    TradeIntent intent(Side::Long, es, 7000.0, 6980.0);

    Order a = Order::fromValidatedIntent(intent, 1);
    Order b = Order::fromValidatedIntent(intent, 1);
    Order c = Order::fromValidatedIntent(intent, 1);

    REQUIRE(a.getId() != b.getId());
    REQUIRE(b.getId() != c.getId());
    REQUIRE(b.getId() == a.getId() + 1);
    REQUIRE(c.getId() == b.getId() + 1);
}

TEST_CASE("Order timestamps its creation", "[order]") {
    Instrument es("ES", 50.0, 0.25);
    TradeIntent intent(Side::Long, es, 7000.0, 6980.0);

    auto before = std::chrono::system_clock::now();
    Order order = Order::fromValidatedIntent(intent, 1);
    auto after = std::chrono::system_clock::now();

    REQUIRE(order.getCreatedAt() >= before);
    REQUIRE(order.getCreatedAt() <= after);
}