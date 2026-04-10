#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <bmlc/parser/ast.hpp>

namespace bmlc {

/**
 * Descriptor for register allocation state
 * Tracks what variable (if any) is currently in each register
 */
struct RegisterDescriptor {
    std::string register_name;    // "B", "C", "D", "E", "H", "L"
    std::string current_variable; // empty string if free
    bool is_dirty;                // true if needs write-back to memory
    
    RegisterDescriptor() 
        : register_name(""), current_variable(""), is_dirty(false) {}
    
    RegisterDescriptor(const std::string& reg_name)
        : register_name(reg_name), current_variable(""), is_dirty(false) {}
    
    bool is_free() const {
        return current_variable.empty();
    }
};

/**
 * Descriptor for variable allocation state
 * Tracks where each variable is located (register, memory, or both)
 */
struct VariableDescriptor {
    std::string name;
    Type type;
    std::string location;  // "B", "C", "D", "E", "H", "L", or "memory(0xXXXX)"
    uint16_t memory_addr;  // if location is memory
    bool is_dirty;         // true if value differs from memory
    bool is_in_register;   // true if value is in a register
    
    VariableDescriptor()
        : name(""), type(Type::UINT8), location(""), memory_addr(0),
          is_dirty(false), is_in_register(false) {}
    
    VariableDescriptor(const std::string& var_name, Type var_type)
        : name(var_name), type(var_type), location(""), memory_addr(0),
          is_dirty(false), is_in_register(false) {}
};

/**
 * Helper struct to track function information during codegen
 * Used when entering a function to track its parameters and return type
 */
struct FunctionContext {
    std::string function_name;
    Type return_type;
    int param_count;
    std::vector<std::string> param_names;
    std::vector<Type> param_types;
    
    FunctionContext()
        : function_name(""), return_type(Type::UINT8), param_count(0) {}
    
    FunctionContext(const std::string& name, Type ret_type, int pcount)
        : function_name(name), return_type(ret_type), param_count(pcount) {}
};

/**
 * Helper struct for tracking expression evaluation results
 * Indicates where the result of an expression is located
 */
struct ExpressionResult {
    std::string location;  // "A", "B", "C", etc. (register) or "address(0xXXXX)"
    Type type;             // The type of the expression
    bool in_register;      // true if result is in a CPU register
    
    ExpressionResult()
        : location(""), type(Type::UINT8), in_register(true) {}
    
    ExpressionResult(const std::string& loc, Type t, bool in_reg)
        : location(loc), type(t), in_register(in_reg) {}
    
    bool is_in_accumulator() const {
        return location == "A";
    }
};

} // namespace bmlc
