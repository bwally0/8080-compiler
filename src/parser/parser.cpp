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
    func_decl->location = current_location();

    expect(TokenType::KW_FUNC);

    // parse function name
    if (current_token_.type != TokenType::IDENTIFIER) {
        throw std::runtime_error(filename_ + ":" + std::to_string(current_token_.line) + ":" +
            std::to_string(current_token_.column) + ": error: expected identifier");
    }
    func_decl->name = current_token_.lexeme;
    advance();

    expect(TokenType::L_PAREN);
    func_decl->parameters = parse_param_list();
    expect(TokenType::R_PAREN);

    expect(TokenType::ARROW);

    // parse return type
    if (current_token_.type == TokenType::KW_UINT8) {
        func_decl->return_type = Type::UINT8;
        advance();
    } else if (current_token_.type == TokenType::KW_UINT16) {
        func_decl->return_type = Type::UINT16;
        advance();
    } else if (current_token_.type == TokenType::KW_INT8) {
        func_decl->return_type = Type::INT8;
        advance();
    } else if (current_token_.type == TokenType::KW_INT16) {
        func_decl->return_type = Type::INT16;
        advance();
    } else {
        throw std::runtime_error(filename_ + ":" + std::to_string(current_token_.line) + ":" +
            std::to_string(current_token_.column) + ": error: expected return type");
    }

    func_decl->body = parse_block();

    return func_decl;
}

std::vector<Param> Parser::parse_param_list() {
    std::vector<Param> params;

    // Check if there are any parameters
    if (current_token_.type == TokenType::R_PAREN) {
        return params;
    }

    // Parse first parameter
    Param param;
    if (current_token_.type == TokenType::KW_UINT8) {
        param.type = Type::UINT8;
        advance();
    } else if (current_token_.type == TokenType::KW_UINT16) {
        param.type = Type::UINT16;
        advance();
    } else if (current_token_.type == TokenType::KW_INT8) {
        param.type = Type::INT8;
        advance();
    } else if (current_token_.type == TokenType::KW_INT16) {
        param.type = Type::INT16;
        advance();
    } else {
        throw std::runtime_error(filename_ + ":" + std::to_string(current_token_.line) + ":" +
            std::to_string(current_token_.column) + ": error: expected type in parameter list");
    }

    if (current_token_.type != TokenType::IDENTIFIER) {
        throw std::runtime_error(filename_ + ":" + std::to_string(current_token_.line) + ":" +
            std::to_string(current_token_.column) + ": error: expected identifier in parameter");
    }
    param.name = current_token_.lexeme;
    advance();

    params.push_back(param);

    // Parse remaining parameters
    while (current_token_.type == TokenType::COMMA) {
        advance(); // consume comma

        Param next_param;
        if (current_token_.type == TokenType::KW_UINT8) {
            next_param.type = Type::UINT8;
            advance();
        } else if (current_token_.type == TokenType::KW_UINT16) {
            next_param.type = Type::UINT16;
            advance();
        } else if (current_token_.type == TokenType::KW_INT8) {
            next_param.type = Type::INT8;
            advance();
        } else if (current_token_.type == TokenType::KW_INT16) {
            next_param.type = Type::INT16;
            advance();
        } else {
            throw std::runtime_error(filename_ + ":" + std::to_string(current_token_.line) + ":" +
                std::to_string(current_token_.column) + ": error: expected type in parameter list");
        }

        if (current_token_.type != TokenType::IDENTIFIER) {
            throw std::runtime_error(filename_ + ":" + std::to_string(current_token_.line) + ":" +
                std::to_string(current_token_.column) + ": error: expected identifier in parameter");
        }
        next_param.name = current_token_.lexeme;
        advance();

        params.push_back(next_param);
    }

    return params;
}

std::unique_ptr<Block> Parser::parse_block() {
    auto block = std::make_unique<Block>();
    block->location = current_location();

    expect(TokenType::L_BRACE);

    while (current_token_.type != TokenType::R_BRACE && current_token_.type != TokenType::END_OF_FILE) {
        auto statement = parse_statement();
        if (statement) {
            block->statements.push_back(std::move(statement));
        }
    }

    expect(TokenType::R_BRACE);

    return block;
}

