#include <bmlc/codegen/codegen.hpp>
#include <bmlc/codegen/instruction_emitter.hpp>
#include <stdexcept>
#include <sstream>
#include <iomanip>

namespace bmlc {

// ============================================================================
// Constructor
// ============================================================================

CodeGenerator::CodeGenerator(const SymbolTable* symbol_table)
    : symbol_table(symbol_table) {
    // Initialize available registers (A is reserved for arithmetic)
    available_registers = {"B", "C", "D", "E", "H", "L"};
    next_memory_addr = GLOBAL_VAR_START;
    next_label_id = 0;
}

// ============================================================================
// Main Generation Method
// ============================================================================

void CodeGenerator::generate(const Program& program) {
    // First pass: allocate memory for all global variables
    for (const auto& decl : program.declarations) {
        if (auto var_decl = dynamic_cast<const VarDeclaration*>(decl.get())) {
            allocate_memory(var_decl->name, 2);  // All vars get 2 bytes for simplicity
        }
    }
    
    // Check if main function exists
    bool has_main_function = false;
    for (const auto& decl : program.declarations) {
        if (auto func_decl = dynamic_cast<const FuncDeclaration*>(decl.get())) {
            if (func_decl->name == "main") {
                has_main_function = true;
                break;
            }
        }
    }
    
    // Emit header comments
    emit_comment("Fixed calling convention addresses (reserved)");
    emit_comment("ARG 0-3: 0000h-0003h");
    emit_comment("RET 0-1: 0004h-0005h");
    emit("");
    
    // Emit global variable map
    emit_comment("Global variables:");
    for (const auto& [var_name, addr] : var_addresses) {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0') << std::setw(4) << addr << "h";
        emit_comment(var_name + " at " + oss.str());
    }
    emit("");
    
    // Emit entry point: initialize stack pointer, then jump to main
    emit_comment("Program entry point");
    emit_comment("Initialize stack pointer");
    emit(InstructionEmitter::lxi("SP", 0x0F00));  // Stack at 3840 (high RAM)
    emit(InstructionEmitter::jmp("main"));
    emit("");
    
    // Generate all declarations (functions, later)
    for (const auto& decl : program.declarations) {
        generate_declaration(*decl);
    }
    
    // Emit main entry point only if main function doesn't exist
    if (!has_main_function) {
        emit("");
        emit(InstructionEmitter::label("main"));
        emit(InstructionEmitter::hlt());  // Halt when no user-defined main
    }
    
    // If main function exists, it should end with RET (already handled in generate_return)
    // If we generated a default main, it ends with HLT
}

std::vector<std::string> CodeGenerator::get_output() const {
    return assembly;
}

// ============================================================================
// Declaration Generation
// ============================================================================

void CodeGenerator::generate_declaration(const Declaration& decl) {
    if (auto var_decl = dynamic_cast<const VarDeclaration*>(&decl)) {
        generate_variable_declaration(*var_decl);
    } else if (auto func_decl = dynamic_cast<const FuncDeclaration*>(&decl)) {
        generate_function_declaration(*func_decl);
    }
}

void CodeGenerator::generate_variable_declaration(const VarDeclaration& decl) {
    // Variables are just symbolic; their memory is already allocated in generate()
    // Nothing to emit here
}

void CodeGenerator::generate_function_declaration(const FuncDeclaration& decl) {
    // Track the current function being generated
    current_function_name = decl.name;
    
    // Emit function label
    emit("");
    emit(InstructionEmitter::label(decl.name));
    emit_comment("Function: " + decl.name);
    
    // Load parameters from fixed ARG addresses and store to memory
    for (int i = 0; i < static_cast<int>(decl.parameters.size()); ++i) {
        emit_comment("Load parameter " + std::to_string(i) + " from ARG " + 
                     std::to_string(i));
        emit(InstructionEmitter::lda(ARG0_ADDR + i));
        
        // Allocate memory for this parameter (treat it like a local variable)
        uint16_t param_addr = allocate_memory(decl.parameters[i].name, 1);
        emit(InstructionEmitter::sta(param_addr));
    }
    
    // Generate function body
    emit_comment("Function body:");
    if (decl.body) {
        generate_block(*decl.body);
    }
    
    current_function_name = "";
}

// ============================================================================
// Statement Generation
// ============================================================================

void CodeGenerator::generate_statement(const Statement& stmt) {
    if (auto assign = dynamic_cast<const AssignmentStatement*>(&stmt)) {
        generate_assignment(*assign);
    } else if (auto if_stmt = dynamic_cast<const IfStatement*>(&stmt)) {
        generate_if(*if_stmt);
    } else if (auto while_stmt = dynamic_cast<const WhileStatement*>(&stmt)) {
        generate_while(*while_stmt);
    } else if (auto ret = dynamic_cast<const RetStatement*>(&stmt)) {
        generate_return(*ret);
    } else if (auto expr_stmt = dynamic_cast<const ExpressionStatement*>(&stmt)) {
        generate_expression(*expr_stmt->expression);
    }
}

void CodeGenerator::generate_block(const Block& block) {
    for (const auto& stmt : block.statements) {
        generate_statement(*stmt);
    }
}

void CodeGenerator::generate_assignment(const AssignmentStatement& stmt) {
    emit_comment("Assignment: " + stmt.name + " = ...");
    
    // Generate RHS expression (result will be in A)
    generate_expression(*stmt.value);
    
    // Get target variable location
    if (var_addresses.find(stmt.name) == var_addresses.end()) {
        throw std::runtime_error("Undefined variable in assignment: " + stmt.name);
    }
    uint16_t target_addr = var_addresses[stmt.name];
    
    // Store A to target variable
    emit(InstructionEmitter::sta(target_addr));
}

void CodeGenerator::generate_if(const IfStatement& stmt) {
    std::string label_else = get_unique_label("else");
    std::string label_endif = get_unique_label("endif");
    
    emit_comment("If statement");
    
    // Generate condition (result in A)
    generate_expression(*stmt.condition);
    
    // Test result: if zero, jump to else
    emit(InstructionEmitter::cpi(0));
    emit(InstructionEmitter::jz(label_else));
    
    // Generate then-block
    generate_block(*stmt.body);
    
    // Jump to endif
    emit(InstructionEmitter::jmp(label_endif));
    
    // Else label
    emit(InstructionEmitter::label(label_else));
    
    // Endif label
    emit(InstructionEmitter::label(label_endif));
}

void CodeGenerator::generate_while(const WhileStatement& stmt) {
    std::string label_loop_start = get_unique_label("loop_start");
    std::string label_loop_end = get_unique_label("loop_end");
    
    emit_comment("While loop");
    
    // Loop start
    emit(InstructionEmitter::label(label_loop_start));
    
    // Generate condition (result in A)
    generate_expression(*stmt.condition);
    
    // Test result: if zero, jump to end
    emit(InstructionEmitter::cpi(0));
    emit(InstructionEmitter::jz(label_loop_end));
    
    // Generate loop body
    generate_block(*stmt.body);
    
    // Jump back to start
    emit(InstructionEmitter::jmp(label_loop_start));
    
    // Loop end
    emit(InstructionEmitter::label(label_loop_end));
}

void CodeGenerator::generate_return(const RetStatement& stmt) {
    emit_comment("Return statement");
    
    if (stmt.value) {
        // Generate return expression (result in A)
        generate_expression(*stmt.value);
        
        // Store return value to RET 0
        emit(InstructionEmitter::sta(RET0_ADDR));
    }
    
    // For main function, emit HLT (entry point shouldn't return)
    if (current_function_name == "main") {
        emit(InstructionEmitter::hlt());
    } else {
        // For other functions, emit RET
        emit(InstructionEmitter::ret());
    }
}

// ============================================================================
// Expression Generation
// ============================================================================

std::string CodeGenerator::generate_expression(const Expression& expr) {
    if (auto literal = dynamic_cast<const NumberLiteral*>(&expr)) {
        return generate_literal(*literal);
    } else if (auto var = dynamic_cast<const VariableExpression*>(&expr)) {
        return generate_variable(*var);
    } else if (auto binop = dynamic_cast<const BinaryExpression*>(&expr)) {
        return generate_binary_op(*binop);
    } else if (auto call = dynamic_cast<const FuncCallExpression*>(&expr)) {
        return generate_function_call(*call);
    }
    throw std::runtime_error("Unknown expression type");
}

std::string CodeGenerator::generate_literal(const NumberLiteral& expr) {
    // Load literal into A
    emit(InstructionEmitter::mvi("A", static_cast<uint8_t>(expr.value)));
    return "A";
}

std::string CodeGenerator::generate_variable(const VariableExpression& expr) {
    // Load variable into A
    if (var_addresses.find(expr.name) == var_addresses.end()) {
        throw std::runtime_error("Undefined variable: " + expr.name);
    }
    uint16_t addr = var_addresses[expr.name];
    emit(InstructionEmitter::lda(addr));
    return "A";
}

std::string CodeGenerator::generate_binary_op(const BinaryExpression& expr) {
    // Generate LHS (result in A)
    generate_expression(*expr.left);
    
    // Save A to temporary register
    emit(InstructionEmitter::mov("B", "A"));
    
    // Generate RHS (result in A)
    generate_expression(*expr.right);
    
    // Perform operation
    switch (expr.op) {
        case BinaryOperator::PLUS:
            emit(InstructionEmitter::add("B"));
            break;
        case BinaryOperator::MINUS:
            emit(InstructionEmitter::sub("B"));
            break;
        case BinaryOperator::AND:
            emit(InstructionEmitter::ana("B"));
            break;
        case BinaryOperator::OR:
            emit(InstructionEmitter::ora("B"));
            break;
        case BinaryOperator::XOR:
            emit(InstructionEmitter::xra("B"));
            break;
        case BinaryOperator::LESS:
            // CRITICAL FIX: CMP r computes A - r, sets carry if A < r (unsigned)
            // Current: B = LHS, A = RHS
            // Need: A = LHS, then CMP with RHS to compute LHS - RHS
            emit(InstructionEmitter::mov("C", "A"));  // C = RHS
            emit(InstructionEmitter::mov("A", "B"));  // A = LHS (B already has it)
            emit(InstructionEmitter::cmp("C"));  // A - C: carry set if LHS < RHS
            
            // Set A = 1 if LHS < RHS (carry set), else A = 0
            std::string label_not_less = get_unique_label("not_less");
            std::string label_less_end = get_unique_label("less_end");
            
            // Jump if NOT carry (LHS >= RHS)
            emit(InstructionEmitter::jnc(label_not_less));
            
            // LHS < RHS path: set A = 1
            emit(InstructionEmitter::mvi("A", 1));
            emit(InstructionEmitter::jmp(label_less_end));
            
            // LHS >= RHS path: set A = 0
            emit(InstructionEmitter::label(label_not_less));
            emit(InstructionEmitter::mvi("A", 0));
            
            // End of comparison
            emit(InstructionEmitter::label(label_less_end));
            break;
    }
    
    return "A";
}

std::string CodeGenerator::generate_function_call(const FuncCallExpression& expr) {
    // Built-in: out(port, value) -> OUT instruction
    if (expr.name == "out") {
        auto port_literal = dynamic_cast<const NumberLiteral*>(expr.arguments[0].get());
        if (!port_literal) {
            throw std::runtime_error("out() port must be a constant");
        }
        generate_expression(*expr.arguments[1]);
        emit(InstructionEmitter::out(static_cast<uint8_t>(port_literal->value)));
        return "A";
    }

    emit_comment("Function call: " + expr.name);
    
    // Evaluate arguments and store to ARG slots
    for (int i = 0; i < static_cast<int>(expr.arguments.size()); ++i) {
        // Generate argument expression
        generate_expression(*expr.arguments[i]);
        
        // Store A to ARG slot
        uint16_t arg_addr = ARG0_ADDR + i;
        emit(InstructionEmitter::sta(arg_addr));
    }
    
    // Call function
    emit(InstructionEmitter::call(expr.name));
    
    // Load return value from RET 0
    emit(InstructionEmitter::lda(RET0_ADDR));
    
    return "A";
}

// ============================================================================
// Register and Memory Management
// ============================================================================

std::string CodeGenerator::allocate_register(const std::string& var_name) {
    // Try to find a free register
    for (const auto& reg : available_registers) {
        if (var_registers.find(reg) == var_registers.end() || var_registers[reg].empty()) {
            var_registers[reg] = var_name;
            return reg;
        }
    }
    // All registers full; spill least recently used
    // For now, just return empty (will be handled at higher level)
    return "";
}

void CodeGenerator::free_register(const std::string& var_name) {
    for (auto& [reg, var] : var_registers) {
        if (var == var_name) {
            var = "";
            break;
        }
    }
}

void CodeGenerator::spill_variable(const std::string& var_name) {
    // Not implemented yet
    // Would write register contents to memory
}

uint16_t CodeGenerator::allocate_memory(const std::string& var_name, size_t size) {
    if (var_addresses.find(var_name) != var_addresses.end()) {
        return var_addresses[var_name];
    }
    
    uint16_t addr = next_memory_addr;
    var_addresses[var_name] = addr;
    next_memory_addr += size;
    
    return addr;
}

// ============================================================================
// Utility Methods
// ============================================================================

void CodeGenerator::emit(const std::string& instruction) {
    assembly.push_back(instruction);
}

void CodeGenerator::emit_comment(const std::string& comment) {
    assembly.push_back("; " + comment);
}

std::string CodeGenerator::get_unique_label(const std::string& prefix) {
    return prefix + "_" + std::to_string(next_label_id++);
}

std::string CodeGenerator::get_variable_location(const std::string& var_name) {
    // Check if in register
    for (const auto& [reg, var] : var_registers) {
        if (var == var_name) {
            return reg;
        }
    }
    
    // Otherwise in memory
    auto it = var_addresses.find(var_name);
    if (it != var_addresses.end()) {
        return "memory(0x" + std::to_string(it->second) + ")";
    }
    
    return "unknown";
}

void CodeGenerator::emit_load_to_a(const std::string& source) {
    if (source == "A") {
        // Already in A
        return;
    } else if (source.find("0x") == 0) {
        // Memory address
        uint16_t addr = std::stoul(source, nullptr, 16);
        emit(InstructionEmitter::lda(addr));
    } else {
        // Register
        emit(InstructionEmitter::mov("A", source));
    }
}

void CodeGenerator::emit_store_from_a(const std::string& dest) {
    if (dest == "A") {
        // Already in A
        return;
    } else if (dest.find("0x") == 0) {
        // Memory address
        uint16_t addr = std::stoul(dest, nullptr, 16);
        emit(InstructionEmitter::sta(addr));
    } else {
        // Register
        emit(InstructionEmitter::mov(dest, "A"));
    }
}

void CodeGenerator::emit_move(const std::string& dest, const std::string& src) {
    emit(InstructionEmitter::mov(dest, src));
}

void CodeGenerator::emit_label(const std::string& label) {
    emit(InstructionEmitter::label(label));
}

void CodeGenerator::emit_load_16bit(const std::string& reg_pair, uint16_t value) {
    emit(InstructionEmitter::lxi(reg_pair, value));
}

void CodeGenerator::emit_load_address(const std::string& reg_pair, uint16_t address) {
    emit(InstructionEmitter::lxi(reg_pair, address));
}

} // namespace bmlc
