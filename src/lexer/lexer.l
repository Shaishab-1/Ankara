%{
/*
 * lexer.l
 * Flex specification for the Mini Programming Language.
 * Milestone 2: standalone lexer, not yet wired to Bison.
 * Each rule stores a full Token (type + lexeme + line) into the
 * global `currentToken`, then returns 1 so the test driver knows
 * "a token was produced". Flex automatically returns 0 at EOF.
 */
#include "tokens.h"

int currentLine = 1;
Token currentToken;

static Token makeToken(TokenType type) {
    Token t;
    t.type   = type;
    t.lexeme = yytext;
    t.line   = currentLine;
    return t;
}
%}

%option noyywrap noinput nounput

DIGIT    [0-9]
LETTER   [a-zA-Z_]
ID       {LETTER}({LETTER}|{DIGIT})*
INTLIT   {DIGIT}+
FLOATLIT {DIGIT}+"."{DIGIT}+

%%

"//".*                        { /* single-line comment: discard */ }
"/*"([^*]|\*+[^*/])*\*+"/"     { /* block comment: discard */ }

"int"     { currentToken = makeToken(TokenType::KW_INT);   return 1; }
"float"   { currentToken = makeToken(TokenType::KW_FLOAT); return 1; }
"bool"    { currentToken = makeToken(TokenType::KW_BOOL);  return 1; }
"if"      { currentToken = makeToken(TokenType::KW_IF);    return 1; }
"else"    { currentToken = makeToken(TokenType::KW_ELSE);  return 1; }
"while"   { currentToken = makeToken(TokenType::KW_WHILE); return 1; }
"print"   { currentToken = makeToken(TokenType::KW_PRINT); return 1; }
"true"    { currentToken = makeToken(TokenType::KW_TRUE);  return 1; }
"false"   { currentToken = makeToken(TokenType::KW_FALSE); return 1; }

{FLOATLIT} { currentToken = makeToken(TokenType::FLOAT_LITERAL); return 1; }
{INTLIT}   { currentToken = makeToken(TokenType::INT_LITERAL);   return 1; }
{ID}       { currentToken = makeToken(TokenType::IDENTIFIER);    return 1; }

"<="      { currentToken = makeToken(TokenType::LE); return 1; }
">="      { currentToken = makeToken(TokenType::GE); return 1; }
"=="      { currentToken = makeToken(TokenType::EQ); return 1; }
"!="      { currentToken = makeToken(TokenType::NE); return 1; }
"&&"      { currentToken = makeToken(TokenType::AND); return 1; }
"||"      { currentToken = makeToken(TokenType::OR);  return 1; }

"<"       { currentToken = makeToken(TokenType::LT);     return 1; }
">"       { currentToken = makeToken(TokenType::GT);     return 1; }
"!"       { currentToken = makeToken(TokenType::NOT);    return 1; }
"="       { currentToken = makeToken(TokenType::ASSIGN); return 1; }
"+"       { currentToken = makeToken(TokenType::PLUS);   return 1; }
"-"       { currentToken = makeToken(TokenType::MINUS);  return 1; }
"*"       { currentToken = makeToken(TokenType::STAR);   return 1; }
"/"       { currentToken = makeToken(TokenType::SLASH);  return 1; }
"%"       { currentToken = makeToken(TokenType::PERCENT); return 1; }

"{"       { currentToken = makeToken(TokenType::LBRACE);    return 1; }
"}"       { currentToken = makeToken(TokenType::RBRACE);    return 1; }
"("       { currentToken = makeToken(TokenType::LPAREN);    return 1; }
")"       { currentToken = makeToken(TokenType::RPAREN);    return 1; }
";"       { currentToken = makeToken(TokenType::SEMICOLON); return 1; }

\n        { currentLine++; }
[ \t\r]+  { /* whitespace: discard */ }

.         { currentToken = makeToken(TokenType::LEX_ERROR); return 1; }

%%