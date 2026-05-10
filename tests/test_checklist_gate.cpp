#include <catch2/catch_test_macros.hpp>
#include "checklist/ChecklistGate.h"
#include "checklist/ChecklistTemplate.h"
#include "checklist/ChecklistResponse.h"
#include "checklist/ChecklistItem.h"

TEST_CASE("ChecklistGate passes when all items are checked", "[checklist_gate]") {
    ChecklistTemplate tmpl;
    tmpl.addItem(ChecklistItem("volume", "Is there volume?"));
    tmpl.addItem(ChecklistItem("htf", "Higher timeframe?"));

    ChecklistResponse responses;
    responses["volume"] = true;
    responses["htf"] = true;

    ChecklistGate gate;
    auto result = gate.evaluate(tmpl, responses);

    REQUIRE(result.passed == true);
    REQUIRE(result.failureReasons.empty());
}

TEST_CASE("ChecklistGate fails when any item is unchecked", "[checklist_gate]") {
    ChecklistTemplate tmpl;
    tmpl.addItem(ChecklistItem("volume", "Is there volume?"));
    tmpl.addItem(ChecklistItem("htf", "Higher timeframe?"));

    ChecklistGate gate;

    SECTION("One item false") {
        ChecklistResponse responses;
        responses["volume"] = true;
        responses["htf"] = false;

        auto result = gate.evaluate(tmpl, responses);

        REQUIRE(result.passed == false);
        REQUIRE(result.failureReasons.size() == 1);
        REQUIRE(result.failureReasons[0] == "Higher timeframe?");
    }

    SECTION("All items false") {
        ChecklistResponse responses;
        responses["volume"] = false;
        responses["htf"] = false;

        auto result = gate.evaluate(tmpl, responses);

        REQUIRE(result.passed == false);
        REQUIRE(result.failureReasons.size() == 2);
    }
}

TEST_CASE("ChecklistGate treats missing responses as failures", "[checklist_gate]") {
    ChecklistTemplate tmpl;
    tmpl.addItem(ChecklistItem("volume", "Is there volume?"));
    tmpl.addItem(ChecklistItem("htf", "Higher timeframe?"));

    SECTION("No responses at all") {
        ChecklistResponse responses;  // empty

        ChecklistGate gate;
        auto result = gate.evaluate(tmpl, responses);

        REQUIRE(result.passed == false);
        REQUIRE(result.failureReasons.size() == 2);
    }

    SECTION("Some responses missing") {
        ChecklistResponse responses;
        responses["volume"] = true;
        // "htf" not answered

        ChecklistGate gate;
        auto result = gate.evaluate(tmpl, responses);

        REQUIRE(result.passed == false);
        REQUIRE(result.failureReasons.size() == 1);
        REQUIRE(result.failureReasons[0] == "Higher timeframe?");
    }
}

TEST_CASE("ChecklistGate ignores extra responses not in template", "[checklist_gate]") {
    ChecklistTemplate tmpl;
    tmpl.addItem(ChecklistItem("volume", "Is there volume?"));

    ChecklistResponse responses;
    responses["volume"] = true;
    responses["unrelated"] = true;     // not in template
    responses["another_extra"] = false; // not in template

    ChecklistGate gate;
    auto result = gate.evaluate(tmpl, responses);

    REQUIRE(result.passed == true);
    REQUIRE(result.failureReasons.empty());
}

TEST_CASE("ChecklistGate passes vacuously on empty template", "[checklist_gate]") {
    ChecklistTemplate tmpl;  // no items added
    ChecklistResponse responses;

    ChecklistGate gate;
    auto result = gate.evaluate(tmpl, responses);

    REQUIRE(result.passed == true);
    REQUIRE(result.failureReasons.empty());
}

TEST_CASE("ChecklistGate failure reasons preserve item order", "[checklist_gate]") {
    ChecklistTemplate tmpl;
    tmpl.addItem(ChecklistItem("first", "First question"));
    tmpl.addItem(ChecklistItem("second", "Second question"));
    tmpl.addItem(ChecklistItem("third", "Third question"));

    ChecklistResponse responses;
    responses["first"] = false;
    responses["second"] = true;
    responses["third"] = false;

    ChecklistGate gate;
    auto result = gate.evaluate(tmpl, responses);

    REQUIRE(result.passed == false);
    REQUIRE(result.failureReasons.size() == 2);
    REQUIRE(result.failureReasons[0] == "First question");
    REQUIRE(result.failureReasons[1] == "Third question");
}