#include <bmlc/codegen/instruction_emitter.hpp>
#include <sstream>
#include <iomanip>

namespace bmlc {

// ============================================================================
// Utility Functions
// ============================================================================

std::string InstructionEmitter::format_immediate(uint8_t value) {
    std::ostringstream oss;
    oss << std::hex << std::setw(2) << std::setfill('0') 
        << static_cast<int>(value) << "h";
    return oss.str();
}

std::string InstructionEmitter::format_immediate_16(uint16_t value) {
    std::ostringstream oss;
    oss << std::hex << std::setw(4) << std::setfill('0') 
        << static_cast<int>(value) << "h";
    return oss.str();
}

std::string InstructionEmitter::format_address(uint16_t address) {
    std::ostringstream oss;
    oss << std::hex << std::setw(4) << std::setfill('0') 
        << static_cast<int>(address) << "h";
    return oss.str();
}

// ============================================================================
// Data Movement Instructions
// ============================================================================

std::string InstructionEmitter::mov(const std::string& dest, const std::string& src) {
    return "MOV " + dest + ", " + src;
}

std::string InstructionEmitter::mvi(const std::string& dest, uint8_t value) {
    return "MVI " + dest + ", " + format_immediate(value);
}

std::string InstructionEmitter::lda(uint16_t address) {
    return "LDA " + format_address(address);
}

std::string InstructionEmitter::sta(uint16_t address) {
    return "STA " + format_address(address);
}

std::string InstructionEmitter::lhld(uint16_t address) {
    return "LHLD " + format_address(address);
}

std::string InstructionEmitter::shld(uint16_t address) {
    return "SHLD " + format_address(address);
}

std::string InstructionEmitter::lxi(const std::string& reg_pair, uint16_t value) {
    return "LXI " + reg_pair + ", " + format_immediate_16(value);
}

// ============================================================================
// Load Indirect Instructions
// ============================================================================

std::string InstructionEmitter::ldax(const std::string& reg_pair) {
    return "LDAX " + reg_pair;
}

std::string InstructionEmitter::stax(const std::string& reg_pair) {
    return "STAX " + reg_pair;
}

// ============================================================================
// Arithmetic Instructions
// ============================================================================

std::string InstructionEmitter::add(const std::string& reg) {
    return "ADD " + reg;
}

std::string InstructionEmitter::adi(uint8_t value) {
    return "ADI " + format_immediate(value);
}

std::string InstructionEmitter::adc(const std::string& reg) {
    return "ADC " + reg;
}

std::string InstructionEmitter::aci(uint8_t value) {
    return "ACI " + format_immediate(value);
}

std::string InstructionEmitter::sub(const std::string& reg) {
    return "SUB " + reg;
}

std::string InstructionEmitter::sui(uint8_t value) {
    return "SUI " + format_immediate(value);
}

std::string InstructionEmitter::sbb(const std::string& reg) {
    return "SBB " + reg;
}

std::string InstructionEmitter::sbi(uint8_t value) {
    return "SBI " + format_immediate(value);
}

// ============================================================================
// Increment/Decrement Instructions
// ============================================================================

std::string InstructionEmitter::inr(const std::string& reg) {
    return "INR " + reg;
}

std::string InstructionEmitter::dcr(const std::string& reg) {
    return "DCR " + reg;
}

std::string InstructionEmitter::inx(const std::string& reg_pair) {
    return "INX " + reg_pair;
}

std::string InstructionEmitter::dcx(const std::string& reg_pair) {
    return "DCX " + reg_pair;
}

// ============================================================================
// 16-bit Arithmetic
// ============================================================================

std::string InstructionEmitter::dad(const std::string& reg_pair) {
    return "DAD " + reg_pair;
}

// ============================================================================
// Bitwise Operations
// ============================================================================

std::string InstructionEmitter::ana(const std::string& reg) {
    return "ANA " + reg;
}

std::string InstructionEmitter::ani(uint8_t value) {
    return "ANI " + format_immediate(value);
}

std::string InstructionEmitter::ora(const std::string& reg) {
    return "ORA " + reg;
}

std::string InstructionEmitter::ori(uint8_t value) {
    return "ORI " + format_immediate(value);
}

std::string InstructionEmitter::xra(const std::string& reg) {
    return "XRA " + reg;
}

std::string InstructionEmitter::xri(uint8_t value) {
    return "XRI " + format_immediate(value);
}

// ============================================================================
// Comparison
// ============================================================================

std::string InstructionEmitter::cmp(const std::string& reg) {
    return "CMP " + reg;
}

std::string InstructionEmitter::cpi(uint8_t value) {
    return "CPI " + format_immediate(value);
}

// ============================================================================
// Rotate Operations (operate on Accumulator)
// ============================================================================

std::string InstructionEmitter::rlc() {
    return "RLC";
}

std::string InstructionEmitter::rrc() {
    return "RRC";
}

std::string InstructionEmitter::ral() {
    return "RAL";
}

std::string InstructionEmitter::rar() {
    return "RAR";
}

// ============================================================================
// Complement/Carry Operations
// ============================================================================

std::string InstructionEmitter::cma() {
    return "CMA";
}

std::string InstructionEmitter::cmc() {
    return "CMC";
}

std::string InstructionEmitter::stc() {
    return "STC";
}

// ============================================================================
// Jump Instructions
// ============================================================================

std::string InstructionEmitter::jmp(const std::string& label) {
    return "JMP " + label;
}

std::string InstructionEmitter::jz(const std::string& label) {
    return "JZ " + label;
}

std::string InstructionEmitter::jnz(const std::string& label) {
    return "JNZ " + label;
}

std::string InstructionEmitter::jc(const std::string& label) {
    return "JC " + label;
}

std::string InstructionEmitter::jnc(const std::string& label) {
    return "JNC " + label;
}

std::string InstructionEmitter::jpe(const std::string& label) {
    return "JPE " + label;
}

std::string InstructionEmitter::jpo(const std::string& label) {
    return "JPO " + label;
}

std::string InstructionEmitter::jp(const std::string& label) {
    return "JP " + label;
}

std::string InstructionEmitter::jm(const std::string& label) {
    return "JM " + label;
}

// ============================================================================
// Call Instructions
// ============================================================================

std::string InstructionEmitter::call(const std::string& label) {
    return "CALL " + label;
}

std::string InstructionEmitter::cz(const std::string& label) {
    return "CZ " + label;
}

std::string InstructionEmitter::cnz(const std::string& label) {
    return "CNZ " + label;
}

std::string InstructionEmitter::cc(const std::string& label) {
    return "CC " + label;
}

std::string InstructionEmitter::cnc(const std::string& label) {
    return "CNC " + label;
}

std::string InstructionEmitter::cpe(const std::string& label) {
    return "CPE " + label;
}

std::string InstructionEmitter::cpo(const std::string& label) {
    return "CPO " + label;
}

std::string InstructionEmitter::cp(const std::string& label) {
    return "CP " + label;
}

std::string InstructionEmitter::cm(const std::string& label) {
    return "CM " + label;
}

// ============================================================================
// Return Instructions
// ============================================================================

std::string InstructionEmitter::ret() {
    return "RET";
}

std::string InstructionEmitter::rz() {
    return "RZ";
}

std::string InstructionEmitter::rnz() {
    return "RNZ";
}

std::string InstructionEmitter::rc() {
    return "RC";
}

std::string InstructionEmitter::rnc() {
    return "RNC";
}

std::string InstructionEmitter::rpe() {
    return "RPE";
}

std::string InstructionEmitter::rpo() {
    return "RPO";
}

std::string InstructionEmitter::rp() {
    return "RP";
}

std::string InstructionEmitter::rm() {
    return "RM";
}

std::string InstructionEmitter::rst(uint8_t n) {
    // n should be 0-7
    return "RST " + std::to_string(static_cast<int>(n & 0x07));
}

// ============================================================================
// Stack Operations
// ============================================================================

std::string InstructionEmitter::push(const std::string& reg_pair) {
    return "PUSH " + reg_pair;
}

std::string InstructionEmitter::pop(const std::string& reg_pair) {
    return "POP " + reg_pair;
}

std::string InstructionEmitter::xthl() {
    return "XTHL";
}

std::string InstructionEmitter::sphl() {
    return "SPHL";
}

// ============================================================================
// I/O Instructions
// ============================================================================

std::string InstructionEmitter::in(uint8_t port) {
    return "IN " + format_immediate(port);
}

std::string InstructionEmitter::out(uint8_t port) {
    return "OUT " + format_immediate(port);
}

// ============================================================================
// Flow Control / Special Instructions
// ============================================================================

std::string InstructionEmitter::ei() {
    return "EI";
}

std::string InstructionEmitter::di() {
    return "DI";
}

std::string InstructionEmitter::nop() {
    return "NOP";
}

std::string InstructionEmitter::hlt() {
    return "HLT";
}

// ============================================================================
// Labels and Comments
// ============================================================================

std::string InstructionEmitter::label(const std::string& name) {
    return name + ":";
}

std::string InstructionEmitter::comment(const std::string& text) {
    return "; " + text;
}

} // namespace bmlc
