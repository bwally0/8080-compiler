#include <bmlc/codegen/register_pool.hpp>
#include <algorithm>

namespace bmlc {

// ============================================================================
// RegisterPool Implementation
// ============================================================================

RegisterPool::RegisterPool() {
    // Initialize all available registers as free
    registers["B"] = RegisterDescriptor("B");
    registers["C"] = RegisterDescriptor("C");
    registers["D"] = RegisterDescriptor("D");
    registers["E"] = RegisterDescriptor("E");
    registers["H"] = RegisterDescriptor("H");
    registers["L"] = RegisterDescriptor("L");
    
    // Initialize usage counters to 0
    for (const auto& reg : registers) {
        usage_count[reg.first] = 0;
    }
}

std::string RegisterPool::allocate(const std::string& var_name) {
    // Find first free register
    for (auto& [reg_name, descriptor] : registers) {
        if (descriptor.is_free()) {
            descriptor.current_variable = var_name;
            descriptor.is_dirty = false;
            mark_used(reg_name);
            return reg_name;
        }
    }
    // No free registers available
    return "";
}

void RegisterPool::free(const std::string& reg_name) {
    auto it = registers.find(reg_name);
    if (it != registers.end()) {
        it->second.current_variable = "";
        it->second.is_dirty = false;
    }
}

std::string RegisterPool::get_variable_in(const std::string& reg_name) const {
    auto it = registers.find(reg_name);
    if (it != registers.end()) {
        return it->second.current_variable;
    }
    return "";
}

std::string RegisterPool::get_register_of(const std::string& var_name) const {
    for (const auto& [reg_name, descriptor] : registers) {
        if (descriptor.current_variable == var_name) {
            return reg_name;
        }
    }
    return "";
}

bool RegisterPool::is_free(const std::string& reg_name) const {
    auto it = registers.find(reg_name);
    if (it != registers.end()) {
        return it->second.is_free();
    }
    return false;
}

std::set<std::string> RegisterPool::get_free_registers() const {
    std::set<std::string> free_regs;
    for (const auto& [reg_name, descriptor] : registers) {
        if (descriptor.is_free()) {
            free_regs.insert(reg_name);
        }
    }
    return free_regs;
}

std::string RegisterPool::get_lru_register() {
    // Find the register with the lowest usage count
    std::string lru_reg;
    int min_usage = INT_MAX;
    
    for (const auto& [reg_name, usage] : usage_count) {
        auto it = registers.find(reg_name);
        if (it != registers.end() && !it->second.is_free()) {
            if (usage < min_usage) {
                min_usage = usage;
                lru_reg = reg_name;
            }
        }
    }
    
    return lru_reg;
}

void RegisterPool::mark_used(const std::string& reg_name) {
    auto it = usage_count.find(reg_name);
    if (it != usage_count.end()) {
        it->second = global_usage_counter++;
    }
}

void RegisterPool::reset() {
    for (auto& [reg_name, descriptor] : registers) {
        descriptor.current_variable = "";
        descriptor.is_dirty = false;
    }
    for (auto& [reg_name, count] : usage_count) {
        count = 0;
    }
    global_usage_counter = 0;
}

// ============================================================================
// MemoryManager Implementation
// ============================================================================

MemoryManager::MemoryManager() {
    // next_address starts at GLOBAL_VAR_START
    next_address = GLOBAL_VAR_START;
}

uint16_t MemoryManager::allocate(const std::string& var_name, size_t size) {
    // Check if already allocated
    if (is_allocated(var_name)) {
        return var_addresses[var_name];
    }
    
    // Allocate at current position
    uint16_t addr = next_address;
    var_addresses[var_name] = addr;
    var_sizes[var_name] = size;
    next_address += size;
    
    return addr;
}

void MemoryManager::deallocate(const std::string& var_name) {
    auto it = var_addresses.find(var_name);
    if (it != var_addresses.end()) {
        var_addresses.erase(it);
        var_sizes.erase(var_name);
    }
}

uint16_t MemoryManager::get_address(const std::string& var_name) const {
    auto it = var_addresses.find(var_name);
    if (it != var_addresses.end()) {
        return it->second;
    }
    return 0;
}

bool MemoryManager::is_allocated(const std::string& var_name) const {
    return var_addresses.find(var_name) != var_addresses.end();
}

uint16_t MemoryManager::get_next_available() const {
    return next_address;
}

void MemoryManager::reset() {
    var_addresses.clear();
    var_sizes.clear();
    next_address = GLOBAL_VAR_START;
}

bool MemoryManager::is_reserved(uint16_t addr) {
    return addr >= ARG0_ADDR && addr <= RET1_ADDR;
}

} // namespace bmlc
