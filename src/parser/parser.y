%{
/*
 * parser.y
 * Bison grammar for the Mini Programming Language.
 * Milestone 3: validates syntax only (no AST construction yet,
 * no semantic checks yet). AST building is added in Milestone 4.
 */
#include <stdio.h>
#include <stdlib.h>
#include "../ast/ast.h"

extern int line_number;          // defined in lexer.l
extern int lexical_error_count;  // defined in lexer.l
int syntax_error_count = 0;

extern int yylex();
void yyerror(const char* msg);
%}

/* ---- Semantic value types carried by tokens ---- */
%union {
    int    ival;
    double fval;
    char*  sval;
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

/* ---- Operator precedence, lowest to highest ----
 * Following Section 5.3 of the manual: logical, then relational,
 * then arithmetic, matching standard language conventions.
 */
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
    stmt_list
    ;

stmt_list:
    /* empty */
    | stmt_list stmt
    ;

stmt:
    decl_stmt
    | assign_stmt
    | if_stmt
    | while_stmt
    | print_stmt
    | block
    | error SEMICOLON   { syntax_error_count++; }
    ;

/* ---- Declarations ---- */
type:
    INT
    | FLOAT
    | BOOL
    ;

decl_stmt:
    type ID SEMICOLON
    ;

/* ---- Assignment ---- */
assign_stmt:
    ID ASSIGN expr SEMICOLON
    ;

/* ---- Control flow ---- */
if_stmt:
    IF LPAREN expr RPAREN block
    | IF LPAREN expr RPAREN block ELSE block
    ;

while_stmt:
    WHILE LPAREN expr RPAREN block
    ;

/* ---- print ---- */
print_stmt:
    PRINT expr SEMICOLON
    ;

/* ---- Nested block with its own scope (Section 5.2) ---- */
block:
    LBRACE stmt_list RBRACE
    ;

/* ---- Expressions ---- */
expr:
    expr OR expr
    | expr AND expr
    | NOT expr
    | expr EQ expr
    | expr NEQ expr
    | expr LT expr
    | expr GT expr
    | expr LE expr
    | expr GE expr
    | expr PLUS expr
    | expr MINUS expr
    | expr MUL expr
    | expr DIV expr
    | expr MOD expr
    | MINUS expr %prec UMINUS
    | LPAREN expr RPAREN
    | ID
    | INT_CONST
    | FLOAT_CONST
    | TRUE
    | FALSE
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

    printf("Parsing completed successfully - no errors.\n");
    return 0;
}