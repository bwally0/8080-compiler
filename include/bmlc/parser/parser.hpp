#pragma once

#include <bmlc/lexer/token.hpp>
#include <bmlc/lexer/lexer.hpp>
#include <bmlc/parser/ast.hpp>
#include <memory>

namespace bmlc {

class Parser {
public:
    explicit Parser(const Lexer& lexer, const std::string &filename);
    std::unique_ptr<Program> parse_program();

private:
    Lexer& lexer_;
    Token current_token_;
    std::string filename_;

    void advance();
    void expect(TokenType type);
    SourceLocation current_location() const;

    std::unique_ptr<Declaration> parse_declaration();
    std::unique_ptr<VarDeclaration> parse_var_declaration();
    std::unique_ptr<FuncDeclaration> parse_func_declaration();
    std::vector<Param> parse_param_list();

    std::unique_ptr<Block> parse_block();
    std::unique_ptr<Statement> parse_statement();
    std::unique_ptr<AssignmentStatement> parse_assignment_statement();
    std::unique_ptr<IfStatement> parse_if_statement();
    std::unique_ptr<WhileStatement> parse_while_statement();
    std::unique_ptr<RetStatement> parse_ret_statement();

    std::unique_ptr<Expression> parse_expression();
    std::unique_ptr<Expression> parse_binary_tail(std::unique_ptr<Expression> left);
    std::unique_ptr<Expression> parse_operand();
    std::unique_ptr<Expression> parse_function_call_tail(const std::string& name);
    std::vector<std::unique_ptr<Expression>> parse_argument_list();
};

} // namespace bmlc