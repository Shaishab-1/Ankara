#include "tac_generator.h"
#include <sstream>

TACGenerator::TACGenerator() : tempCount(0), labelCount(0) {}

std::string TACGenerator::newTemp() {
    return "t" + std::to_string(++tempCount);
}

std::string TACGenerator::newLabel() {
    return "L" + std::to_string(++labelCount);
}

void TACGenerator::emit(const std::string& instruction) {
    instructions.push_back(instruction);
}

std::vector<std::string> TACGenerator::generate(ASTNode* root) {
    ProgramNode* program = dynamic_cast<ProgramNode*>(root);
    if (program) {
        genStmtList(program->statements);
    }
    return instructions;
}

void TACGenerator::genStmtList(std::vector<ASTNode*>* stmts) {
    if (!stmts) return;
    for (ASTNode* stmt : *stmts) {
        if (stmt) genStmt(stmt);
    }
}

void TACGenerator::genStmt(ASTNode* stmt) {
    
    if (dynamic_cast<DeclNode*>(stmt)) {
        return;
    }

    // ---- Assignment: x = expr ----
    if (auto assign = dynamic_cast<AssignNode*>(stmt)) {
        std::string result = genExpr(assign->expr);
        emit(assign->name + " = " + result);
        return;
    }

    // ---- if / if-else ----
    if (auto ifNode = dynamic_cast<IfNode*>(stmt)) {
        std::string condResult = genExpr(ifNode->condition);

        if (!ifNode->elseBlock) {
            // if (cond) { then }
            std::string endLabel = newLabel();
            emit("if_false " + condResult + " goto " + endLabel);
            genStmt(ifNode->thenBlock);
            emit(endLabel + ":");
        } else {
            // if (cond) { then } else { else }
            std::string elseLabel = newLabel();
            std::string endLabel  = newLabel();
            emit("if_false " + condResult + " goto " + elseLabel);
            genStmt(ifNode->thenBlock);
            emit("goto " + endLabel);
            emit(elseLabel + ":");
            genStmt(ifNode->elseBlock);
            emit(endLabel + ":");
        }
        return;
    }

    // ---- while ----
    if (auto whileNode = dynamic_cast<WhileNode*>(stmt)) {
        std::string startLabel = newLabel();
        std::string endLabel   = newLabel();

        emit(startLabel + ":");
        std::string condResult = genExpr(whileNode->condition);
        emit("if_false " + condResult + " goto " + endLabel);
        genStmt(whileNode->body);
        emit("goto " + startLabel);
        emit(endLabel + ":");
        return;
    }

    // ---- print ----
    if (auto printNode = dynamic_cast<PrintNode*>(stmt)) {
        std::string result = genExpr(printNode->expr);
        emit("print " + result);
        return;
    }

    
    if (auto block = dynamic_cast<BlockNode*>(stmt)) {
        genStmtList(block->statements);
        return;
    }
}

std::string TACGenerator::genExpr(ASTNode* expr) {
    if (!expr) return "";

    // ---- Identifier: already a named value, no temp needed ----
    if (auto idNode = dynamic_cast<IdNode*>(expr)) {
        return idNode->name;
    }

    // ---- Literals: emit their text form directly ----
    if (auto intLit = dynamic_cast<IntLiteralNode*>(expr)) {
        return std::to_string(intLit->value);
    }
    if (auto floatLit = dynamic_cast<FloatLiteralNode*>(expr)) {
        std::ostringstream oss;
        oss << floatLit->value;
        return oss.str();
    }
    if (auto boolLit = dynamic_cast<BoolLiteralNode*>(expr)) {
        return boolLit->value ? "true" : "false";
    }

    // ---- Unary operation ----
    if (auto unary = dynamic_cast<UnaryOpNode*>(expr)) {
        std::string operand = genExpr(unary->operand);
        std::string temp = newTemp();
        emit(temp + " = " + unary->op + operand);
        return temp;
    }

    // ---- Binary operation ----
    if (auto bin = dynamic_cast<BinaryOpNode*>(expr)) {
        std::string left  = genExpr(bin->left);
        std::string right = genExpr(bin->right);
        std::string temp = newTemp();
        emit(temp + " = " + left + " " + bin->op + " " + right);
        return temp;
    }

    return "";
}