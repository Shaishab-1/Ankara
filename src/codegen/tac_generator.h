#ifndef TAC_GENERATOR_H
#define TAC_GENERATOR_H

#include <string>
#include <vector>
#include "../ast/ast.h"


class TACGenerator {
public:
    TACGenerator();

    // Generates TAC for the whole program and returns the
    // instruction list in order.
    std::vector<std::string> generate(ASTNode* root);

private:
    std::vector<std::string> instructions;
    int tempCount;
    int labelCount;

    std::string newTemp();
    std::string newLabel();

    void genStmtList(std::vector<ASTNode*>* stmts);
    void genStmt(ASTNode* stmt);

    // Returns the name (variable, temp, or literal text) holding
    // this expression's evaluated result.
    std::string genExpr(ASTNode* expr);

    void emit(const std::string& instruction);
};

#endif // TAC_GENERATOR_H