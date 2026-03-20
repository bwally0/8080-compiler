#pragma once

#include <bmlc/parser/ast.hpp>
#include <bmlc/semantic/symbol_table.hpp>
#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>

namespace bmlc {

class SemanticAnalyzer {
public:
    explicit SemanticAnalyzer();
    
    // 8080 Calling Convention Constants
    static constexpr int MAX_ARG_SLOTS = 4;
    static constexpr int MAX_ARG_BYTES = 4;
    static constexpr int MAX_RETURN_BYTES = 2;
    
    // Type Range Constants
    static constexpr int LIT_UINT8_MIN = 0;
    static constexpr int LIT_UINT8_MAX = 255;
    static constexpr int LIT_UINT16_MIN = 0;
    static constexpr int LIT_UINT16_MAX = 65535;
    static constexpr int LIT_INT8_MIN = -128;
    static constexpr int LIT_INT8_MAX = 127;
    static constexpr int LIT_INT16_MIN = -32768;
    static constexpr int LIT_INT16_MAX = 32767;
    
    void analyze(const Program& program);
    const SymbolTable& get_symbol_table() const { return symbol_table_; }
    
    const std::vector<std::string>& get_errors() const { return errors_; }
    const std::vector<std::string>& get_warnings() const { return warnings_; }
    
    bool has_errors() const { return !errors_.empty(); }
    bool has_warnings() const { return !warnings_.empty(); }

private:
    SymbolTable symbol_table_;
    std::vector<std::string> errors_;
    std::vector<std::string> warnings_;
    std::unordered_set<std::string> local_scope_;
    std::unordered_map<std::string, Type> local_scope_types_;
    Type current_function_return_type_;
    
    void add_error(const std::string& message);
    void add_warning(const std::string& message);
    
    void collect_declarations(const Program& program);
    void visit_declaration(const Declaration& decl);
    void visit_var_declaration(const VarDeclaration& decl);
    void visit_func_declaration(const FuncDeclaration& decl);
    
    void check_function_bodies(const Program& program);
    void visit_func_body(const FuncDeclaration& func);
    void visit_block(const Block& block);
    void visit_statement(const Statement& stmt);
    void visit_assignment(const AssignmentStatement& stmt);
    void visit_if_statement(const IfStatement& stmt);
    void visit_while_statement(const WhileStatement& stmt);
    void visit_return_statement(const RetStatement& stmt);
    
    void check_unused_symbols();
    
    // Type checking
    Type get_expression_type(const Expression& expr);
    void visit_expression(const Expression& expr);
    
    // Function call checks
    void check_function_declarations();
    void check_function_call(const FuncCallExpression& call);
    
    // Literal value checks
    bool is_literal_in_range(int value, Type type) const;
    void check_literal_value(const NumberLiteral& literal, Type expected_type);
    void check_literal_value_in_expression(const Expression& expr, Type expected_type);
    
    // Helper functions
    int get_type_size(Type type) const;
    bool is_symbol_defined(const std::string& name) const;
    std::string format_location(const SourceLocation& loc) const;
};

} // namespace bmlc
