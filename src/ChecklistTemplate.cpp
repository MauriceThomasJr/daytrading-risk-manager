#include "checklist/ChecklistTemplate.h"

void ChecklistTemplate::addItem(ChecklistItem item) {
    items_.push_back(item);
}
const std::vector<ChecklistItem>& ChecklistTemplate::getItems() const {
    return items_;
}