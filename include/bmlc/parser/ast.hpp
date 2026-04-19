#pragma once

#include <memory>
#include <vector>
#include <string>
#include <bmlc/lexer/token.hpp>

namespace bmlc {

struct Declaration;
struct Statement;
struct Expression;
struct Block;

struct SourceLocation {
    std::string filename;
    int line;
    int column;
};

enum class Type {
    UINT8,
    UINT16,
    INT8,
    INT16
};

inline std::string type_to_string(Type type) {
    switch (type) {
        case Type::UINT8: return "uint8";
        case Type::UINT16: return "uint16";
        case Type::INT8: return "int8";
        case Type::INT16: return "int16";
        default: return "unknown";
    }
}

enum class BinaryOperator {
    PLUS,
    MINUS,
    LESS,
    AND,
    OR,
    XOR
};

inline std::string binary_operator_to_string(BinaryOperator op) {
    switch (op) {
        case BinaryOperator::PLUS: return "+";
        case BinaryOperator::MINUS: return "-";
        case BinaryOperator::LESS: return "<";
        case BinaryOperator::AND: return "&";
        case BinaryOperator::OR: return "|";
        case BinaryOperator::XOR: return "^";
        default: return "unknown";
    }
}

inline BinaryOperator token_to_binary_operator(TokenType type) {
    switch (type) {
        case TokenType::PLUS: return BinaryOperator::PLUS;
        case TokenType::MINUS: return BinaryOperator::MINUS;
        case TokenType::LESS: return BinaryOperator::LESS;
        case TokenType::AND: return BinaryOperator::AND;
        case TokenType::OR: return BinaryOperator::OR;
        case TokenType::XOR: return BinaryOperator::XOR;
        default: throw std::runtime_error("error: invalid binary operator token: " + token_type_to_string(type));
    }
}

struct ASTNode {
    virtual ~ASTNode() = default;
    SourceLocation location;
};

// Program

struct Program : ASTNode {
    std::vector<std::unique_ptr<Declaration>> declarations;
};

// Declarations

struct Declaration : ASTNode {
    virtual ~Declaration() = default;
};

struct VarDeclaration : Declaration {
    Type type;
    std::string name;
};

struct Param {
    Type type;
    std::string name;
};

struct FuncDeclaration : Declaration {
    Type return_type;
    std::string name;
    std::vector<Param> parameters;
    std::unique_ptr<Block> body;
};

// Statements

struct Statement : ASTNode {
    virtual ~Statement() = default;
};

struct AssignmentStatement : Statement {
    std::string name;
    std::unique_ptr<Expression> value;
};

struct IfStatement : Statement {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Block> body;
};

struct WhileStatement : Statement {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Block> body;
};

struct RetStatement : Statement {
    std::unique_ptr<Expression> value;
};

struct ExpressionStatement : Statement {
    std::unique_ptr<Expression> expression;
};

struct Block : ASTNode {
    std::vector<std::unique_ptr<Statement>> statements;
};

// Expressions

struct Expression : ASTNode {
    virtual ~Expression() = default;
};

struct BinaryExpression : Expression {
    BinaryOperator op;
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
};

struct VariableExpression : Expression {
    std::string name;
};

struct NumberLiteral : Expression {
    int value;
};

struct FuncCallExpression : Expression {
    std::string name;
    std::vector<std::unique_ptr<Expression>> arguments;
};

// AST Printing
void print_ast(const Program& program);
void print_declaration(const Declaration& decl, int indent = 0);
void print_statement(const Statement& stmt, int indent = 0);
void print_expression(const Expression& expr, int indent = 0);

} // namespace bmlc