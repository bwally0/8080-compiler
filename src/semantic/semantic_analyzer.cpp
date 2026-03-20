#include <bmlc/semantic/semantic_analyzer.hpp>
#include <sstream>

namespace bmlc {

SemanticAnalyzer::SemanticAnalyzer() 
    : current_function_return_type_(Type::UINT8) {
}

void SemanticAnalyzer::analyze(const Program& program) {
    // Collect all declarations (functions and variables)
    collect_declarations(program);
    
    // Check that main function exists
    if (!symbol_table_.has_main()) {
        add_error("error: no main() function defined");
    }
    
    // Check function bodies for undefined references
    check_function_bodies(program);
    
    // Check for unused symbols
    check_unused_symbols();
}

void SemanticAnalyzer::collect_declarations(const Program& program) {
    for (const auto& decl : program.declarations) {
        visit_declaration(*decl);
    }
}

void SemanticAnalyzer::visit_declaration(const Declaration& decl) {
    if (auto var_decl = dynamic_cast<const VarDeclaration*>(&decl)) {
        visit_var_declaration(*var_decl);
    } else if (auto func_decl = dynamic_cast<const FuncDeclaration*>(&decl)) {
        visit_func_declaration(*func_decl);
    }
}

void SemanticAnalyzer::visit_var_declaration(const VarDeclaration& decl) {
    if (symbol_table_.is_defined(decl.name)) {
        add_error(format_location(decl.location) + ": error: variable '" + decl.name + 
                  "' already declared");
    } else {
        Symbol symbol;
        symbol.name = decl.name;
        symbol.location = decl.location;
        symbol.type = decl.type;
        symbol.is_function = false;
        symbol_table_.define(symbol);
    }
}

void SemanticAnalyzer::visit_func_declaration(const FuncDeclaration& decl) {
    if (symbol_table_.is_defined(decl.name)) {
        add_error(format_location(decl.location) + ": error: function '" + decl.name + 
                  "' already declared");
    } else {
        Symbol symbol;
        symbol.name = decl.name;
        symbol.location = decl.location;
        symbol.type = decl.return_type;
        symbol.is_function = true;
        symbol.parameters = decl.parameters;
        symbol_table_.define(symbol);
    }
}

void SemanticAnalyzer::check_function_bodies(const Program& program) {
    for (const auto& decl : program.declarations) {
        if (auto func_decl = dynamic_cast<const FuncDeclaration*>(decl.get())) {
            visit_func_body(*func_decl);
        }
    }
}

void SemanticAnalyzer::visit_func_body(const FuncDeclaration& func) {
    symbol_table_.mark_as_used(func.name);
    
    // Set current function return type
    current_function_return_type_ = func.return_type;
    
    // Create local scope for function parameters
    local_scope_.clear();
    local_scope_types_.clear();
    for (const auto& param : func.parameters) {
        local_scope_.insert(param.name);
        local_scope_types_[param.name] = param.type;
    }
    
    if (func.body) {
        visit_block(*func.body);
    }
    
    // Clear local scope
    local_scope_.clear();
    local_scope_types_.clear();
}

void SemanticAnalyzer::visit_block(const Block& block) {
    for (const auto& stmt : block.statements) {
        visit_statement(*stmt);
    }
}

void SemanticAnalyzer::visit_statement(const Statement& stmt) {
    if (auto assign = dynamic_cast<const AssignmentStatement*>(&stmt)) {
        visit_assignment(*assign);
    } else if (auto if_stmt = dynamic_cast<const IfStatement*>(&stmt)) {
        visit_if_statement(*if_stmt);
    } else if (auto while_stmt = dynamic_cast<const WhileStatement*>(&stmt)) {
        visit_while_statement(*while_stmt);
    } else if (auto ret_stmt = dynamic_cast<const RetStatement*>(&stmt)) {
        visit_return_statement(*ret_stmt);
    }
}

void SemanticAnalyzer::visit_assignment(const AssignmentStatement& stmt) {
    if (!is_symbol_defined(stmt.name)) {
        add_error(format_location(stmt.location) + ": error: undefined variable '" + 
                  stmt.name + "'");
    } else {
        symbol_table_.mark_as_used(stmt.name);
        
        // Check assignment type compatibility
        if (stmt.value) {
            Symbol* var_symbol = symbol_table_.lookup(stmt.name);
            Type expr_type = get_expression_type(*stmt.value);
            
            if (var_symbol && var_symbol->type != expr_type) {
                add_error(format_location(stmt.location) + ": error: type mismatch in assignment: " +
                          "cannot assign " + type_to_string(expr_type) + 
                          " to " + type_to_string(var_symbol->type));
            }
        }
    }
    
    if (stmt.value) {
        visit_expression(*stmt.value);
    }
}

void SemanticAnalyzer::visit_if_statement(const IfStatement& stmt) {
    if (stmt.condition) {
        visit_expression(*stmt.condition);
    }
    if (stmt.body) {
        visit_block(*stmt.body);
    }
}

void SemanticAnalyzer::visit_while_statement(const WhileStatement& stmt) {
    if (stmt.condition) {
        visit_expression(*stmt.condition);
    }
    if (stmt.body) {
        visit_block(*stmt.body);
    }
}

void SemanticAnalyzer::visit_return_statement(const RetStatement& stmt) {
    if (stmt.value) {
        Type expr_type = get_expression_type(*stmt.value);
        if (expr_type != current_function_return_type_) {
            add_error(format_location(stmt.value->location) + ": error: return type mismatch: " +
                      "expected " + type_to_string(current_function_return_type_) + 
                      " but got " + type_to_string(expr_type));
        }
        visit_expression(*stmt.value);
    }
}

void SemanticAnalyzer::visit_expression(const Expression& expr) {
    if (auto var_expr = dynamic_cast<const VariableExpression*>(&expr)) {
        if (!is_symbol_defined(var_expr->name)) {
            add_error(format_location(var_expr->location) + ": error: undefined variable '" + 
                      var_expr->name + "'");
        } else {
            symbol_table_.mark_as_used(var_expr->name);
        }
    } else if (auto call_expr = dynamic_cast<const FuncCallExpression*>(&expr)) {
        if (!symbol_table_.is_defined(call_expr->name)) {
            add_error(format_location(call_expr->location) + ": error: undefined function '" + 
                      call_expr->name + "'");
        } else {
            symbol_table_.mark_as_used(call_expr->name);
        }
        for (const auto& arg : call_expr->arguments) {
            visit_expression(*arg);
        }
    } else if (auto binary_expr = dynamic_cast<const BinaryExpression*>(&expr)) {
        if (binary_expr->left) {
            visit_expression(*binary_expr->left);
        }
        if (binary_expr->right) {
            visit_expression(*binary_expr->right);
        }
    }
}

void SemanticAnalyzer::add_error(const std::string& message) {
    errors_.push_back(message);
}

void SemanticAnalyzer::add_warning(const std::string& message) {
    warnings_.push_back(message);
}

bool SemanticAnalyzer::is_symbol_defined(const std::string& name) const {
    // Check local scope first (function parameters)
    if (local_scope_.find(name) != local_scope_.end()) {
        return true;
    }
    // Check global scope
    return symbol_table_.is_defined(name);
}

std::string SemanticAnalyzer::format_location(const SourceLocation& loc) const {
    return loc.filename + ":" + std::to_string(loc.line) + ":" + std::to_string(loc.column);
}

void SemanticAnalyzer::check_unused_symbols() {
    auto unused = symbol_table_.get_unused_symbols();
    for (auto symbol : unused) {
        std::string kind = symbol->is_function ? "function" : "variable";
        add_warning(format_location(symbol->location) + ": warning: unused " + kind + 
                    " '" + symbol->name + "'");
    }
}

Type SemanticAnalyzer::get_expression_type(const Expression& expr) {
    if (auto literal = dynamic_cast<const NumberLiteral*>(&expr)) {
        // For now, assume all numeric literals are uint8
        return Type::UINT8;
    } else if (auto var_expr = dynamic_cast<const VariableExpression*>(&expr)) {
        // Check local scope first (function parameters)
        auto it = local_scope_types_.find(var_expr->name);
        if (it != local_scope_types_.end()) {
            return it->second;
        }
        
        // Check global scope
        Symbol* symbol = symbol_table_.lookup(var_expr->name);
        if (symbol) {
            return symbol->type;
        }
        // Shouldn't reach here if visit_expression was called first
        return Type::UINT8;
    } else if (auto call_expr = dynamic_cast<const FuncCallExpression*>(&expr)) {
        Symbol* symbol = symbol_table_.lookup(call_expr->name);
        if (symbol && symbol->is_function) {
            return symbol->type;
        }
        return Type::UINT8;
    } else if (auto binary_expr = dynamic_cast<const BinaryExpression*>(&expr)) {
        Type left_type = get_expression_type(*binary_expr->left);
        Type right_type = get_expression_type(*binary_expr->right);
        
        // Check that both operands have the same type
        if (left_type != right_type) {
            add_error(format_location(binary_expr->location) + ": error: type mismatch in binary operation: " +
                      type_to_string(left_type) + " " + binary_operator_to_string(binary_expr->op) + 
                      " " + type_to_string(right_type));
        }
        
        return left_type;
    }
    
    return Type::UINT8;
}

} // namespace bmlc
