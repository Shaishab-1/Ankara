#ifndef AST_H
#define AST_H


#include <iostream>
#include <string>
#include <vector>


enum class VarType { TYPE_INT, TYPE_FLOAT, TYPE_BOOL };

inline std::string varTypeToString(VarType t) {
    switch (t) {
        case VarType::TYPE_INT:   return "int";
        case VarType::TYPE_FLOAT: return "float";
        case VarType::TYPE_BOOL:  return "bool";
    }
    return "unknown";
}

// ---- Base class ----
class ASTNode {
public:
    virtual ~ASTNode() = default;

    // Prints this node and all its children, indented to show
    // tree depth. `indent` is the current nesting level (not
    // characters — each level adds 2 spaces).
    virtual void print(int indent) const = 0;

protected:
    // Shared helper so every node formats indentation identically.
    static void printIndent(int indent) {
        for (int i = 0; i < indent; ++i) std::cout << "  ";
    }
};

// ---- Program (root node) ----
class ProgramNode : public ASTNode {
public:
    std::vector<ASTNode*>* statements;

    explicit ProgramNode(std::vector<ASTNode*>* stmts) : statements(stmts) {}

    void print(int indent) const override {
        printIndent(indent);
        std::cout << "Program\n";
        for (ASTNode* stmt : *statements) {
            if (stmt) stmt->print(indent + 1);
        }
    }
};

// ---- Block ({ ... }) ----
class BlockNode : public ASTNode {
public:
    std::vector<ASTNode*>* statements;

    explicit BlockNode(std::vector<ASTNode*>* stmts) : statements(stmts) {}

    void print(int indent) const override {
        printIndent(indent);
        std::cout << "Block\n";
        for (ASTNode* stmt : *statements) {
            if (stmt) stmt->print(indent + 1);
        }
    }
};

// ---- Declaration: e.g. int x; ----
class DeclNode : public ASTNode {
public:
    VarType type;
    std::string name;
    int line;

    DeclNode(VarType t, std::string n, int ln) : type(t), name(std::move(n)), line(ln) {}

    void print(int indent) const override {
        printIndent(indent);
        std::cout << "Decl(" << varTypeToString(type) << " " << name
                  << ") [line " << line << "]\n";
    }
};

// ---- Assignment: e.g. x = expr; ----
class AssignNode : public ASTNode {
public:
    std::string name;
    ASTNode* expr;
    int line;

    AssignNode(std::string n, ASTNode* e, int ln) : name(std::move(n)), expr(e), line(ln) {}

    void print(int indent) const override {
        printIndent(indent);
        std::cout << "Assign(" << name << ") [line " << line << "]\n";
        if (expr) expr->print(indent + 1);
    }
};

// ---- if / if-else ----
class IfNode : public ASTNode {
public:
    ASTNode* condition;
    ASTNode* thenBlock;
    ASTNode* elseBlock; // nullptr if there is no else

    IfNode(ASTNode* cond, ASTNode* thenB, ASTNode* elseB)
        : condition(cond), thenBlock(thenB), elseBlock(elseB) {}

    void print(int indent) const override {
        printIndent(indent);
        std::cout << "If\n";
        printIndent(indent + 1);
        std::cout << "Condition:\n";
        if (condition) condition->print(indent + 2);
        printIndent(indent + 1);
        std::cout << "Then:\n";
        if (thenBlock) thenBlock->print(indent + 2);
        if (elseBlock) {
            printIndent(indent + 1);
            std::cout << "Else:\n";
            elseBlock->print(indent + 2);
        }
    }
};

// ---- while ----
class WhileNode : public ASTNode {
public:
    ASTNode* condition;
    ASTNode* body;

    WhileNode(ASTNode* cond, ASTNode* b) : condition(cond), body(b) {}

    void print(int indent) const override {
        printIndent(indent);
        std::cout << "While\n";
        printIndent(indent + 1);
        std::cout << "Condition:\n";
        if (condition) condition->print(indent + 2);
        printIndent(indent + 1);
        std::cout << "Body:\n";
        if (body) body->print(indent + 2);
    }
};

// ---- print statement ----
class PrintNode : public ASTNode {
public:
    ASTNode* expr;
    int line;

    PrintNode(ASTNode* e, int ln) : expr(e), line(ln) {}

    void print(int indent) const override {
        printIndent(indent);
        std::cout << "Print [line " << line << "]\n";
        if (expr) expr->print(indent + 1);
    }
};

// ---- Binary operation: +, -, *, /, %, <, >, ==, &&, || etc. ----
class BinaryOpNode : public ASTNode {
public:
    std::string op;
    ASTNode* left;
    ASTNode* right;

    BinaryOpNode(std::string o, ASTNode* l, ASTNode* r)
        : op(std::move(o)), left(l), right(r) {}

    void print(int indent) const override {
        printIndent(indent);
        std::cout << "BinaryOp(" << op << ")\n";
        if (left) left->print(indent + 1);
        if (right) right->print(indent + 1);
    }
};

// ---- Unary operation: - (negation), ! (logical not) ----
class UnaryOpNode : public ASTNode {
public:
    std::string op;
    ASTNode* operand;

    UnaryOpNode(std::string o, ASTNode* operand_) : op(std::move(o)), operand(operand_) {}

    void print(int indent) const override {
        printIndent(indent);
        std::cout << "UnaryOp(" << op << ")\n";
        if (operand) operand->print(indent + 1);
    }
};

// ---- Identifier reference (variable use) ----
class IdNode : public ASTNode {
public:
    std::string name;
    int line;

    IdNode(std::string n, int ln) : name(std::move(n)), line(ln) {}

    void print(int indent) const override {
        printIndent(indent);
        std::cout << "Id(" << name << ") [line " << line << "]\n";
    }
};

// ---- Literals ----
class IntLiteralNode : public ASTNode {
public:
    int value;
    explicit IntLiteralNode(int v) : value(v) {}

    void print(int indent) const override {
        printIndent(indent);
        std::cout << "IntLiteral(" << value << ")\n";
    }
};

class FloatLiteralNode : public ASTNode {
public:
    double value;
    explicit FloatLiteralNode(double v) : value(v) {}

    void print(int indent) const override {
        printIndent(indent);
        std::cout << "FloatLiteral(" << value << ")\n";
    }
};

class BoolLiteralNode : public ASTNode {
public:
    bool value;
    explicit BoolLiteralNode(bool v) : value(v) {}

    void print(int indent) const override {
        printIndent(indent);
        std::cout << "BoolLiteral(" << (value ? "true" : "false") << ")\n";
    }
};

#endif // AST_H