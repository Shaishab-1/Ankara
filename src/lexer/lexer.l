%{
/*
 * lexer.l
 * Flex specification for the Mini Programming Language.
 * Milestone 3: now integrated with the Bison parser.
 * Token codes (INT, PLUS, ID, ...) and the yylval union type
 * come from parser.tab.h, which Bison generates from parser.y.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"

int line_number = 1;
int lexical_error_count = 0;
%}

%option noyywrap noinput nounput

DIGIT       [0-9]
LETTER      [a-zA-Z_]
ID          {LETTER}({LETTER}|{DIGIT})*
INT_CONST   {DIGIT}+
FLOAT_CONST {DIGIT}+"."{DIGIT}+

%%

"//".*                     { /* single-line comment: discard */ }
"/*"([^*]|\*+[^*/])*\*+"/" { /* block comment: discard */ }

"int"     { return INT; }
"float"   { return FLOAT; }
"bool"    { return BOOL; }
"if"      { return IF; }
"else"    { return ELSE; }
"while"   { return WHILE; }
"print"   { return PRINT; }
"true"    { return TRUE; }
"false"   { return FALSE; }

"=="      { return EQ; }
"!="      { return NEQ; }
"<="      { return LE; }
">="      { return GE; }
"&&"      { return AND; }
"||"      { return OR; }
"<"       { return LT; }
">"       { return GT; }

"+"       { return PLUS; }
"-"       { return MINUS; }
"*"       { return MUL; }
"/"       { return DIV; }
"%"       { return MOD; }
"!"       { return NOT; }
"="       { return ASSIGN; }

";"       { return SEMICOLON; }
","       { return COMMA; }
"("       { return LPAREN; }
")"       { return RPAREN; }
"{"       { return LBRACE; }
"}"       { return RBRACE; }

{FLOAT_CONST} {
                yylval.fval = atof(yytext);
                return FLOAT_CONST;
            }
{INT_CONST} {
                yylval.ival = atoi(yytext);
                return INT_CONST;
            }
{ID}        {
                yylval.sval = strdup(yytext);
                return ID;
            }

[ \t\r]+  { /* whitespace: discard */ }
\n        { line_number++; }

.         {
                fprintf(stderr,
                        "Lexical Error at line %d: Invalid character '%c'\n",
                        line_number,
                        yytext[0]);
                lexical_error_count++;
          }

%%