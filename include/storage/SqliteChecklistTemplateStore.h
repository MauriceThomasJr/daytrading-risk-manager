#ifndef SQLITE_CHECKLIST_TEMPLATE_STORE_H
#define SQLITE_CHECKLIST_TEMPLATE_STORE_H

#include "storage/IChecklistTemplateStore.h"
#include <SQLiteCpp/SQLiteCpp.h>
#include <string>

class SqliteChecklistTemplateStore : public IChecklistTemplateStore {
public:
    explicit SqliteChecklistTemplateStore(const std::string& dbPath);

    void save(const std::string& templateId,
              const ChecklistTemplate& tmpl) override;
    std::optional<ChecklistTemplate> load(
        const std::string& templateId) const override;
    bool exists(const std::string& templateId) const override;

private:
    void ensureSchema();

    mutable SQLite::Database db_;
};

#endif