#include <catch2/catch_test_macros.hpp>
#include "checklist/ChecklistItem.h"

TEST_CASE("ChecklistItem stores id and prompt", "[checklist_item]") {
    ChecklistItem item("volume", "Is there volume confirmation?");

    REQUIRE(item.getId() == "volume");
    REQUIRE(item.getPrompt() == "Is there volume confirmation?");
}

TEST_CASE("ChecklistItem supports varied content", "[checklist_item]") {
    SECTION("Short id and prompt") {
        ChecklistItem item("a", "Yes?");
        REQUIRE(item.getId() == "a");
        REQUIRE(item.getPrompt() == "Yes?");
    }

    SECTION("Long descriptive prompt") {
        ChecklistItem item(
            "htf_alignment",
            "Have you confirmed alignment with the higher timeframe trend?");
        REQUIRE(item.getPrompt().length() > 30);
    }
}