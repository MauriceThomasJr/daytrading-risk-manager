#include "checklist/ChecklistItem.h"

ChecklistItem::ChecklistItem(std::string id, std::string prompt) 
    : id_(id), prompt_(prompt) {}

const std::string& ChecklistItem::getId() const {
    return id_;
}
const std::string& ChecklistItem::getPrompt() const {
    return prompt_;
}