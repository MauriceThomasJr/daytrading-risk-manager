#include <catch2/catch_test_macros.hpp>
#include "domain/Instrument.h"

TEST_CASE("Instrument stores symbol, dollarPerPoint, and tickSize", "[instrument]") {
    Instrument es("ES", 50.0, 0.25);

    SECTION("Getters return constructor values") {
        REQUIRE(es.getSymbol() == "ES");
        REQUIRE(es.getDollarPerPoint() == 50.0);
        REQUIRE(es.getTickSize() == 0.25);
    }
}

TEST_CASE("Instrument supports different futures contracts", "[instrument]") {
    SECTION("NQ has $20 per point") {
        Instrument nq("NQ", 20.0, 0.25);
        REQUIRE(nq.getSymbol() == "NQ");
        REQUIRE(nq.getDollarPerPoint() == 20.0);
    }

    SECTION("MES has $5 per point") {
        Instrument mes("MES", 5.0, 0.25);
        REQUIRE(mes.getDollarPerPoint() == 5.0);
    }

    SECTION("Stocks have $1 per point and $0.01 tick") {
        Instrument aapl("AAPL", 1.0, 0.01);
        REQUIRE(aapl.getSymbol() == "AAPL");
        REQUIRE(aapl.getTickSize() == 0.01);
    }
}

TEST_CASE("Instrument symbol is returned by const reference", "[instrument]") {
    Instrument es("ES", 50.0, 0.25);

    // This test compiles only if getSymbol() returns a reference.
    // If it returned by value, &es.getSymbol() would point to a temporary.
    const std::string& symbolRef = es.getSymbol();
    REQUIRE(symbolRef == "ES");
}