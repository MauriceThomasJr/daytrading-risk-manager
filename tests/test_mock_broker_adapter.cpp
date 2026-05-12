#include <catch2/catch_test_macros.hpp>
#include "broker/MockBrokerAdapter.h"
#include "domain/Order.h"
#include "domain/TradeIntent.h"
#include "domain/Instrument.h"
#include "domain/Side.h"

namespace {
    Order makeOrder() {
        Instrument es("ES", 50.0, 0.25);
        TradeIntent intent(Side::Long, es, 7000.0, 6980.0);
        return Order::fromValidatedIntent(intent, 1);
    }
}

TEST_CASE("MockBrokerAdapter starts empty", "[broker]") {
    MockBrokerAdapter broker;

    REQUIRE(broker.sentCount() == 0);
    REQUIRE(broker.sentOrders().empty());
}

TEST_CASE("MockBrokerAdapter accepts orders by default", "[broker]") {
    MockBrokerAdapter broker;

    auto response = broker.send(makeOrder());

    REQUIRE(response.accepted == true);
    REQUIRE(response.brokerOrderId.has_value());
    REQUIRE(response.rejectionReason.has_value() == false);
}

TEST_CASE("MockBrokerAdapter records sent orders", "[broker]") {
    MockBrokerAdapter broker;

    Order first = makeOrder();
    Order second = makeOrder();

    broker.send(first);
    broker.send(second);

    REQUIRE(broker.sentCount() == 2);
    REQUIRE(broker.sentOrders()[0].getId() == first.getId());
    REQUIRE(broker.sentOrders()[1].getId() == second.getId());
}

TEST_CASE("MockBrokerAdapter assigns unique broker IDs", "[broker]") {
    MockBrokerAdapter broker;

    auto first  = broker.send(makeOrder());
    auto second = broker.send(makeOrder());
    auto third  = broker.send(makeOrder());

    REQUIRE(first.brokerOrderId.has_value());
    REQUIRE(second.brokerOrderId.has_value());
    REQUIRE(third.brokerOrderId.has_value());

    REQUIRE(*first.brokerOrderId != *second.brokerOrderId);
    REQUIRE(*second.brokerOrderId != *third.brokerOrderId);
}

TEST_CASE("MockBrokerAdapter simulates rejection when configured", "[broker]") {
    MockBrokerAdapter broker;
    broker.simulateRejection("Insufficient margin");

    auto response = broker.send(makeOrder());

    REQUIRE(response.accepted == false);
    REQUIRE(response.rejectionReason.has_value());
    REQUIRE(*response.rejectionReason == "Insufficient margin");
    REQUIRE(response.brokerOrderId.has_value() == false);
}

TEST_CASE("MockBrokerAdapter does not record rejected orders", "[broker]") {
    MockBrokerAdapter broker;
    broker.simulateRejection("Market closed");

    broker.send(makeOrder());
    broker.send(makeOrder());

    REQUIRE(broker.sentCount() == 0);
    REQUIRE(broker.sentOrders().empty());
}

TEST_CASE("MockBrokerAdapter clearSimulation restores acceptance", "[broker]") {
    MockBrokerAdapter broker;

    broker.simulateRejection("Test rejection");
    auto rejected = broker.send(makeOrder());
    REQUIRE(rejected.accepted == false);

    broker.clearSimulation();
    auto accepted = broker.send(makeOrder());
    REQUIRE(accepted.accepted == true);
    REQUIRE(accepted.brokerOrderId.has_value());

    // Only the second order was actually recorded
    REQUIRE(broker.sentCount() == 1);
}

TEST_CASE("MockBrokerAdapter works through IBrokerAdapter interface", "[broker]") {
    MockBrokerAdapter concrete;
    IBrokerAdapter& broker = concrete;

    auto response = broker.send(makeOrder());

    REQUIRE(response.accepted == true);
    REQUIRE(concrete.sentCount() == 1);
}