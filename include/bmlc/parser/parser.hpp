#pragma once

#include <bmlc/lexer/token.hpp>
#include <bmlc/lexer/lexer.hpp>
#include <bmlc/parser/ast.hpp>
#include <memory>

namespace bmlc {

class Parser {
public:
    explicit Parser(const Lexer& lexer);
    std::unique_ptr<Program> parseProgram();

private:
    Lexer& lexer_;
    Token current_token_;
};

} // namespace bmlc