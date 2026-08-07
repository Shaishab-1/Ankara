#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "../ast/ast.h"
#include "../symbol_table/symbol_table.h"


class SemanticAnalyzer {
public:
    SemanticAnalyzer();

    // Walks the whole program. Returns true if no semantic errors
    // were found.
    bool analyze(ProgramNode* root);

    int errorCount() const { return errors; }

private:
    SymbolTable symTab;
    int errors;

    void visitStmt(ASTNode* stmt);
    void visitBlock(BlockNode* block);
    void visitDecl(DeclNode* decl);
    void visitAssign(AssignNode* assign);
    void visitIf(IfNode* node);
    void visitWhile(WhileNode* node);
    void visitPrint(PrintNode* node);

  
    VarType resolveExprType(ASTNode* expr);

    void reportError(int line, const std::string& message);
};

#endif // SEMANTIC_ANALYZER_H