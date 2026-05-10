#ifndef CHECKLIST_EVALUATION_H
#define CHECKLIST_EVALUATION_H

#include <string>
#include <vector>

struct ChecklistEvaluation {
    bool passed;
    std::vector<std::string> failureReasons;
};

#endif