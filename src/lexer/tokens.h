#ifndef TOKENS_H
#define TOKENS_H

#include <string>

enum class TokenType {
    // Keywords
    KW_INT, KW_FLOAT, KW_BOOL, KW_IF, KW_ELSE, KW_WHILE, KW_PRINT,
    KW_TRUE, KW_FALSE,

    // Identifiers and literals
    IDENTIFIER, INT_LITERAL, FLOAT_LITERAL,

    // Arithmetic operators
    PLUS, MINUS, STAR, SLASH, PERCENT,

    // Relational operators
    LT, GT, LE, GE, EQ, NE,

    // Logical operators
    AND, OR, NOT,

    // Assignment
    ASSIGN,

    // Delimiters
    LBRACE, RBRACE, LPAREN, RPAREN, SEMICOLON,

    // Special
    LEX_ERROR
};


struct Token {
    TokenType type;
    std::string lexeme;
    int line;
};


std::string tokenTypeToString(TokenType type);

#endif 