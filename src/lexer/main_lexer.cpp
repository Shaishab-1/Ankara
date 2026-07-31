// main_lexer.cpp
// Temporary driver used ONLY to test the lexer in isolation
// during Milestone 2. This file will be removed/replaced once
// the parser (Bison) takes over calling yylex() in Milestone 3.

#include <cstdio>
#include <iostream>
#include "tokens.h"

extern int yylex();
extern Token currentToken;
extern FILE* yyin;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <source-file>\n";
        return 1;
    }

    FILE* file = fopen(argv[1], "r");
    if (!file) {
        std::cerr << "Error: cannot open file '" << argv[1] << "'\n";
        return 1;
    }
    yyin = file;

    bool hasError = false;

    // yylex() returns 1 for every recognized token, 0 at end of file.
    while (yylex()) {
        if (currentToken.type == TokenType::LEX_ERROR) {
            std::cerr << "Lexical Error at line " << currentToken.line
                       << ": unexpected character '" << currentToken.lexeme
                       << "'\n";
            hasError = true;
        } else {
            std::cout << "Line " << currentToken.line << ": "
                      << tokenTypeToString(currentToken.type)
                      << " -> '" << currentToken.lexeme << "'\n";
        }
    }

    fclose(file);
    return hasError ? 1 : 0;
}