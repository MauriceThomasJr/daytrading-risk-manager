#include <catch2/catch_test_macros.hpp>
#include "storage/SqliteChecklistTemplateStore.h"
#include "checklist/ChecklistItem.h"

#include <filesystem>

namespace {
    struct TempDbFile {
        std::string path;
        explicit TempDbFile(const std::string& filename) : path(filename) {
            std::filesystem::remove(path);
        }
        ~TempDbFile() {
            std::filesystem::remove(path);
        }
    };

    ChecklistTemplate makeSampleTemplate() {
        ChecklistTemplate tmpl;
        tmpl.addItem(ChecklistItem("volume", "Is there volume?"));
        tmpl.addItem(ChecklistItem("htf", "Higher timeframe checked?"));
        tmpl.addItem(ChecklistItem("plan", "In trading plan?"));
        return tmpl;
    }
}

TEST_CASE("SqliteChecklistTemplateStore starts empty", "[sqlite_template_store]") {
    SqliteChecklistTemplateStore store(":memory:");

    REQUIRE(store.exists("daily-es") == false);
    REQUIRE(store.load("daily-es").has_value() == false);
}

TEST_CASE("SqliteChecklistTemplateStore saves and loads templates", "[sqlite_template_store]") {
    SqliteChecklistTemplateStore store(":memory:");

    store.save("daily-es", makeSampleTemplate());

    auto loaded = store.load("daily-es");

    REQUIRE(loaded.has_value());
    REQUIRE(loaded->getItems().size() == 3);
    REQUIRE(loaded->getItems()[0].getId() == "volume");
    REQUIRE(loaded->getItems()[0].getPrompt() == "Is there volume?");
    REQUIRE(loaded->getItems()[1].getId() == "htf");
    REQUIRE(loaded->getItems()[2].getId() == "plan");
}

TEST_CASE("SqliteChecklistTemplateStore preserves item order", "[sqlite_template_store]") {
    SqliteChecklistTemplateStore store(":memory:");

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

TEST_CASE("SqliteChecklistTemplateStore handles multiple templates", "[sqlite_template_store]") {
    SqliteChecklistTemplateStore store(":memory:");

    ChecklistTemplate es;
    es.addItem(ChecklistItem("es-vol", "ES volume?"));
    store.save("daily-es", es);

    ChecklistTemplate stocks;
    stocks.addItem(ChecklistItem("earnings", "Earnings checked?"));
    stocks.addItem(ChecklistItem("news", "Recent news scanned?"));
    store.save("swing-stocks", stocks);

    REQUIRE(store.exists("daily-es"));
    REQUIRE(store.exists("swing-stocks"));
    REQUIRE(store.exists("not-real") == false);

    REQUIRE(store.load("daily-es")->getItems().size() == 1);
    REQUIRE(store.load("swing-stocks")->getItems().size() == 2);
    REQUIRE(store.load("swing-stocks")->getItems()[0].getId() == "earnings");
}

TEST_CASE("SqliteChecklistTemplateStore save replaces existing items", "[sqlite_template_store]") {
    SqliteChecklistTemplateStore store(":memory:");

    // Original template with 3 items
    store.save("evolving", makeSampleTemplate());
    REQUIRE(store.load("evolving")->getItems().size() == 3);

    // Replace with a new template with 2 different items
    ChecklistTemplate replacement;
    replacement.addItem(ChecklistItem("new1", "New first"));
    replacement.addItem(ChecklistItem("new2", "New second"));
    store.save("evolving", replacement);

    auto loaded = store.load("evolving");

    REQUIRE(loaded.has_value());
    REQUIRE(loaded->getItems().size() == 2);
    REQUIRE(loaded->getItems()[0].getId() == "new1");
    REQUIRE(loaded->getItems()[1].getId() == "new2");
}

TEST_CASE("SqliteChecklistTemplateStore stores empty templates", "[sqlite_template_store]") {
    SqliteChecklistTemplateStore store(":memory:");

    ChecklistTemplate empty;
    store.save("empty", empty);

    REQUIRE(store.exists("empty"));

    auto loaded = store.load("empty");
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->getItems().empty());
}

TEST_CASE("SqliteChecklistTemplateStore distinguishes empty from missing", "[sqlite_template_store]") {
    SqliteChecklistTemplateStore store(":memory:");

    ChecklistTemplate empty;
    store.save("exists-but-empty", empty);

    // Empty template exists; missing template doesn't.
    REQUIRE(store.exists("exists-but-empty"));
    REQUIRE(store.exists("does-not-exist") == false);

    REQUIRE(store.load("exists-but-empty").has_value());
    REQUIRE(store.load("does-not-exist").has_value() == false);
}

TEST_CASE("SqliteChecklistTemplateStore persists across reopens", "[sqlite_template_store]") {
    TempDbFile temp("test_template_persist.db");

    {
        SqliteChecklistTemplateStore writer(temp.path);
        writer.save("daily-es", makeSampleTemplate());
    }   // writer destructs, DB closed

    {
        SqliteChecklistTemplateStore reader(temp.path);
        auto loaded = reader.load("daily-es");

        REQUIRE(loaded.has_value());
        REQUIRE(loaded->getItems().size() == 3);
        REQUIRE(loaded->getItems()[0].getId() == "volume");
        REQUIRE(loaded->getItems()[2].getId() == "plan");
    }
}

TEST_CASE("SqliteChecklistTemplateStore works through interface", "[sqlite_template_store]") {
    SqliteChecklistTemplateStore concrete(":memory:");
    IChecklistTemplateStore& store = concrete;

    store.save("test", makeSampleTemplate());

    REQUIRE(store.exists("test"));
    REQUIRE(store.load("test")->getItems().size() == 3);
}