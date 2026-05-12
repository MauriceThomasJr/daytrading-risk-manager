#ifndef IN_MEMORY_CHECKLIST_TEMPLATE_STORE_H
#define IN_MEMORY_CHECKLIST_TEMPLATE_STORE_H

#include "storage/IChecklistTemplateStore.h"
#include <unordered_map>

class InMemoryChecklistTemplateStore : public IChecklistTemplateStore {
public:
    void save(const std::string& templateId,
              const ChecklistTemplate& tmpl) override;
    std::optional<ChecklistTemplate> load(
        const std::string& templateId) const override;
    bool exists(const std::string& templateId) const override;

    int size() const;

private:
    std::unordered_map<std::string, ChecklistTemplate> templates_;
};

#endif