#pragma once

#include <bmlc/parser/ast.hpp>
#include <bmlc/semantic/symbol_table.hpp>
#include <vector>
#include <string>
#include <unordered_set>

namespace bmlc {

class SemanticAnalyzer {
public:
    explicit SemanticAnalyzer();
    
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
    void visit_expression(const Expression& expr);
    
    bool is_symbol_defined(const std::string& name) const;
    std::string format_location(const SourceLocation& loc) const;
};

} // namespace bmlc
