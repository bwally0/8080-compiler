#pragma once

#include <memory>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <bmlc/parser/ast.hpp>
#include <bmlc/semantic/symbol_table.hpp>

namespace bmlc {

/**
 * Code Generator for Intel 8080 Assembly
 * 
 * Generates 8080 assembly code from a semantic-analyzed AST.
 * 
 * Memory Layout:
 *   0x1000-0x1003: ARG 0-3 (function argument passing - reserved)
 *   0x1004-0x1005: RET 0-1 (return value slots - reserved)
 *   0x1006-...:    Global variables
 *   0x0100-...:    Code segment
 */
class CodeGenerator {
public:
    CodeGenerator(const SymbolTable* symbol_table);
    
    /**
     * Generate 8080 assembly code from the AST
     * @param program The semantic-analyzed AST program
     */
    void generate(const Program& program);
    
    /**
     * Get the generated assembly instructions
     * @return Vector of assembly instruction strings
     */
    std::vector<std::string> get_output() const;
    
private:
    // Fixed calling convention addresses (do not allocate)
    static constexpr uint16_t ARG0_ADDR = 0x1000;
    static constexpr uint16_t ARG1_ADDR = 0x1001;
    static constexpr uint16_t ARG2_ADDR = 0x1002;
    static constexpr uint16_t ARG3_ADDR = 0x1003;
    static constexpr uint16_t RET0_ADDR = 0x1004;
    static constexpr uint16_t RET1_ADDR = 0x1005;
    static constexpr uint16_t GLOBAL_VAR_START = 0x1006;
    
    // State
    std::vector<std::string> assembly;      // Accumulated assembly output
    const SymbolTable* symbol_table;        // From semantic analyzer
    std::map<std::string, uint16_t> var_addresses; // Variable -> memory address
    std::map<std::string, std::string> var_registers; // Variable -> register
    std::set<std::string> available_registers; // { "B", "C", "D", "E", "H", "L" }
    uint16_t next_memory_addr = GLOBAL_VAR_START;
    uint16_t next_label_id = 0;
    std::string current_function_name;      // Track which function we're generating
    
    // AST traversal methods
    void generate_declaration(const Declaration& decl);
    void generate_variable_declaration(const VarDeclaration& decl);
    void generate_function_declaration(const FuncDeclaration& decl);
    
    void generate_statement(const Statement& stmt);
    void generate_block(const Block& block);
    void generate_assignment(const AssignmentStatement& stmt);
    void generate_if(const IfStatement& stmt);
    void generate_while(const WhileStatement& stmt);
    void generate_return(const RetStatement& stmt);
    
    std::string generate_expression(const Expression& expr);
    std::string generate_literal(const NumberLiteral& expr);
    std::string generate_variable(const VariableExpression& expr);
    std::string generate_binary_op(const BinaryExpression& expr);
    std::string generate_function_call(const FuncCallExpression& expr);
    
    // Register and memory management
    std::string allocate_register(const std::string& var_name);
    void free_register(const std::string& var_name);
    void spill_variable(const std::string& var_name);
    uint16_t allocate_memory(const std::string& var_name, size_t size = 2);
    
    // Utility methods
    void emit(const std::string& instruction);
    void emit_comment(const std::string& comment);
    std::string get_unique_label(const std::string& prefix = "label");
    std::string get_variable_location(const std::string& var_name);
    
    // Instruction helpers
    void emit_load_to_a(const std::string& source);
    void emit_store_from_a(const std::string& dest);
    void emit_move(const std::string& dest, const std::string& src);
    void emit_label(const std::string& label);
    
    // 16-bit/multi-byte handling
    void emit_load_16bit(const std::string& reg_pair, uint16_t value);
    void emit_load_address(const std::string& reg_pair, uint16_t address);
};

} // namespace bmlc
