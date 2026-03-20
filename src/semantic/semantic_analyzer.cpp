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
    
    // Check function declarations for calling convention compliance
    check_function_declarations();
    
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
            
            // Check literal values in assignments
            if (var_symbol) {
                check_literal_value_in_expression(*stmt.value, var_symbol->type);
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
            check_function_call(*call_expr);
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

int SemanticAnalyzer::get_type_size(Type type) const {
    switch (type) {
        case Type::UINT8:
        case Type::INT8:
            return 1;
        case Type::UINT16:
        case Type::INT16:
            return 2;
        default:
            return 1;
    }
}

void SemanticAnalyzer::check_function_declarations() {
    auto symbols = symbol_table_.get_all_symbols();
    for (auto symbol : symbols) {
        if (!symbol->is_function) continue;
        
        // Check argument count and total size
        int total_arg_size = 0;
        for (const auto& param : symbol->parameters) {
            total_arg_size += get_type_size(param.type);
        }
        
        // Check parameter count
        if (symbol->parameters.size() > MAX_ARG_SLOTS) {
            add_error(format_location(symbol->location) + ": error: function '" + symbol->name + 
                      "' has " + std::to_string(symbol->parameters.size()) + 
                      " parameters but calling convention only supports " + 
                      std::to_string(MAX_ARG_SLOTS) + " (ARG 0-" + 
                      std::to_string(MAX_ARG_SLOTS - 1) + ")");
        }
        
        // Check total argument size
        if (total_arg_size > MAX_ARG_BYTES) {
            add_error(format_location(symbol->location) + ": error: function '" + symbol->name + 
                      "' arguments require " + std::to_string(total_arg_size) + 
                      " bytes but only " + std::to_string(MAX_ARG_BYTES) + 
                      " available in calling convention");
        }
        
        // Check return value size
        int return_size = get_type_size(symbol->type);
        if (return_size > MAX_RETURN_BYTES) {
            add_error(format_location(symbol->location) + ": error: function '" + symbol->name + 
                      "' return type requires " + std::to_string(return_size) + 
                      " bytes but only " + std::to_string(MAX_RETURN_BYTES) + 
                      " return slots available (RET 0-" + 
                      std::to_string(MAX_RETURN_BYTES - 1) + ")");
        }
    }
}

void SemanticAnalyzer::check_function_call(const FuncCallExpression& call) {
    Symbol* func_symbol = symbol_table_.lookup(call.name);
    if (!func_symbol || !func_symbol->is_function) {
        return;  // Error already reported in visit_expression
    }
    
    // Check argument count
    if (call.arguments.size() != func_symbol->parameters.size()) {
        add_error(format_location(call.location) + ": error: function '" + call.name + 
                  "' expects " + std::to_string(func_symbol->parameters.size()) + 
                  " arguments but got " + std::to_string(call.arguments.size()));
        return;
    }
    
    // Check argument types
    for (size_t i = 0; i < call.arguments.size(); ++i) {
        Type arg_type = get_expression_type(*call.arguments[i]);
        Type param_type = func_symbol->parameters[i].type;
        
        if (arg_type != param_type) {
            add_error(format_location(call.location) + ": error: argument " + 
                      std::to_string(i + 1) + " type mismatch: expected " + 
                      type_to_string(param_type) + " but got " + type_to_string(arg_type));
        }
        
        // Check literal values in arguments
        check_literal_value_in_expression(*call.arguments[i], param_type);
    }
}

bool SemanticAnalyzer::is_literal_in_range(int value, Type type) const {
    switch (type) {
        case Type::UINT8:
            return value >= LIT_UINT8_MIN && value <= LIT_UINT8_MAX;
        case Type::UINT16:
            return value >= LIT_UINT16_MIN && value <= LIT_UINT16_MAX;
        case Type::INT8:
            return value >= LIT_INT8_MIN && value <= LIT_INT8_MAX;
        case Type::INT16:
            return value >= LIT_INT16_MIN && value <= LIT_INT16_MAX;
        default:
            return true;
    }
}

void SemanticAnalyzer::check_literal_value(const NumberLiteral& literal, Type expected_type) {
    int value = literal.value;
    
    // Check if literal fits in the expected type
    if (!is_literal_in_range(value, expected_type)) {
        std::string range_str;
        switch (expected_type) {
            case Type::UINT8:
                range_str = "0-255";
                break;
            case Type::UINT16:
                range_str = "0-65535";
                break;
            case Type::INT8:
                range_str = "-128-127";
                break;
            case Type::INT16:
                range_str = "-32768-32767";
                break;
            default:
                range_str = "unknown";
        }
        
        add_error(format_location(literal.location) + ": error: literal " + 
                  std::to_string(value) + " does not fit in type " + 
                  type_to_string(expected_type) + " (range " + range_str + ")");
    }
    
    // Warn about negative literals in unsigned types
    if (value < 0 && (expected_type == Type::UINT8 || expected_type == Type::UINT16)) {
        add_warning(format_location(literal.location) + ": warning: negative literal " + 
                    std::to_string(value) + " assigned to unsigned type " + 
                    type_to_string(expected_type));
    }
}

void SemanticAnalyzer::check_literal_value_in_expression(const Expression& expr, Type expected_type) {
    if (auto literal = dynamic_cast<const NumberLiteral*>(&expr)) {
        check_literal_value(*literal, expected_type);
    } else if (auto binary_expr = dynamic_cast<const BinaryExpression*>(&expr)) {
        // Recursively check literals in binary expressions
        if (binary_expr->left) {
            check_literal_value_in_expression(*binary_expr->left, expected_type);
        }
        if (binary_expr->right) {
            check_literal_value_in_expression(*binary_expr->right, expected_type);
        }
    }
}

} // namespace bmlc
