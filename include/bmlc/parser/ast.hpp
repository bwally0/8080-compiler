#pragma once

#include <memory>
#include <vector>
#include <string>

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

enum class BinaryOperator {
    PLUS,
    MINUS,
    LESS,
    AND,
    OR,
    XOR
};

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

} // namespace bmlc