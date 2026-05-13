#include "storage/SqliteChecklistTemplateStore.h"
#include "checklist/ChecklistItem.h"

SqliteChecklistTemplateStore::SqliteChecklistTemplateStore(const std::string& dbPath)
    : db_(dbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE) {
    ensureSchema();
}

void SqliteChecklistTemplateStore::ensureSchema() {
    db_.exec(
        "CREATE TABLE IF NOT EXISTS checklist_templates ("
        "  template_id TEXT PRIMARY KEY"
        ")"
    );
    db_.exec(
        "CREATE TABLE IF NOT EXISTS checklist_items ("
        "  template_id TEXT NOT NULL,"
        "  position    INTEGER NOT NULL,"
        "  item_id     TEXT NOT NULL,"
        "  prompt      TEXT NOT NULL,"
        "  PRIMARY KEY (template_id, item_id),"
        "  FOREIGN KEY (template_id) REFERENCES checklist_templates(template_id)"
        ")"
    );
}

void SqliteChecklistTemplateStore::save(const std::string& templateId,
                                        const ChecklistTemplate& tmpl) {
    // Wrap all the work in a transaction so the template + items land atomically.
    // If anything throws, the transaction's destructor rolls back automatically.
    SQLite::Transaction transaction(db_);

    // Step 1: ensure the parent row exists.
    {
        SQLite::Statement insertTemplate(db_,
            "INSERT OR IGNORE INTO checklist_templates (template_id) VALUES (?)"
        );
        insertTemplate.bind(1, templateId);
        insertTemplate.exec();
    }

    // Step 2: remove any existing items for this template.
    {
        SQLite::Statement deleteItems(db_,
            "DELETE FROM checklist_items WHERE template_id = ?"
        );
        deleteItems.bind(1, templateId);
        deleteItems.exec();
    }

    // Step 3: insert each item with its position, preserving order.
    {
        SQLite::Statement insertItem(db_,
            "INSERT INTO checklist_items "
            "(template_id, position, item_id, prompt) "
            "VALUES (?, ?, ?, ?)"
        );

        int position = 0;
        for (const auto& item : tmpl.getItems()) {
            insertItem.bind(1, templateId);
            insertItem.bind(2, position++);
            insertItem.bind(3, item.getId());
            insertItem.bind(4, item.getPrompt());
            insertItem.exec();
            insertItem.reset();  // ready to bind again for the next iteration
        }
    }

    transaction.commit();
}

std::optional<ChecklistTemplate> SqliteChecklistTemplateStore::load(
    const std::string& templateId) const {
    // First confirm the template exists. If not, return early.
    if (!exists(templateId)) {
        return std::nullopt;
    }

    // Load all items for this template, in order.
    SQLite::Statement query(db_,
        "SELECT item_id, prompt FROM checklist_items "
        "WHERE template_id = ? "
        "ORDER BY position ASC"
    );
    query.bind(1, templateId);

    ChecklistTemplate tmpl;
    while (query.executeStep()) {
        tmpl.addItem(ChecklistItem(
            query.getColumn(0).getString(),
            query.getColumn(1).getString()
        ));
    }

    return tmpl;
}

bool SqliteChecklistTemplateStore::exists(const std::string& templateId) const {
    SQLite::Statement query(db_,
        "SELECT 1 FROM checklist_templates WHERE template_id = ? LIMIT 1"
    );
    query.bind(1, templateId);

    return query.executeStep();
}