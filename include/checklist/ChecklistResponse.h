#ifndef CHECKLIST_RESPONSE_H
#define CHECKLIST_RESPONSE_H

#include <string>
#include <unordered_map>

// Maps item IDs to their boolean responses (checked / not checked).
using ChecklistResponse = std::unordered_map<std::string, bool>;

#endif