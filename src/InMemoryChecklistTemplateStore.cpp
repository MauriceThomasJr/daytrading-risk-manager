#include "storage/InMemoryChecklistTemplateStore.h"

void InMemoryChecklistTemplateStore::save(const std::string& templateId,
                                          const ChecklistTemplate& tmpl) {
    templates_.insert_or_assign(templateId, tmpl);
}

std::optional<ChecklistTemplate> InMemoryChecklistTemplateStore::load(
    const std::string& templateId) const {
    auto it = templates_.find(templateId);
    if (it == templates_.end()) {
        return std::nullopt;
    }
    return it->second;
}

bool InMemoryChecklistTemplateStore::exists(const std::string& templateId) const {
    return templates_.find(templateId) != templates_.end();
}

int InMemoryChecklistTemplateStore::size() const {
    return static_cast<int>(templates_.size());
}