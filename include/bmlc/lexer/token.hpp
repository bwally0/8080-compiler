#pragma once

#include<iomanip>
#include <string>

namespace bmlc {

enum class TokenType {
    IDENTIFIER,   // variable and function names
    NUMBER,       // numeric literals

    // keywords
    KW_VAR,       // variable declaration
    KW_FUNC,      // function declaration
    KW_IF,        // conditional statement
    KW_ELSE,      // else statement
    KW_WHILE,     // while loop
    KW_RETURN,    // return statement

    // data types
    KW_UINT8,
    KW_UINT16,
    KW_INT8,
    KW_INT16,
    KW_VOID,

    ASSIGN,       // =
    PLUS,         // +
    MINUS,        // -
    LESS,         // <
    AND,          // &
    OR,           // |
    XOR,          // ^

    L_PAREN,      // (
    R_PAREN,      // )
    L_BRACE,      // {
    R_BRACE,      // }
    COMMA,        // ,
    SEMICOLON,    // ;

    ARROW,        // ->
    END_OF_FILE   // end of file
};

inline std::string token_type_to_string(TokenType type) {
    switch (type) {
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::KW_VAR: return "KW_VAR";
        case TokenType::KW_FUNC: return "KW_FUNC";
        case TokenType::KW_IF: return "KW_IF";
        case TokenType::KW_ELSE: return "KW_ELSE";
        case TokenType::KW_WHILE: return "KW_WHILE";
        case TokenType::KW_RETURN: return "KW_RETURN";
        case TokenType::KW_UINT8: return "KW_UINT8";
        case TokenType::KW_UINT16: return "KW_UINT16";
        case TokenType::KW_INT8: return "KW_INT8";
        case TokenType::KW_INT16: return "KW_INT16";
        case TokenType::KW_VOID: return "KW_VOID";
        case TokenType::ASSIGN: return "ASSIGN";
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::LESS: return "LESS";
        case TokenType::AND: return "AND";
        case TokenType::OR: return "OR";
        case TokenType::XOR: return "XOR";
        case TokenType::L_PAREN: return "L_PAREN";
        case TokenType::R_PAREN: return "R_PAREN";
        case TokenType::L_BRACE: return "L_BRACE";
        case TokenType::R_BRACE: return "R_BRACE";
        case TokenType::COMMA: return "COMMA";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::ARROW: return "ARROW";
        case TokenType::END_OF_FILE: return "END_OF_FILE";
        default: return "UNKNOWN";
    }
}

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
};

inline std::ostream &operator<<(std::ostream &os, const Token &token) {
    std::string location = std::to_string(token.line) + ":" + std::to_string(token.column);
    os << std::left;
    os << std::setw(10) << location;
    os << std::setw(15) << token_type_to_string(token.type);
    os << "'" + token.lexeme + "'";
    return os;
}

} // namespace bmlc