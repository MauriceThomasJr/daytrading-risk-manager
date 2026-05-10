#include <catch2/catch_test_macros.hpp>
#include "checklist/ChecklistTemplate.h"
#include "checklist/ChecklistItem.h"

TEST_CASE("ChecklistTemplate starts empty", "[checklist_template]") {
    ChecklistTemplate tmpl;
    REQUIRE(tmpl.getItems().size() == 0);
}

TEST_CASE("ChecklistTemplate stores added items", "[checklist_template]") {
    ChecklistTemplate tmpl;

    tmpl.addItem(ChecklistItem("volume", "Is there volume?"));
    tmpl.addItem(ChecklistItem("htf", "Higher timeframe checked?"));

    REQUIRE(tmpl.getItems().size() == 2);
    REQUIRE(tmpl.getItems()[0].getId() == "volume");
    REQUIRE(tmpl.getItems()[1].getId() == "htf");
}

TEST_CASE("ChecklistTemplate preserves insertion order", "[checklist_template]") {
    ChecklistTemplate tmpl;

    tmpl.addItem(ChecklistItem("first", "First question"));
    tmpl.addItem(ChecklistItem("second", "Second question"));
    tmpl.addItem(ChecklistItem("third", "Third question"));

    const auto& items = tmpl.getItems();
    REQUIRE(items[0].getPrompt() == "First question");
    REQUIRE(items[1].getPrompt() == "Second question");
    REQUIRE(items[2].getPrompt() == "Third question");
}