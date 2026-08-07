%code requires {
    #include <vector>
    #include "../ast/ast.h"
    #include "../symbol_table/symbol_table.h"
    #include "../semantic/semantic_analyzer.h"
}

%{
/*
 * parser.y
 * ...
 */
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "../ast/ast.h"
#include "../symbol_table/symbol_table.h"

extern int line_number;          // defined in lexer.l
extern int lexical_error_count;  // defined in lexer.l
int syntax_error_count = 0;

extern int yylex();
void yyerror(const char* msg);

// Root of the AST, filled in by the `program` rule's action.
ASTNode* programRoot = nullptr;
SymbolTable symTab;   // global symbol table, built during parsing
%}

/* ---- Semantic value types carried by tokens/rules ---- */
%union {
    int    ival;
    double fval;
    char*  sval;
    ASTNode* node;
    std::vector<ASTNode*>* nodelist;
}

/* ---- Tokens with a value ---- */
%token <ival> INT_CONST
%token <fval> FLOAT_CONST
%token <sval> ID

/* ---- Keywords ---- */
%token INT FLOAT BOOL IF ELSE WHILE PRINT TRUE FALSE

/* ---- Operators and punctuation ---- */
%token EQ NEQ LE GE AND OR LT GT
%token PLUS MINUS MUL DIV MOD NOT ASSIGN
%token SEMICOLON COMMA LPAREN RPAREN LBRACE RBRACE

/* ---- Non-terminal value types ---- */
%type <node> stmt decl_stmt assign_stmt if_stmt while_stmt print_stmt block expr
%type <nodelist> stmt_list
%type <ival> type

/* ---- Operator precedence, lowest to highest ---- */
%left OR
%left AND
%right NOT
%left EQ NEQ
%left LT GT LE GE
%left PLUS MINUS
%left MUL DIV MOD
%right UMINUS

%%

/* ---- Program structure ---- */
program:
    stmt_list { programRoot = new ProgramNode($1); }
    ;

stmt_list:
    /* empty */                { $$ = new std::vector<ASTNode*>(); }
    | stmt_list stmt           {
                                    if ($2) $1->push_back($2);
                                    $$ = $1;
                                }
    ;

stmt:
    decl_stmt     { $$ = $1; }
    | assign_stmt { $$ = $1; }
    | if_stmt     { $$ = $1; }
    | while_stmt  { $$ = $1; }
    | print_stmt  { $$ = $1; }
    | block       { $$ = $1; }
    | error SEMICOLON { $$ = nullptr; syntax_error_count++; }
    ;

/* ---- Declarations ---- */
type:
    INT   { $$ = static_cast<int>(VarType::TYPE_INT); }
    | FLOAT { $$ = static_cast<int>(VarType::TYPE_FLOAT); }
    | BOOL  { $$ = static_cast<int>(VarType::TYPE_BOOL); }
    ;

decl_stmt:
    type ID SEMICOLON {
        symTab.insert(std::string($2), static_cast<VarType>($1), line_number);
        $$ = new DeclNode(static_cast<VarType>($1), std::string($2), line_number);
        free($2);
    }
    ;

/* ---- Assignment ---- */
assign_stmt:
    ID ASSIGN expr SEMICOLON {
        $$ = new AssignNode(std::string($1), $3, line_number);
        free($1);
    }
    ;

/* ---- Control flow ---- */
if_stmt:
    IF LPAREN expr RPAREN block {
        $$ = new IfNode($3, $5, nullptr);
    }
    | IF LPAREN expr RPAREN block ELSE block {
        $$ = new IfNode($3, $5, $7);
    }
    ;

while_stmt:
    WHILE LPAREN expr RPAREN block {
        $$ = new WhileNode($3, $5);
    }
    ;

/* ---- print ---- */
print_stmt:
    PRINT expr SEMICOLON {
        $$ = new PrintNode($2, line_number);
    }
    ;

/* ---- Nested block with its own scope (Section 5.2) ---- */
block:
    LBRACE { symTab.enterScope(); } stmt_list RBRACE {
        symTab.exitScope();
        $$ = new BlockNode($3);
    }
    ;

/* ---- Expressions ---- */
expr:
    expr OR expr    { $$ = new BinaryOpNode("||", $1, $3); }
    | expr AND expr { $$ = new BinaryOpNode("&&", $1, $3); }
    | NOT expr      { $$ = new UnaryOpNode("!", $2); }
    | expr EQ expr  { $$ = new BinaryOpNode("==", $1, $3); }
    | expr NEQ expr { $$ = new BinaryOpNode("!=", $1, $3); }
    | expr LT expr  { $$ = new BinaryOpNode("<", $1, $3); }
    | expr GT expr  { $$ = new BinaryOpNode(">", $1, $3); }
    | expr LE expr  { $$ = new BinaryOpNode("<=", $1, $3); }
    | expr GE expr  { $$ = new BinaryOpNode(">=", $1, $3); }
    | expr PLUS expr  { $$ = new BinaryOpNode("+", $1, $3); }
    | expr MINUS expr { $$ = new BinaryOpNode("-", $1, $3); }
    | expr MUL expr   { $$ = new BinaryOpNode("*", $1, $3); }
    | expr DIV expr   { $$ = new BinaryOpNode("/", $1, $3); }
    | expr MOD expr   { $$ = new BinaryOpNode("%", $1, $3); }
    | MINUS expr %prec UMINUS { $$ = new UnaryOpNode("-", $2); }
    | LPAREN expr RPAREN { $$ = $2; }
    | ID {
            $$ = new IdNode(std::string($1), line_number);
            free($1);
        }
    | INT_CONST   { $$ = new IntLiteralNode($1); }
    | FLOAT_CONST { $$ = new FloatLiteralNode($1); }
    | TRUE  { $$ = new BoolLiteralNode(true); }
    | FALSE { $$ = new BoolLiteralNode(false); }
    ;

%%

void yyerror(const char* msg) {
    fprintf(stderr, "Syntax Error at line %d: %s\n", line_number, msg);
    syntax_error_count++;
}

int main(int argc, char** argv) {
    extern FILE* yyin;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source-file>\n", argv[0]);
        return 1;
    }

    yyin = fopen(argv[1], "r");
    if (!yyin) {
        fprintf(stderr, "Error: cannot open file '%s'\n", argv[1]);
        return 1;
    }

    line_number = 1;
    yyparse();

    fclose(yyin);

    if (syntax_error_count > 0 || lexical_error_count > 0) {
        fprintf(stderr, "\nCompilation failed: %d syntax error(s), %d lexical error(s)\n",
                syntax_error_count, lexical_error_count);
        return 1;
    }

    printf("Parsing completed successfully - no errors.\n\n");
    printf("=== Abstract Syntax Tree ===\n");
    if (programRoot) {
        programRoot->print(0);
    }

    printf("\n=== Semantic Analysis ===\n");
    SemanticAnalyzer analyzer;
    bool semanticOk = analyzer.analyze(dynamic_cast<ProgramNode*>(programRoot));

    if (!semanticOk) {
        fprintf(stderr, "\nCompilation failed: %d semantic error(s)\n",
                analyzer.errorCount());
        return 1;
    }

    printf("No semantic errors found.\n");
    return 0;
}