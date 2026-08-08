#include "semantic_analyzer.h"
#include <iostream>

SemanticAnalyzer::SemanticAnalyzer() : errors(0) {}

void SemanticAnalyzer::reportError(int line, const std::string& message) {
    std::cerr << "Semantic Error at line " << line << ": " << message << "\n";
    errors++;
}

bool SemanticAnalyzer::analyze(ProgramNode* root) {
    if (!root) return true;
    for (ASTNode* stmt : *root->statements) {
        visitStmt(stmt);
    }
    return errors == 0;
}

void SemanticAnalyzer::visitStmt(ASTNode* stmt) {
    if (!stmt) return;

    if (auto* d = dynamic_cast<DeclNode*>(stmt))        { visitDecl(d); }
    else if (auto* a = dynamic_cast<AssignNode*>(stmt))  { visitAssign(a); }
    else if (auto* i = dynamic_cast<IfNode*>(stmt))      { visitIf(i); }
    else if (auto* w = dynamic_cast<WhileNode*>(stmt))   { visitWhile(w); }
    else if (auto* p = dynamic_cast<PrintNode*>(stmt))   { visitPrint(p); }
    else if (auto* b = dynamic_cast<BlockNode*>(stmt))   { visitBlock(b); }
    
}

void SemanticAnalyzer::visitBlock(BlockNode* block) {
    symTab.enterScope();
    for (ASTNode* stmt : *block->statements) {
        visitStmt(stmt);
    }
    symTab.exitScope();
}

void SemanticAnalyzer::visitDecl(DeclNode* decl) {
    
    bool ok = symTab.insert(decl->name, decl->type, decl->line);
    if (!ok) {
        reportError(decl->line,
            "redeclaration of variable '" + decl->name + "'");
    }
}

void SemanticAnalyzer::visitAssign(AssignNode* assign) {
    const SymbolEntry* entry = symTab.lookup(assign->name);
    if (!entry) {
        // Section 4.5: "Undeclared variable use" / "Scope violation"
        reportError(assign->line,
            "assignment to undeclared variable '" + assign->name + "'");
        // Still resolve the RHS so further errors inside it are found.
        resolveExprType(assign->expr);
        return;
    }

    VarType rhsType = resolveExprType(assign->expr);

    
    if (entry->type != rhsType) {
        reportError(assign->line,
            "type mismatch: cannot assign " + varTypeToString(rhsType) +
            " to variable '" + assign->name + "' of type " +
            varTypeToString(entry->type));
    }
}

void SemanticAnalyzer::visitIf(IfNode* node) {
    VarType condType = resolveExprType(node->condition);
    if (condType != VarType::TYPE_BOOL) {
        reportError(0, "if condition must be of type bool");
    }
    visitStmt(node->thenBlock);
    if (node->elseBlock) {
        visitStmt(node->elseBlock);
    }
}

void SemanticAnalyzer::visitWhile(WhileNode* node) {
    VarType condType = resolveExprType(node->condition);
    if (condType != VarType::TYPE_BOOL) {
        reportError(0, "while condition must be of type bool");
    }
    visitStmt(node->body);
}

void SemanticAnalyzer::visitPrint(PrintNode* node) {
    // print accepts any type — just resolve it so nested errors
    // (e.g. an undeclared variable inside the expression) surface.
    resolveExprType(node->expr);
}

VarType SemanticAnalyzer::resolveExprType(ASTNode* expr) {
    if (!expr) return VarType::TYPE_INT;

    if (auto* lit = dynamic_cast<IntLiteralNode*>(expr)) {
        (void)lit;
        return VarType::TYPE_INT;
    }
    if (auto* lit = dynamic_cast<FloatLiteralNode*>(expr)) {
        (void)lit;
        return VarType::TYPE_FLOAT;
    }
    if (auto* lit = dynamic_cast<BoolLiteralNode*>(expr)) {
        (void)lit;
        return VarType::TYPE_BOOL;
    }

    if (auto* id = dynamic_cast<IdNode*>(expr)) {
        const SymbolEntry* entry = symTab.lookup(id->name);
        if (!entry) {
            // Section 4.5: "Undeclared variable use"
            reportError(id->line,
                "use of undeclared variable '" + id->name + "'");
            return VarType::TYPE_INT; // fallback so traversal continues
        }
        return entry->type;
    }

    if (auto* un = dynamic_cast<UnaryOpNode*>(expr)) {
        VarType operandType = resolveExprType(un->operand);
        if (un->op == "!") {
            if (operandType != VarType::TYPE_BOOL) {
                reportError(0, "logical NOT (!) requires a bool operand");
            }
            return VarType::TYPE_BOOL;
        }
        // unary minus
        if (operandType == VarType::TYPE_BOOL) {
            reportError(0, "unary minus (-) cannot be applied to bool");
        }
        return operandType;
    }

    if (auto* bin = dynamic_cast<BinaryOpNode*>(expr)) {
        VarType leftType  = resolveExprType(bin->left);
        VarType rightType = resolveExprType(bin->right);
        const std::string& op = bin->op;

        bool isLogical    = (op == "&&" || op == "||");
        bool isRelational = (op == "<" || op == ">" || op == "<=" ||
                              op == ">=" || op == "==" || op == "!=");
        bool isArithmetic = (op == "+" || op == "-" || op == "*" ||
                              op == "/" || op == "%");

        if (isLogical) {
            // Section 4.5: "Invalid expressions — applying logical
            // operators to numeric operands where not permitted."
            if (leftType != VarType::TYPE_BOOL || rightType != VarType::TYPE_BOOL) {
                reportError(0,
                    "logical operator '" + op + "' requires bool operands");
            }
            return VarType::TYPE_BOOL;
        }

        if (isRelational) {
            if (leftType != rightType) {
                reportError(0,
                    "cannot compare " + varTypeToString(leftType) +
                    " with " + varTypeToString(rightType));
            }
            return VarType::TYPE_BOOL;
        }

        if (isArithmetic) {
            if (leftType == VarType::TYPE_BOOL || rightType == VarType::TYPE_BOOL) {
                reportError(0,
                    "arithmetic operator '" + op + "' cannot be applied to bool");
                return VarType::TYPE_INT;
            }
            // int op int -> int; anything involving float -> float.
            if (leftType == VarType::TYPE_FLOAT || rightType == VarType::TYPE_FLOAT) {
                return VarType::TYPE_FLOAT;
            }
            return VarType::TYPE_INT;
        }
    }

    // Should not be reached given the current grammar.
    return VarType::TYPE_INT;
}