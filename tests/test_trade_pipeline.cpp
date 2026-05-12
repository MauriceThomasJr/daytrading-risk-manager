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
#include "journal/InMemoryTradeJournal.h"
#include "broker/MockBrokerAdapter.h"

// ---------- Helpers ----------

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

    InMemoryTradeJournal journal;
    MockBrokerAdapter broker;
    TradePipeline pipeline(ChecklistGate{}, RiskManager(0.01, 0.03, 5), journal, broker);

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

    InMemoryTradeJournal journal;
    MockBrokerAdapter broker;
    TradePipeline pipeline(ChecklistGate{}, RiskManager(0.01, 0.03, 5), journal, broker);

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
        InMemoryTradeJournal journal;
        MockBrokerAdapter broker;
        TradePipeline pipeline(ChecklistGate{}, RiskManager(0.01, 0.03, 1), journal, broker);

        account.recordTradeResult(50.0);

        auto result = pipeline.submit(intent, account, makeTemplate(), makeAllChecked());

        REQUIRE(result.accepted == false);
        REQUIRE(result.order.has_value() == false);
        REQUIRE(result.rejectionReasons.size() == 1);
        REQUIRE(result.rejectionReasons[0] == "Risk rules block new trades");
    }

    SECTION("Daily loss limit hit") {
        InMemoryTradeJournal journal;
        MockBrokerAdapter broker;
        TradePipeline pipeline(ChecklistGate{}, RiskManager(0.01, 0.01, 100), journal, broker);

        account.recordTradeResult(-2000.0);

        auto result = pipeline.submit(intent, account, makeTemplate(), makeAllChecked());

        REQUIRE(result.accepted == false);
        REQUIRE(result.rejectionReasons[0] == "Risk rules block new trades");
    }
}

TEST_CASE("TradePipeline rejects when position size would be zero", "[pipeline]") {
    Account account(1000.0);
    Instrument es("ES", 50.0, 0.25);
    TradeIntent intent(Side::Long, es, 7000.0, 6980.0);

    InMemoryTradeJournal journal;
    MockBrokerAdapter broker;
    TradePipeline pipeline(ChecklistGate{}, RiskManager(0.01, 0.03, 5), journal, broker);

    auto result = pipeline.submit(intent, account, makeTemplate(), makeAllChecked());

    REQUIRE(result.accepted == false);
    REQUIRE(result.order.has_value() == false);
    REQUIRE(result.rejectionReasons.size() == 1);
    REQUIRE(result.rejectionReasons[0] == "Position size would be zero");
}

TEST_CASE("TradePipeline checks gates in correct order", "[pipeline]") {
    Account account(1000.0);
    Instrument es("ES", 50.0, 0.25);
    TradeIntent intent(Side::Long, es, 7000.0, 6980.0);

    InMemoryTradeJournal journal;
    MockBrokerAdapter broker;
    TradePipeline pipeline(ChecklistGate{}, RiskManager(0.01, 0.03, 5), journal, broker);

    SECTION("Checklist failure reported first, even when risk would also fail") {
        ChecklistResponse responses;

        auto result = pipeline.submit(intent, account, makeTemplate(), responses);

        REQUIRE(result.accepted == false);
        REQUIRE(result.rejectionReasons.size() == 2);
        REQUIRE(result.rejectionReasons[0] == "Is there volume?");
    }
}

