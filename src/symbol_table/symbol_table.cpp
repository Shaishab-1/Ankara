#include "symbol_table.h"

SymbolTable::SymbolTable() {
    // Start with the global scope already active (level 0),
    // since top-level declarations exist outside any block.
    scopes.emplace_back();
}

void SymbolTable::enterScope() {
    scopes.emplace_back();
}

void SymbolTable::exitScope() {
    // Guard against popping the global scope by mistake; the
    // global scope should only be removed when the table itself
    // is destroyed.
    if (scopes.size() > 1) {
        scopes.pop_back();
    }
}

bool SymbolTable::insert(const std::string& name, VarType type, int line) {
    auto& innermost = scopes.back();

    // Redeclaration check is scoped to the CURRENT level only;
    // a variable in an outer scope may be legally shadowed.
    if (innermost.find(name) != innermost.end()) {
        return false;
    }

    SymbolEntry entry{name, type, currentScopeLevel(), line};
    innermost[name] = entry;
    return true;
}

const SymbolEntry* SymbolTable::lookup(const std::string& name) const {
    // Search from innermost to outermost scope so that a variable
    // in a nested block correctly shadows one with the same name
    // in an outer block.
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return &found->second;
        }
    }
    return nullptr;
}

int SymbolTable::currentScopeLevel() const {
    return static_cast<int>(scopes.size()) - 1;
}