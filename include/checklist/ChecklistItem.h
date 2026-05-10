#ifndef CHECKLIST_ITEM_H
#define CHECKLIST_ITEM_H
#include <string>

class ChecklistItem {
public:
    ChecklistItem(std::string id, std::string prompt);
    const std::string& getId() const;
    const std::string& getPrompt() const;
private:
    std::string id_;
    std::string prompt_;
};
#endif