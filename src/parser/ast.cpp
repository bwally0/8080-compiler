#include <bmlc/parser/ast.hpp>
#include <iostream>
#include <string>

namespace bmlc {

static std::string indent_str(int indent) {
    return std::string(indent * 2, ' ');
}

void print_ast(const Program& program) {
    std::cout << "Program\n";
    for (const auto& decl : program.declarations) {
        print_declaration(*decl, 1);
    }
}

void print_declaration(const Declaration& decl, int indent) {
    if (auto var_decl = dynamic_cast<const VarDeclaration*>(&decl)) {
        std::cout << indent_str(indent) << "VarDeclaration\n";
        std::cout << indent_str(indent + 1) << "type: " << type_to_string(var_decl->type) << "\n";
        std::cout << indent_str(indent + 1) << "name: " << var_decl->name << "\n";
    } else if (auto func_decl = dynamic_cast<const FuncDeclaration*>(&decl)) {
        std::cout << indent_str(indent) << "FuncDeclaration\n";
        std::cout << indent_str(indent + 1) << "name: " << func_decl->name << "\n";
        std::cout << indent_str(indent + 1) << "return_type: " << type_to_string(func_decl->return_type) << "\n";
        
        if (!func_decl->parameters.empty()) {
            std::cout << indent_str(indent + 1) << "parameters:\n";
            for (const auto& param : func_decl->parameters) {
                std::cout << indent_str(indent + 2) << param.name << ": " << type_to_string(param.type) << "\n";
            }
        }
        
        std::cout << indent_str(indent + 1) << "body:\n";
        std::cout << indent_str(indent + 2) << "Block\n";
        for (const auto& stmt : func_decl->body->statements) {
            print_statement(*stmt, indent + 3);
        }
    }
}

void print_statement(const Statement& stmt, int indent) {
    if (auto assign = dynamic_cast<const AssignmentStatement*>(&stmt)) {
        std::cout << indent_str(indent) << "AssignmentStatement\n";
        std::cout << indent_str(indent + 1) << "name: " << assign->name << "\n";
        std::cout << indent_str(indent + 1) << "value:\n";
        print_expression(*assign->value, indent + 2);
    } else if (auto if_stmt = dynamic_cast<const IfStatement*>(&stmt)) {
        std::cout << indent_str(indent) << "IfStatement\n";
        std::cout << indent_str(indent + 1) << "condition:\n";
        print_expression(*if_stmt->condition, indent + 2);
        std::cout << indent_str(indent + 1) << "body:\n";
        std::cout << indent_str(indent + 2) << "Block\n";
        for (const auto& s : if_stmt->body->statements) {
            print_statement(*s, indent + 3);
        }
    } else if (auto while_stmt = dynamic_cast<const WhileStatement*>(&stmt)) {
        std::cout << indent_str(indent) << "WhileStatement\n";
        std::cout << indent_str(indent + 1) << "condition:\n";
        print_expression(*while_stmt->condition, indent + 2);
        std::cout << indent_str(indent + 1) << "body:\n";
        std::cout << indent_str(indent + 2) << "Block\n";
        for (const auto& s : while_stmt->body->statements) {
            print_statement(*s, indent + 3);
        }
    } else if (auto ret = dynamic_cast<const RetStatement*>(&stmt)) {
        std::cout << indent_str(indent) << "RetStatement\n";
        std::cout << indent_str(indent + 1) << "value:\n";
        print_expression(*ret->value, indent + 2);
    }
}

void print_expression(const Expression& expr, int indent) {
    if (auto binary = dynamic_cast<const BinaryExpression*>(&expr)) {
        std::cout << indent_str(indent) << "BinaryExpression\n";
        std::cout << indent_str(indent + 1) << "op: " << binary_operator_to_string(binary->op) << "\n";
        std::cout << indent_str(indent + 1) << "left:\n";
        print_expression(*binary->left, indent + 2);
        std::cout << indent_str(indent + 1) << "right:\n";
        print_expression(*binary->right, indent + 2);
    } else if (auto var = dynamic_cast<const VariableExpression*>(&expr)) {
        std::cout << indent_str(indent) << "VariableExpression(\"" << var->name << "\")\n";
    } else if (auto num = dynamic_cast<const NumberLiteral*>(&expr)) {
        std::cout << indent_str(indent) << "NumberLiteral(" << num->value << ")\n";
    } else if (auto func_call = dynamic_cast<const FuncCallExpression*>(&expr)) {
        std::cout << indent_str(indent) << "FuncCallExpression\n";
        std::cout << indent_str(indent + 1) << "name: " << func_call->name << "\n";
        if (!func_call->arguments.empty()) {
            std::cout << indent_str(indent + 1) << "arguments:\n";
            for (const auto& arg : func_call->arguments) {
                print_expression(*arg, indent + 2);
            }
        }
    }
}

} // namespace bmlc
