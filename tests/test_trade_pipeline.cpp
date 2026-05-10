#include <catch2/catch_test_macros.hpp>
#include "pipeline/TradePipeline.h"
#include "checklist/ChecklistGate.h"
#include "checklist/ChecklistTemplate.h"
#include "checklist/ChecklistResponse.h"
#include "checklist/ChecklistItem.h"
#include "domain/Account.h"
#include "domain/Instrument.h"
#include "domain/TradeIntent.h"
#include "domain/Side.h"
#include "risk/RiskManager.h"

// ---------- Helpers to keep individual tests focused ----------

static ChecklistTemplate makeTemplate() {
    ChecklistTemplate tmpl;
    tmpl.addItem(ChecklistItem("volume", "Is there volume?"));
    tmpl.addItem(ChecklistItem("htf",    "Higher timeframe checked?"));
    return tmpl;
}

static ChecklistResponse makeAllChecked() {
    ChecklistResponse r;
    r["volume"] = true;
    r["htf"]    = true;
    return r;
}

// ---------- Tests ----------

TEST_CASE("TradePipeline accepts a valid trade", "[pipeline]") {
    Account account(100000.0);
    Instrument es("ES", 50.0, 0.25);
    TradeIntent intent(Side::Long, es, 7000.0, 6980.0);

    TradePipeline pipeline(ChecklistGate{}, RiskManager(0.01, 0.03, 5));

    auto result = pipeline.submit(intent, account, makeTemplate(), makeAllChecked());

    REQUIRE(result.accepted == true);
    REQUIRE(result.order.has_value());
    REQUIRE(result.order->getSize() == 1);
    REQUIRE(result.order->getSide() == Side::Long);
    REQUIRE(result.order->getEntryPrice() == 7000.0);
    REQUIRE(result.rejectionReasons.empty());
}

TEST_CASE("TradePipeline rejects when checklist is incomplete", "[pipeline]") {
    Account account(100000.0);
    Instrument es("ES", 50.0, 0.25);
    TradeIntent intent(Side::Long, es, 7000.0, 6980.0);

    TradePipeline pipeline(ChecklistGate{}, RiskManager(0.01, 0.03, 5));

    SECTION("One item unchecked") {
        ChecklistResponse responses;
        responses["volume"] = true;
        responses["htf"]    = false;

        auto result = pipeline.submit(intent, account, makeTemplate(), responses);

        REQUIRE(result.accepted == false);
        REQUIRE(result.order.has_value() == false);
        REQUIRE(result.rejectionReasons.size() == 1);
        REQUIRE(result.rejectionReasons[0] == "Higher timeframe checked?");
    }

    SECTION("All items unchecked") {
        ChecklistResponse responses;
        responses["volume"] = false;
        responses["htf"]    = false;

        auto result = pipeline.submit(intent, account, makeTemplate(), responses);

        REQUIRE(result.accepted == false);
        REQUIRE(result.rejectionReasons.size() == 2);
    }

    SECTION("Empty response") {
        ChecklistResponse responses;

        auto result = pipeline.submit(intent, account, makeTemplate(), responses);

        REQUIRE(result.accepted == false);
        REQUIRE(result.rejectionReasons.size() == 2);
    }
}

TEST_CASE("TradePipeline rejects when risk rules block new trades", "[pipeline]") {
    Account account(100000.0);
    Instrument es("ES", 50.0, 0.25);
    TradeIntent intent(Side::Long, es, 7000.0, 6980.0);

    SECTION("Max trade count reached") {
        // Only 1 trade allowed per day
        TradePipeline pipeline(ChecklistGate{}, RiskManager(0.01, 0.03, 1));

        // Burn through the cap
        account.recordTradeResult(50.0);

        auto result = pipeline.submit(intent, account, makeTemplate(), makeAllChecked());

        REQUIRE(result.accepted == false);
        REQUIRE(result.order.has_value() == false);
        REQUIRE(result.rejectionReasons.size() == 1);
        REQUIRE(result.rejectionReasons[0] == "Risk rules block new trades");
    }

    SECTION("Daily loss limit hit") {
        // 1% max daily loss = $1,000 on $100k
        TradePipeline pipeline(ChecklistGate{}, RiskManager(0.01, 0.01, 100));

        // Drop into the lockout zone
        account.recordTradeResult(-2000.0);

        auto result = pipeline.submit(intent, account, makeTemplate(), makeAllChecked());

        REQUIRE(result.accepted == false);
        REQUIRE(result.rejectionReasons[0] == "Risk rules block new trades");
    }
}

TEST_CASE("TradePipeline rejects when position size would be zero", "[pipeline]") {
    Account account(1000.0);  // tiny account
    Instrument es("ES", 50.0, 0.25);

    // 20-point stop on ES = $1,000 risk per contract.
    // 1% of $1,000 = $10 max risk.
    // Even one contract is too big — size = 0.
    TradeIntent intent(Side::Long, es, 7000.0, 6980.0);

    TradePipeline pipeline(ChecklistGate{}, RiskManager(0.01, 0.03, 5));

    auto result = pipeline.submit(intent, account, makeTemplate(), makeAllChecked());

    REQUIRE(result.accepted == false);
    REQUIRE(result.order.has_value() == false);
    REQUIRE(result.rejectionReasons.size() == 1);
    REQUIRE(result.rejectionReasons[0] == "Position size would be zero");
}

TEST_CASE("TradePipeline checks gates in correct order", "[pipeline]") {
    // When multiple gates would fail, only the first one's reason is reported.
    // This pins down: checklist runs first, then risk, then sizing.

    Account account(1000.0);  // would fail sizing
    Instrument es("ES", 50.0, 0.25);
    TradeIntent intent(Side::Long, es, 7000.0, 6980.0);

    TradePipeline pipeline(ChecklistGate{}, RiskManager(0.01, 0.03, 5));

    SECTION("Checklist failure reported first, even when risk would also fail") {
        ChecklistResponse responses;  // empty — checklist will fail

        // If we ran sizing, it would also fail. But the checklist runs first.
        auto result = pipeline.submit(intent, account, makeTemplate(), responses);

        REQUIRE(result.accepted == false);
        // We get checklist failures, not "Position size would be zero"
        REQUIRE(result.rejectionReasons.size() == 2);
        REQUIRE(result.rejectionReasons[0] == "Is there volume?");
    }
}

TEST_CASE("TradePipeline sizes correctly across instruments", "[pipeline]") {
    Account account(100000.0);
    TradePipeline pipeline(ChecklistGate{}, RiskManager(0.01, 0.03, 5));

    SECTION("ES at $50/point: 1 contract") {
        Instrument es("ES", 50.0, 0.25);
        TradeIntent intent(Side::Long, es, 7000.0, 6980.0);

        auto result = pipeline.submit(intent, account, makeTemplate(), makeAllChecked());

        REQUIRE(result.accepted == true);
        REQUIRE(result.order->getSize() == 1);
    }

    SECTION("MES at $5/point: 10 contracts") {
        Instrument mes("MES", 5.0, 0.25);
        TradeIntent intent(Side::Long, mes, 7000.0, 6980.0);

        auto result = pipeline.submit(intent, account, makeTemplate(), makeAllChecked());

        REQUIRE(result.accepted == true);
        REQUIRE(result.order->getSize() == 10);
    }
}