#ifndef ICHECKLIST_TEMPLATE_STORE_H
#define ICHECKLIST_TEMPLATE_STORE_H

#include "checklist/ChecklistTemplate.h"
#include <optional>
#include <string>

class IChecklistTemplateStore {
public:
    virtual ~IChecklistTemplateStore() = default;

    // Store or update a template under the given ID.
    virtual void save(const std::string& templateId,
                      const ChecklistTemplate& tmpl) = 0;

    // Load a template by ID. Returns nullopt if not found.
    virtual std::optional<ChecklistTemplate> load(
        const std::string& templateId) const = 0;

    // Check whether a template exists without loading it.
    virtual bool exists(const std::string& templateId) const = 0;
};

#endif