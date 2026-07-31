#include "tokens.h"

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::KW_INT:        return "KW_INT";
        case TokenType::KW_FLOAT:      return "KW_FLOAT";
        case TokenType::KW_BOOL:       return "KW_BOOL";
        case TokenType::KW_IF:         return "KW_IF";
        case TokenType::KW_ELSE:       return "KW_ELSE";
        case TokenType::KW_WHILE:      return "KW_WHILE";
        case TokenType::KW_PRINT:      return "KW_PRINT";
        case TokenType::KW_TRUE:       return "KW_TRUE";
        case TokenType::KW_FALSE:      return "KW_FALSE";
        case TokenType::IDENTIFIER:    return "IDENTIFIER";
        case TokenType::INT_LITERAL:   return "INT_LITERAL";
        case TokenType::FLOAT_LITERAL: return "FLOAT_LITERAL";
        case TokenType::PLUS:          return "PLUS";
        case TokenType::MINUS:         return "MINUS";
        case TokenType::STAR:          return "STAR";
        case TokenType::SLASH:         return "SLASH";
        case TokenType::PERCENT:       return "PERCENT";
        case TokenType::LT:            return "LT";
        case TokenType::GT:            return "GT";
        case TokenType::LE:            return "LE";
        case TokenType::GE:            return "GE";
        case TokenType::EQ:            return "EQ";
        case TokenType::NE:            return "NE";
        case TokenType::AND:           return "AND";
        case TokenType::OR:            return "OR";
        case TokenType::NOT:           return "NOT";
        case TokenType::ASSIGN:        return "ASSIGN";
        case TokenType::LBRACE:        return "LBRACE";
        case TokenType::RBRACE:        return "RBRACE";
        case TokenType::LPAREN:        return "LPAREN";
        case TokenType::RPAREN:        return "RPAREN";
        case TokenType::SEMICOLON:     return "SEMICOLON";
        case TokenType::LEX_ERROR:     return "LEX_ERROR";
        default:                       return "UNKNOWN";
    }
}