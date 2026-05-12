#include <catch2/catch_test_macros.hpp>
#include "storage/InMemoryChecklistTemplateStore.h"
#include "checklist/ChecklistItem.h"

namespace {
    ChecklistTemplate makeSampleTemplate() {
        ChecklistTemplate tmpl;
        tmpl.addItem(ChecklistItem("volume", "Is there volume?"));
        tmpl.addItem(ChecklistItem("htf", "Higher timeframe checked?"));
        tmpl.addItem(ChecklistItem("plan", "In trading plan?"));
        return tmpl;
    }
}

TEST_CASE("InMemoryChecklistTemplateStore starts empty", "[template_store]") {
    InMemoryChecklistTemplateStore store;

    REQUIRE(store.size() == 0);
    REQUIRE(store.exists("any-id") == false);
    REQUIRE(store.load("any-id").has_value() == false);
}

TEST_CASE("InMemoryChecklistTemplateStore saves and loads templates", "[template_store]") {
    InMemoryChecklistTemplateStore store;

    store.save("daily-es", makeSampleTemplate());

    auto loaded = store.load("daily-es");

    REQUIRE(loaded.has_value());
    REQUIRE(loaded->getItems().size() == 3);
    REQUIRE(loaded->getItems()[0].getId() == "volume");
    REQUIRE(loaded->getItems()[0].getPrompt() == "Is there volume?");
    REQUIRE(loaded->getItems()[1].getId() == "htf");
    REQUIRE(loaded->getItems()[2].getId() == "plan");
}

TEST_CASE("InMemoryChecklistTemplateStore preserves item order", "[template_store]") {
    InMemoryChecklistTemplateStore store;

    ChecklistTemplate tmpl;
    tmpl.addItem(ChecklistItem("third", "Third"));
    tmpl.addItem(ChecklistItem("first", "First"));
    tmpl.addItem(ChecklistItem("second", "Second"));
    store.save("ordered", tmpl);

    auto loaded = store.load("ordered");

    REQUIRE(loaded.has_value());
    REQUIRE(loaded->getItems()[0].getId() == "third");
    REQUIRE(loaded->getItems()[1].getId() == "first");
    REQUIRE(loaded->getItems()[2].getId() == "second");
}

TEST_CASE("InMemoryChecklistTemplateStore handles multiple templates", "[template_store]") {
    InMemoryChecklistTemplateStore store;

    ChecklistTemplate es;
    es.addItem(ChecklistItem("es-vol", "ES volume?"));
    store.save("daily-es", es);

    ChecklistTemplate stocks;
    stocks.addItem(ChecklistItem("earnings", "Earnings checked?"));
    stocks.addItem(ChecklistItem("news", "Recent news scanned?"));
    store.save("swing-stocks", stocks);

    REQUIRE(store.size() == 2);
    REQUIRE(store.exists("daily-es"));
    REQUIRE(store.exists("swing-stocks"));
    REQUIRE(store.exists("nothing") == false);

    REQUIRE(store.load("daily-es")->getItems().size() == 1);
    REQUIRE(store.load("swing-stocks")->getItems().size() == 2);
}

TEST_CASE("InMemoryChecklistTemplateStore overwrites on save", "[template_store]") {
    InMemoryChecklistTemplateStore store;

    ChecklistTemplate original;
    original.addItem(ChecklistItem("old", "Old item"));
    store.save("my-template", original);

    ChecklistTemplate replacement;
    replacement.addItem(ChecklistItem("new1", "New first"));
    replacement.addItem(ChecklistItem("new2", "New second"));
    store.save("my-template", replacement);

    auto loaded = store.load("my-template");

    REQUIRE(loaded.has_value());
    REQUIRE(loaded->getItems().size() == 2);
    REQUIRE(loaded->getItems()[0].getId() == "new1");
    REQUIRE(store.size() == 1);  // still only one entry
}

TEST_CASE("InMemoryChecklistTemplateStore stores empty templates", "[template_store]") {
    InMemoryChecklistTemplateStore store;

    ChecklistTemplate empty;
    store.save("empty", empty);

    auto loaded = store.load("empty");

    REQUIRE(loaded.has_value());
    REQUIRE(loaded->getItems().empty());
}

TEST_CASE("InMemoryChecklistTemplateStore works through interface", "[template_store]") {
    InMemoryChecklistTemplateStore concrete;
    IChecklistTemplateStore& store = concrete;

    store.save("test", makeSampleTemplate());

    REQUIRE(store.exists("test"));
    REQUIRE(store.load("test")->getItems().size() == 3);
}