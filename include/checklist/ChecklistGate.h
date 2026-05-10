#ifndef CHECKLIST_GATE_H
#define CHECKLIST_GATE_H

#include "checklist/ChecklistTemplate.h"
#include "checklist/ChecklistResponse.h"
#include "checklist/ChecklistEvaluation.h"

class ChecklistGate {
public:
    ChecklistEvaluation evaluate(const ChecklistTemplate& tmpl,
                                 const ChecklistResponse& response) const;
};

#endif