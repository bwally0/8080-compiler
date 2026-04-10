#pragma once

#include <string>
#include <set>
#include <map>
#include <memory>
#include <bmlc/codegen/descriptors.hpp>

namespace bmlc {

/**
 * Register pool manager for 8080 codegen
 * Manages allocation and deallocation of CPU registers
 */
class RegisterPool {
public:
    RegisterPool();
    
    /**
     * Allocate a free register for a variable
     * @param var_name The variable name
     * @return Register name ("B", "C", "D", "E", "H", "L") or empty string if none available
     */
    std::string allocate(const std::string& var_name);
    
    /**
     * Free a register, making it available for reallocation
     * @param reg_name Register name
     */
    void free(const std::string& reg_name);
    
    /**
     * Get the variable currently in a register
     * @param reg_name Register name
     * @return Variable name, or empty string if free
     */
    std::string get_variable_in(const std::string& reg_name) const;
    
    /**
     * Get the register containing a variable
     * @param var_name Variable name
     * @return Register name, or empty string if not in a register
     */
    std::string get_register_of(const std::string& var_name) const;
    
    /**
     * Check if a register is free
     * @param reg_name Register name
     * @return true if free, false if occupied
     */
    bool is_free(const std::string& reg_name) const;
    
    /**
     * Get all free registers
     * @return Set of free register names
     */
    std::set<std::string> get_free_registers() const;
    
    /**
     * Get the least recently used register from a set
     * @return Register name
     */
    std::string get_lru_register();
    
    /**
     * Mark a register as recently used (for LRU tracking)
     * @param reg_name Register name
     */
    void mark_used(const std::string& reg_name);
    
    /**
     * Reset all registers to free state
     */
    void reset();
    
private:
    std::map<std::string, RegisterDescriptor> registers;
    std::map<std::string, int> usage_count; // For LRU tracking
    int global_usage_counter = 0;
};

/**
 * Memory manager for global variables in 8080 codegen
 */
class MemoryManager {
public:
    // Fixed calling convention addresses (reserved, cannot allocate)
    static constexpr uint16_t ARG0_ADDR = 0x0000;
    static constexpr uint16_t ARG1_ADDR = 0x0001;
    static constexpr uint16_t ARG2_ADDR = 0x0002;
    static constexpr uint16_t ARG3_ADDR = 0x0003;
    static constexpr uint16_t RET0_ADDR = 0x0004;
    static constexpr uint16_t RET1_ADDR = 0x0005;
    static constexpr uint16_t GLOBAL_VAR_START = 0x0006;
    
    MemoryManager();
    
    /**
     * Allocate memory for a global variable
     * @param var_name Variable name
     * @param size Size in bytes (default 2 for simplicity)
     * @return Memory address, or 0 if allocation failed
     */
    uint16_t allocate(const std::string& var_name, size_t size = 2);
    
    /**
     * Deallocate memory for a variable (not typically used)
     * @param var_name Variable name
     */
    void deallocate(const std::string& var_name);
    
    /**
     * Get the memory address of a variable
     * @param var_name Variable name
     * @return Memory address, or 0 if not found
     */
    uint16_t get_address(const std::string& var_name) const;
    
    /**
     * Check if a variable has been allocated
     * @param var_name Variable name
     * @return true if allocated, false otherwise
     */
    bool is_allocated(const std::string& var_name) const;
    
    /**
     * Get the next available memory address
     * @return Next available address
     */
    uint16_t get_next_available() const;
    
    /**
     * Reset all allocations (for new compilation)
     */
    void reset();
    
    /**
     * Check if an address is in the reserved calling convention area
     * @param addr Address to check
     * @return true if reserved
     */
    static bool is_reserved(uint16_t addr);
    
private:
    std::map<std::string, uint16_t> var_addresses;
    std::map<std::string, size_t> var_sizes;
    uint16_t next_address = GLOBAL_VAR_START;
};

} // namespace bmlc
