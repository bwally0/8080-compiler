#include <string>

#include <bmlc/lexer/token.hpp>
#include <bmlc/lexer/lexer.hpp>
#include <bmlc/parser/ast.hpp>
#include <bmlc/parser/parser.hpp>

namespace bmlc {

Parser::Parser(const Lexer& lexer, const std::string &filename)
    : lexer_(const_cast<Lexer&>(lexer)), filename_(filename) {
    advance(); // load first token
}

void Parser::advance() {
    current_token_ = lexer_.next_token();
}

void Parser::expect(TokenType type) {
    if (current_token_.type != type) {
        throw std::runtime_error(filename_ + ":" + std::to_string(current_token_.line) + ":" +
            std::to_string(current_token_.column) + ": error: expected token " +
            token_type_to_string(type) + ", got " +
            token_type_to_string(current_token_.type));
    }
    advance();
}

SourceLocation Parser::current_location() const {
    return SourceLocation{filename_, current_token_.line, current_token_.column};
}

std::unique_ptr<Program> Parser::parse_program() {
    auto program = std::make_unique<Program>();
    program->location = current_location(); 
    std::vector<std::unique_ptr<Declaration>> declarations;


    while (current_token_.type != TokenType::END_OF_FILE) {
        declarations.push_back(parse_declaration());
    }

    program->declarations = std::move(declarations);

    return program;
}

std::unique_ptr<Declaration> Parser::parse_declaration() {
    if (current_token_.type == TokenType::KW_VAR) {
        return parse_var_declaration();
    } else if (current_token_.type == TokenType::KW_FUNC) {
        return parse_func_declaration();
    } else {
        throw std::runtime_error(filename_ + ":" + std::to_string(current_token_.line) + ":" +
            std::to_string(current_token_.column) + ": error: expected declaration");
    }
}

std::unique_ptr<VarDeclaration> Parser::parse_var_declaration() {
    auto var_decl = std::make_unique<VarDeclaration>();
    var_decl->location = current_location();

    expect(TokenType::KW_VAR);

    // parse type
    if (current_token_.type == TokenType::KW_UINT8) {
        var_decl->type = Type::UINT8;
        advance();
    } else if (current_token_.type == TokenType::KW_UINT16) {
        var_decl->type = Type::UINT16;
        advance();
    } else if (current_token_.type == TokenType::KW_INT8) {
        var_decl->type = Type::INT8;
        advance();
    } else if (current_token_.type == TokenType::KW_INT16) {
        var_decl->type = Type::INT16;
        advance();
    } else {
        throw std::runtime_error(filename_ + ":" + std::to_string(current_token_.line) + ":" +
            std::to_string(current_token_.column) + ": error: expected type");
    }

    // parse name
    if (current_token_.type != TokenType::IDENTIFIER) {
        throw std::runtime_error(filename_ + ":" + std::to_string(current_token_.line) + ":" +
            std::to_string(current_token_.column) + ": error: expected identifier");
    }
    var_decl->name = current_token_.lexeme;
    advance();

    expect(TokenType::SEMICOLON);

    return var_decl;
}

std::unique_ptr<FuncDeclaration> Parser::parse_func_declaration() {
    auto func_decl = std::make_unique<FuncDeclaration>();

    return func_decl;
}

std::vector<Param> Parser::parse_param_list() {
    std::vector<Param> params;

    return params;
}

std::unique_ptr<Block> Parser::parse_block() {
    auto block = std::make_unique<Block>();

    return block;
}

std::unique_ptr<Statement> Parser::parse_statement() {
    return nullptr;
}

std::unique_ptr<AssignmentStatement> Parser::parse_assignment_statement() {
    return nullptr;
}

std::unique_ptr<IfStatement> Parser::parse_if_statement() {
    return nullptr;
}

std::unique_ptr<WhileStatement> Parser::parse_while_statement() {
    return nullptr;
}

std::unique_ptr<RetStatement> Parser::parse_ret_statement() {
    return nullptr;
}

std::unique_ptr<Expression> Parser::parse_expression() {
    return nullptr;
}

} // namespace bmlc