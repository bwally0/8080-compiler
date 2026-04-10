#pragma once

#include <string>
#include <cstdint>

namespace bmlc {

/**
 * Helper class for emitting Intel 8080 assembly instructions
 * Provides utilities for generating correct instruction syntax and encodings
 */
class InstructionEmitter {
public:
    InstructionEmitter() = default;
    
    // Data movement
    static std::string mov(const std::string& dest, const std::string& src);
    static std::string mvi(const std::string& dest, uint8_t value);
    static std::string lda(uint16_t address);
    static std::string sta(uint16_t address);
    static std::string lhld(uint16_t address);
    static std::string shld(uint16_t address);
    static std::string lxi(const std::string& reg_pair, uint16_t value);
    
    // Load indirect
    static std::string ldax(const std::string& reg_pair);
    static std::string stax(const std::string& reg_pair);
    
    // Arithmetic and logic
    static std::string add(const std::string& reg);
    static std::string adi(uint8_t value);
    static std::string adc(const std::string& reg);
    static std::string aci(uint8_t value);
    static std::string sub(const std::string& reg);
    static std::string sui(uint8_t value);
    static std::string sbb(const std::string& reg);
    static std::string sbi(uint8_t value);
    
    // Increment/Decrement
    static std::string inr(const std::string& reg);
    static std::string dcr(const std::string& reg);
    static std::string inx(const std::string& reg_pair);
    static std::string dcx(const std::string& reg_pair);
    
    // 16-bit operations
    static std::string dad(const std::string& reg_pair);
    
    // Bitwise operations
    static std::string ana(const std::string& reg);
    static std::string ani(uint8_t value);
    static std::string ora(const std::string& reg);
    static std::string ori(uint8_t value);
    static std::string xra(const std::string& reg);
    static std::string xri(uint8_t value);
    
    // Comparison
    static std::string cmp(const std::string& reg);
    static std::string cpi(uint8_t value);
    
    // Rotate operations
    static std::string rlc();  // Rotate left (A)
    static std::string rrc();  // Rotate right (A)
    static std::string ral();  // Rotate left through carry (A)
    static std::string rar();  // Rotate right through carry (A)
    
    // Complement
    static std::string cma();  // Complement A
    static std::string cmc();  // Complement carry
    static std::string stc();  // Set carry
    
    // Jumps
    static std::string jmp(const std::string& label);
    static std::string jz(const std::string& label);
    static std::string jnz(const std::string& label);
    static std::string jc(const std::string& label);
    static std::string jnc(const std::string& label);
    static std::string jpe(const std::string& label);
    static std::string jpo(const std::string& label);
    static std::string jp(const std::string& label);
    static std::string jm(const std::string& label);
    
    // Calls and returns
    static std::string call(const std::string& label);
    static std::string cz(const std::string& label);
    static std::string cnz(const std::string& label);
    static std::string cc(const std::string& label);
    static std::string cnc(const std::string& label);
    static std::string cpe(const std::string& label);
    static std::string cpo(const std::string& label);
    static std::string cp(const std::string& label);
    static std::string cm(const std::string& label);
    static std::string ret();
    static std::string rz();
    static std::string rnz();
    static std::string rc();
    static std::string rnc();
    static std::string rpe();
    static std::string rpo();
    static std::string rp();
    static std::string rm();
    static std::string rst(uint8_t n);
    
    // Stack operations
    static std::string push(const std::string& reg_pair);
    static std::string pop(const std::string& reg_pair);
    static std::string xthl();  // Exchange HL with stack top
    static std::string sphl();  // Set SP to HL
    
    // I/O
    static std::string in(uint8_t port);
    static std::string out(uint8_t port);
    
    // Flow control
    static std::string ei();    // Enable interrupts
    static std::string di();    // Disable interrupts
    static std::string nop();   // No operation
    static std::string hlt();   // Halt
    
    // Labels and comments
    static std::string label(const std::string& name);
    static std::string comment(const std::string& text);
    
    // Utility for formatting immediate values
    static std::string format_immediate(uint8_t value);
    static std::string format_immediate_16(uint16_t value);
    static std::string format_address(uint16_t address);
};

} // namespace bmlc