TEST_CASE("TradePipeline sizes correctly across instruments", "[pipeline]") {
    Account account(100000.0);
    InMemoryTradeJournal journal;
    MockBrokerAdapter broker;
    TradePipeline pipeline(ChecklistGate{}, RiskManager(0.01, 0.03, 5), journal, broker);

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

TEST_CASE("TradePipeline records accepted trades in the journal", "[pipeline]") {
    Account account(100000.0);
    Instrument es("ES", 50.0, 0.25);
    TradeIntent intent(Side::Long, es, 7000.0, 6980.0);

    InMemoryTradeJournal journal;
    MockBrokerAdapter broker;
    TradePipeline pipeline(ChecklistGate{}, RiskManager(0.01, 0.03, 5), journal, broker);

    SECTION("Accepted trade is journaled") {
        auto result = pipeline.submit(intent, account, makeTemplate(), makeAllChecked());

        REQUIRE(result.accepted);
        REQUIRE(journal.size() == 1);
        REQUIRE(journal.recentTrades(1)[0].getId() == result.order->getId());
    }

    SECTION("Rejected trade is not journaled") {
        ChecklistResponse incomplete;
        auto result = pipeline.submit(intent, account, makeTemplate(), incomplete);

        REQUIRE(result.accepted == false);
        REQUIRE(journal.size() == 0);
    }
}

// ---------- New tests for broker integration ----------

TEST_CASE("TradePipeline sends accepted trades to the broker", "[pipeline]") {
    Account account(100000.0);
    Instrument es("ES", 50.0, 0.25);
    TradeIntent intent(Side::Long, es, 7000.0, 6980.0);

    InMemoryTradeJournal journal;
    MockBrokerAdapter broker;
    TradePipeline pipeline(ChecklistGate{}, RiskManager(0.01, 0.03, 5), journal, broker);

    auto result = pipeline.submit(intent, account, makeTemplate(), makeAllChecked());

    REQUIRE(result.accepted);
    REQUIRE(broker.sentCount() == 1);
    REQUIRE(broker.sentOrders()[0].getId() == result.order->getId());
}

TEST_CASE("TradePipeline does not contact broker when gates reject", "[pipeline]") {
    Account account(100000.0);
    Instrument es("ES", 50.0, 0.25);
    TradeIntent intent(Side::Long, es, 7000.0, 6980.0);

    InMemoryTradeJournal journal;
    MockBrokerAdapter broker;
    TradePipeline pipeline(ChecklistGate{}, RiskManager(0.01, 0.03, 5), journal, broker);

    SECTION("Checklist failure: broker not called") {
        ChecklistResponse incomplete;
        pipeline.submit(intent, account, makeTemplate(), incomplete);
        REQUIRE(broker.sentCount() == 0);
    }

    SECTION("Risk failure: broker not called") {
        InMemoryTradeJournal localJournal;
        MockBrokerAdapter localBroker;
        TradePipeline strictPipeline(ChecklistGate{}, RiskManager(0.01, 0.03, 1),
                                     localJournal, localBroker);

        account.recordTradeResult(0.0);  // burn the one allowed trade
        strictPipeline.submit(intent, account, makeTemplate(), makeAllChecked());
        REQUIRE(localBroker.sentCount() == 0);
    }
}

TEST_CASE("TradePipeline propagates broker rejection", "[pipeline]") {
    Account account(100000.0);
    Instrument es("ES", 50.0, 0.25);
    TradeIntent intent(Side::Long, es, 7000.0, 6980.0);

    InMemoryTradeJournal journal;
    MockBrokerAdapter broker;
    broker.simulateRejection("Insufficient margin");

    TradePipeline pipeline(ChecklistGate{}, RiskManager(0.01, 0.03, 5), journal, broker);

    auto result = pipeline.submit(intent, account, makeTemplate(), makeAllChecked());

    REQUIRE(result.accepted == false);
    REQUIRE(result.order.has_value() == false);
    REQUIRE(result.rejectionReasons.size() == 1);
    REQUIRE(result.rejectionReasons[0] == "Broker rejected: Insufficient margin");
}

TEST_CASE("TradePipeline does not journal broker-rejected trades", "[pipeline]") {
    Account account(100000.0);
    Instrument es("ES", 50.0, 0.25);
    TradeIntent intent(Side::Long, es, 7000.0, 6980.0);

    InMemoryTradeJournal journal;
    MockBrokerAdapter broker;
    broker.simulateRejection("Market halted");

    TradePipeline pipeline(ChecklistGate{}, RiskManager(0.01, 0.03, 5), journal, broker);

    pipeline.submit(intent, account, makeTemplate(), makeAllChecked());

    REQUIRE(journal.size() == 0);
}