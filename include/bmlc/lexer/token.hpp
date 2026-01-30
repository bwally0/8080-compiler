#pragma once

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

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
};

} // namespace bmlc