std::unique_ptr<Statement> Parser::parse_statement() {
    if (current_token_.type == TokenType::IDENTIFIER) {
        return parse_assignment_statement();
    } else if (current_token_.type == TokenType::KW_IF) {
        return parse_if_statement();
    } else if (current_token_.type == TokenType::KW_WHILE) {
        return parse_while_statement();
    } else if (current_token_.type == TokenType::KW_RET) {
        return parse_ret_statement();
    } else {
        throw std::runtime_error(filename_ + ":" + std::to_string(current_token_.line) + ":" +
            std::to_string(current_token_.column) + ": error: expected statement");
    }
}

std::unique_ptr<AssignmentStatement> Parser::parse_assignment_statement() {
    auto assignment_statement = std::make_unique<AssignmentStatement>();
    assignment_statement->location = current_location();
    assignment_statement->name = current_token_.lexeme;

    expect(TokenType::IDENTIFIER);
    expect(TokenType::ASSIGN);

    assignment_statement->value = parse_expression();

    expect(TokenType::SEMICOLON);
    return assignment_statement;
}

std::unique_ptr<IfStatement> Parser::parse_if_statement() {
    auto if_statement = std::make_unique<IfStatement>();
    if_statement->location = current_location();

    expect(TokenType::KW_IF);
    expect(TokenType::L_PAREN);

    if_statement->condition = parse_expression();

    expect(TokenType::R_PAREN);

    if_statement->body = parse_block();

    return if_statement;
}

std::unique_ptr<WhileStatement> Parser::parse_while_statement() {
    auto while_statement = std::make_unique<WhileStatement>();
    while_statement->location = current_location();

    expect(TokenType::KW_WHILE);
    expect(TokenType::L_PAREN);

    while_statement->condition = parse_expression();

    expect(TokenType::R_PAREN);

    while_statement->body = parse_block();

    return while_statement;
}

std::unique_ptr<RetStatement> Parser::parse_ret_statement() {
    auto ret_statement = std::make_unique<RetStatement>();
    ret_statement->location = current_location();

    expect(TokenType::KW_RET);

    ret_statement->value = parse_expression();

    expect(TokenType::SEMICOLON);

    return ret_statement;
}

std::unique_ptr<Expression> Parser::parse_expression() {
    auto operand = parse_operand();
    return parse_binary_tail(std::move(operand));
}

std::unique_ptr<Expression> Parser::parse_binary_tail(std::unique_ptr<Expression> left) {
    if (!is_binary_operator(current_token_.type)) {
        return left;
    }

    BinaryOperator op = token_to_binary_operator(current_token_.type);
    advance(); // consume operator

    auto right = parse_operand();
    auto binary_expression = std::make_unique<BinaryExpression>();

    binary_expression->location = current_location();
    binary_expression->op = op;
    binary_expression->left = std::move(left);
    binary_expression->right = std::move(right);

    return parse_binary_tail(std::move(binary_expression));
}

std::unique_ptr<Expression> Parser::parse_operand() {
    if (current_token_.type == TokenType::NUMBER) {
        auto literal = std::make_unique<NumberLiteral>();
        literal->value = std::stoi(current_token_.lexeme);
        advance();
        return literal;
    } else if (current_token_.type == TokenType::IDENTIFIER) {
        auto name = current_token_.lexeme;
        advance();
        return parse_function_call_tail(name);
    }
    throw std::runtime_error("expected operand");
}

std::unique_ptr<Expression> Parser::parse_function_call_tail(const std::string& name) {
    if (current_token_.type != TokenType::L_PAREN) {
        // variable reference, not a function call
        auto variable = std::make_unique<VariableExpression>();
        variable->name = name;
        return variable;
    }

    expect(TokenType::L_PAREN);
    auto args = parse_argument_list();
    expect(TokenType::R_PAREN);


    auto function_call = std::make_unique<FuncCallExpression>();
    function_call->name = name;
    function_call->arguments = std::move(args);
    return function_call;
}

std::vector<std::unique_ptr<Expression>> Parser::parse_argument_list() {
    std::vector<std::unique_ptr<Expression>> args;

    if (current_token_.type == TokenType::R_PAREN) {
        return args; // empty argument list
    }

    args.push_back(parse_expression());

    while (current_token_.type == TokenType::COMMA) {
        advance(); // consume comma
        args.push_back(parse_expression());
    }

    return args;
}

} // namespace bmlc