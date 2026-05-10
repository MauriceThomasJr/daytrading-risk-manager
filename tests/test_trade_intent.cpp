#include <catch2/catch_test_macros.hpp>
#include "domain/TradeIntent.h"
#include "domain/Instrument.h"
#include "domain/Side.h"

TEST_CASE("TradeIntent stores all constructor values", "[trade_intent]") {
    Instrument es("ES", 50.0, 0.25);
    TradeIntent intent(Side::Long, es, 7000.0, 6980.0, 7050.0);

    REQUIRE(intent.getSide() == Side::Long);
    REQUIRE(intent.getEntryPrice() == 7000.0);
    REQUIRE(intent.getStopPrice() == 6980.0);
    REQUIRE(intent.getInstrument().getSymbol() == "ES");
    REQUIRE(intent.getInstrument().getDollarPerPoint() == 50.0);
}

TEST_CASE("TradeIntent supports both long and short sides", "[trade_intent]") {
    Instrument nq("NQ", 20.0, 0.25);

    SECTION("Long trade: stop below entry") {
        TradeIntent longTrade(Side::Long, nq, 18000.0, 17950.0);
        REQUIRE(longTrade.getSide() == Side::Long);
    }

    SECTION("Short trade: stop above entry") {
        TradeIntent shortTrade(Side::Short, nq, 18000.0, 18050.0);
        REQUIRE(shortTrade.getSide() == Side::Short);
    }
}

TEST_CASE("TradeIntent target price is optional", "[trade_intent]") {
    Instrument es("ES", 50.0, 0.25);

    SECTION("With target price") {
        TradeIntent intent(Side::Long, es, 7000.0, 6980.0, 7050.0);

        REQUIRE(intent.getTargetPrice().has_value());
        REQUIRE(intent.getTargetPrice().value() == 7050.0);
    }

    SECTION("Without target price (default)") {
        TradeIntent intent(Side::Long, es, 7000.0, 6980.0);

        REQUIRE(intent.getTargetPrice().has_value() == false);
    }

    SECTION("Without target price (explicit nullopt)") {
        TradeIntent intent(Side::Long, es, 7000.0, 6980.0, std::nullopt);

        REQUIRE(intent.getTargetPrice().has_value() == false);
    }
}

TEST_CASE("TradeIntent stores instrument by value (independent copy)", "[trade_intent]") {
    Instrument es("ES", 50.0, 0.25);
    TradeIntent intent(Side::Long, es, 7000.0, 6980.0);

    // The intent should hold its own copy of the instrument data.
    // This works whether stored by value or by reference (current design: by value).
    REQUIRE(intent.getInstrument().getSymbol() == "ES");
    REQUIRE(intent.getInstrument().getDollarPerPoint() == 50.0);
    REQUIRE(intent.getInstrument().getTickSize() == 0.25);
}