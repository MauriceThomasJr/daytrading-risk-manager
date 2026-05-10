#include "checklist/ChecklistGate.h"

ChecklistEvaluation ChecklistGate::evaluate(const ChecklistTemplate& tmpl,
                                            const ChecklistResponse& response) const {
    ChecklistEvaluation result;
    result.passed = true;

    for (const auto& item : tmpl.getItems()) {
        auto it = response.find(item.getId());
        bool satisfied = (it != response.end() && it->second);

        if (!satisfied) {
            result.passed = false;
            result.failureReasons.push_back(item.getPrompt());
        }
    }

    return result;
}