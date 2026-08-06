#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string>
#include <vector>
#include <unordered_map>
#include "../ast/ast.h"

// One declared variable's record (Section 4.4 required fields:
// Name, Type, Scope, Line Declared).
struct SymbolEntry {
    std::string name;
    VarType type;
    int scopeLevel;
    int lineDeclared;
};

// Manages nested scopes as a stack of hash maps. Each map holds
// the variables declared directly in that scope level; entering
// a block pushes a new map, leaving a block pops it.
class SymbolTable {
public:
    SymbolTable();

    // Enters a new nested scope (e.g. on '{').
    void enterScope();

    // Leaves the current scope, discarding all variables declared
    // in it (e.g. on '}').
    void exitScope();

    // Attempts to declare `name`. Returns false if `name` is
    // already declared in the CURRENT (innermost) scope only —
    // shadowing an outer scope's variable is allowed.
    bool insert(const std::string& name, VarType type, int line);

    // Searches from the innermost scope outward. Returns nullptr
    // if not found in any active scope.
    const SymbolEntry* lookup(const std::string& name) const;

    // Returns the current nesting depth (0 = global scope).
    int currentScopeLevel() const;

private:
    std::vector<std::unordered_map<std::string, SymbolEntry>> scopes;
};

#endif // SYMBOL_TABLE_H