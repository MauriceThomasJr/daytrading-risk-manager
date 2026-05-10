#ifndef CHECKLIST_TEMPLATE_H
#define CHECKLIST_TEMPLATE_H
#include "checklist/ChecklistItem.h"
#include <vector>

class ChecklistTemplate {
public:
    void addItem(ChecklistItem item);
    const std::vector<ChecklistItem>& getItems() const;

private:
    std::vector<ChecklistItem> items_;

};
#endif