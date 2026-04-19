# Code Generation for BML Compiler (Intel 8080 Target)

This document outlines the steps and strategy for implementing code generation after semantic analysis on the AST for the Intel 8080 architecture.

## Overview

Code generation is the final phase of compilation. After the semantic analyzer validates the AST and symbol table, the code generator walks the AST and produces Intel 8080 assembly instructions. The output should be correct, executable 8080 machine code (or assembly that can be assembled).

## Phase 1: Architecture & Design Decisions

### 1.1 Register Allocation Strategy
Choose one of the following approaches:

#### Option A: Simple Sequential Allocation (Recommended for start)
- Assign registers to variables in declaration order
- Maintain a register pool: B, C, D, E, H, L (A is reserved for arithmetic)
- When a variable is needed, check if already in a register; if not, assign next available
- When all registers full, spill to global data memory (starting from 0x1006)

#### Option B: Linear Scan Allocation (More complex)
- Analyze variable lifetimes using the AST
- Allocate registers based on usage patterns
- Spill variables that don't fit to global data memory

**Recommendation**: Start with Option A for simplicity.

**Important**: ARG 0-3 (0x1000-0x1003) and RET 0-1 (0x1004-0x1005) are fixed reserved addresses. Never allocate global variables to these addresses.

### 1.2 AAccumulator (A) Role
- A is the primary accumulator for arithmetic/logic operations
- Most ALU operations require A as one operand
- Result of ALU ops is stored back in A
- Plan: Use A as a temporary work register; move results to target registers as needed

### 1.3 Memory Layout

BML uses a **fixed address space calling convention** for function arguments and return values:

```
[0x1000]       -- ARG 0 (1 byte, function argument 0)
[0x1001]       -- ARG 1 (1 byte, function argument 1)
[0x1002]       -- ARG 2 (1 byte, function argument 2)
[0x1003]       -- ARG 3 (1 byte, function argument 3)
[0x1004]       -- RET 0 (1 byte, return value low byte)
[0x1005]       -- RET 1 (1 byte, return value high byte, for uint16/int16)
[0x1006 - ...]  -- Global variables data section
[0x0100 - ...]  -- Code segment (program instructions)
```

**Key points:**
- ARG 0-3 are **fixed memory addresses** (0x1000-0x1003)
- RET 0-1 are **fixed memory addresses** (0x1004-0x1005)
- Global variables start immediately after RET slots (0x1006 onwards)
- Each global variable gets a 2-byte slot (enough for uint16, the largest type)
- Track global variable addresses in a memory map during codegen, starting from 0x1006

### 1.4 Calling Convention

BML uses a **fixed-address calling convention** instead of stack frames:

**Caller's responsibility (function call):**
1. Write arguments to ARG 0-3 memory addresses (only the needed slots)
2. Execute CALL instruction
3. Read return value from RET 0-1 memory addresses after CALL returns

**Callee's responsibility (function):**
1. On entry, parameters are available at fixed ARG 0-3 addresses
2. Load arguments as needed from ARG 0-3
3. Before RET, write return value to RET 0-1 addresses
4. Execute RET instruction

**Example: `add(a, b)` call**
```asm
; Write arguments to fixed locations
MOV A, [variable_a]   ; Load first argument
STA 0x1000            ; Write to ARG 0

MOV A, [variable_b]   ; Load second argument
STA 0x1001            ; Write to ARG 1

CALL func_add         ; Call function

; After call, read return value
LDA 0x1004            ; Load RET 0
STA [result_var]      ; Store result
```

**Inside function:**
```asm
func_add:
    LDA 0x1000        ; Load ARG 0 (first param)
    MOV B, A
    LDA 0x1001        ; Load ARG 1 (second param)
    ADD B             ; Add to A
    STA 0x1004        ; Store result in RET 0
    RET
```

## Phase 2: Code Generator Structure

### 2.1 Create Codegen Class/Module

```cpp
class CodeGenerator {
private:
    std::vector<std::string> assembly;      // Accumulated assembly output
    SymbolTable* symbol_table;               // From semantic analyzer
    std::map<std::string, uint16_t> var_addresses; // Variable -> memory address
    std::map<std::string, std::string> var_registers; // Variable -> register
    std::set<std::string> available_registers; // { "B", "C", "D", "E", "H", "L" }
    uint16_t next_memory_addr = 0x1006;     // Start after ARG 0-3 and RET 0-1
    uint16_t next_label_id = 0;
    
    // Fixed calling convention addresses (do not allocate)
    const uint16_t ARG0_ADDR = 0x1000;
    const uint16_t ARG1_ADDR = 0x1001;
    const uint16_t ARG2_ADDR = 0x1002;
    const uint16_t ARG3_ADDR = 0x1003;
    const uint16_t RET0_ADDR = 0x1004;
    const uint16_t RET1_ADDR = 0x1005;
    
public:
    void generate(const Program& program);
    std::vector<std::string> get_output() const;
    
private:
    // AST traversal
    void generate_declaration(const Declaration& decl);
    void generate_statement(const Statement& stmt);
    void generate_expression(const Expression& expr);
    
    // Utilities
    std::string allocate_register(const std::string& var_name);
    void free_register(const std::string& var_name);
    void spill_variable(const std::string& var_name);
    uint16_t allocate_memory(const std::string& var_name, size_t size);
    void emit(const std::string& instruction);
    std::string get_unique_label();
};
```

### 2.2 Supporting Data Structures

**Register Descriptor** (track what's in each register):
```cpp
struct RegisterDescriptor {
    std::string register_name;
    std::string current_variable;  // empty if free
    bool is_dirty;                 // needs write-back?
};
```

**Variable Descriptor** (track where each variable is):
```cpp
struct VariableDescriptor {
    std::string name;
    Type type;
    std::string location;          // "B", "memory(0x1000)", etc.
    bool is_dirty;
};
```

## Phase 3: Code Generation for AST Nodes

### 3.1 Program Node
```
1. Reserve fixed memory addresses for calling convention:
   - 0x1000-0x1003: ARG 0-3 (caller/callee parameter passing)
   - 0x1004-0x1005: RET 0-1 (return values)
   - Next available address: 0x1006
2. For each Global Variable Declaration:
   - Allocate memory starting from 0x1006 onwards
   - Each variable gets 2-byte slot (even uint8, for simplicity)
   - Store address in variable memory map
3. For each Function Declaration:
   - Generate function code (see 3.2)
4. For each Statement (top-level):
   - Generate statement code (see 3.3)
5. Emit halt/exit instruction (RST 0, HLT, or JMP $)
```

Example output:
```asm
; Fixed calling convention addresses (reserved)
; 0x1000 ARG 0
; 0x1001 ARG 1
; 0x1002 ARG 2
; 0x1003 ARG 3
; 0x1004 RET 0
; 0x1005 RET 1

; Global variable storage starts at 0x1006
; var_x at 0x1006
; var_y at 0x1008

; Jump to main routine or entry point
JMP main

; Function definitions
func_add:
    ...
    RET

main:
    ...
    HLT
```

### 3.2 Function Declaration Node
```
1. Emit function label (e.g., func_add:)
2. Load parameters from fixed ARG addresses:
   - Argument 0 -> load from 0x1000
   - Argument 1 -> load from 0x1001
   - Argument 2 -> load from 0x1002
   - Argument 3 -> load from 0x1003
   - Store in registers or local working variables (not ARG slots themselves)
3. For each statement in function body:
   - Generate statement code
4. If function has non-void return type and last statement isn't return:
   - Error (should be caught in semantic analysis)
5. Before RET instruction, write return value to fixed RET addresses:
   - For uint8/int8 return: write to RET 0 (0x1004)
   - For uint16/int16 return: write low byte to RET 0, high byte to RET 1
6. Emit RET instruction
```

Example:
```asm
func_add:
; Parameters available at fixed ARG addresses
; Load argument 0 from ARG 0 (0x1000)
LDA 0x1000
MOV B, A          ; Save in register B for later

; Load argument 1 from ARG 1 (0x1001)
LDA 0x1001

; Perform addition
ADD B             ; A = A + B

; Write return value to RET 0 (0x1004)
STA 0x1004

; Return
RET
```

### 3.3 Statement Nodes

#### Block Statement
```
For each statement in block:
  - Generate statement code
```

#### Assignment Statement
```
1. Generate code for RHS expression
   - Result will be in A (accumulator)
2. Move result from A to target variable location:
   - If variable in register R: MOV R, A
   - If variable in memory (addr): 
     - LXI H, addr
     - MOV M, A (or SHLD addr for 16-bit)
3. Mark variable as up-to-date
```

#### If Statement
```
1. Generate code for condition expression
   - Result in A (0 = false, non-zero = true)
2. Emit: CPI 0
3. Emit: JZ label_else or J label_endif (choose based on logic)
4. Generate code for then-branch
5. Emit: JMP label_endif
6. Emit: label_else:
7. Generate code for else-branch (if exists)
8. Emit: label_endif:
```

#### While Statement
```
1. Emit: label_loop_start:
2. Generate code for condition expression
3. Emit: CPI 0
4. Emit: JZ label_loop_end
5. Generate code for loop body
6. Emit: JMP label_loop_start
7. Emit: label_loop_end:
```

#### Return Statement
```
1. If expression:
   - Generate code for return expression
   - Result in A
   - If function return type is uint8/int8:
     - Store A to RET 0 (0x1004): STA 0x1004
   - If function return type is uint16/int16:
     - Store low byte to RET 0 (0x1004): STA 0x1004
     - Store high byte to RET 1 (0x1005): MOV A, [high_byte]; STA 0x1005
2. Emit: RET
```

### 3.4 Expression Nodes

#### Literal Expression
```
1. Load literal into A
   - If uint8/int8: MVI A, #value
   - If uint16/int16: 
     - LXI HL, #value (or load high/low separately)
     - Move HL to A (or result register)
2. Return result location (A, or HL for 16-bit)
```

#### Variable Expression
```
1. Look up variable location:
   - If in register R: result is R
   - If in memory: 
     - Load from memory into A
     - Result in A
2. Return result location
```

#### Binary Operation Expression
```
1. Generate code for LHS
   - Result in A (or specific register)
2. Save A to temporary location (register or memory)
3. Generate code for RHS
   - Result in A
4. Load LHS from temporary into secondary register (B, C, etc.)
5. Perform operation:
   - For PLUS: ADD B (A += B)
   - For MINUS: SUB B (A -= B)
   - For AND: ANA B (A &= B)
   - For OR: ORA B (A |= B)
   - For XOR: XRA B (A ^= B)
   - For LESS: CMP B (A - B, set flags, result goes to flags)
6. For comparison, additional logic may be needed to set result in A
7. Return result in A
```

#### Unary Operation Expression (if supported, e.g., negation)
```
1. Generate code for operand
   - Result in A
2. For negation: CMA (complement A), then add 1
3. Return result in A
```

#### Function Call Expression
```
1. Evaluate each argument in order:
   - Generate code for argument 0 expression
   - Result in A
   - Store A to fixed memory address: 0x1000 (ARG 0)
   - Repeat for arguments 1-3, storing to 0x1001-0x1003 (ARG 1-3)
2. Emit: CALL func_name
3. After CALL returns, retrieve return value from fixed RET addresses:
   - For uint8/int8: Load from 0x1004 (RET 0) into A
   - For uint16/int16: Load from 0x1004 and 0x1005 (RET 0 and RET 1)
4. Return result location (A)
```

Example:
```asm
; Call add(x, y)
; Assume x is at memory 0x1010, y is at memory 0x1011

; Prepare argument 0
LDA 0x1010        ; Load x
STA 0x1000        ; Write to ARG 0

; Prepare argument 1
LDA 0x1011        ; Load y
STA 0x1001        ; Write to ARG 1

; Call function
CALL func_add

; Retrieve return value
LDA 0x1004        ; Load RET 0
; A now contains result
```

## Phase 4: Register & Memory Management

### 4.1 Register Allocation

```cpp
std::string allocate_register(const std::string& var_name) {
    // Find first available register
    for (auto& reg : available_registers) {
        if (var_registers[reg].empty()) {
            var_registers[reg] = var_name;
            return reg;
        }
    }
    // All registers full; spill least recently used
    spill_variable(...);
    return allocate_register(var_name); // Recursively try again
}
```

### 4.2 Memory Allocation

```cpp
uint16_t allocate_memory(const std::string& var_name, size_t size) {
    // Fixed addresses are reserved: 0x1000-0x1005 (ARG 0-3, RET 0-1)
    // Global variables start at 0x1006
    uint16_t addr = next_memory_addr;
    var_addresses[var_name] = addr;
    next_memory_addr += size;
    return addr;
}
```

The memory map is organized as:
- 0x1000-0x1003: ARG 0-3 (reserved, do not allocate)
- 0x1004-0x1005: RET 0-1 (reserved, do not allocate)
- 0x1006 onwards: Global variables (next_memory_addr initialized to 0x1006)

### 4.3 Spilling Variables

When a register is needed but all are in use:
```
1. Identify a variable in a register to spill
2. If variable modified: write back to memory
3. Free the register
```

## Phase 5: Label & Control Flow

### 5.1 Label Generation

```cpp
std::string get_unique_label(const std::string& prefix = "label") {
    return prefix + "_" + std::to_string(next_label_id++);
}
```

### 5.2 Jump Instructions

Use conditional jumps for if/while:
- `JZ addr` -- Jump if zero flag set
- `JNZ addr` -- Jump if zero flag not set
- `JC addr` -- Jump if carry set
- `JNC addr` -- Jump if carry not set
- `JMP addr` -- Unconditional jump

Comparison sets flags; test with JZ/JNZ.

## Phase 6: Output Generation

### 6.1 Assembly Output Format

Emit as plain text assembly (can be fed to an 8080 assembler):

```asm
; Fixed calling convention addresses (reserved)
; ARG 0-3: 0x1000-0x1003
; RET 0-1: 0x1004-0x1005

; Global variables start at 0x1006
; var_x at 0x1006
; var_y at 0x1008

; Program entry point
    JMP main

; Function definitions
func_add:
    ; Load parameters from ARG 0-3
    LDA 0x1000
    MOV B, A
    LDA 0x1001
    ADD B
    ; Store return value
    STA 0x1004
    RET

main:
    ; Program code here
    ...
    HLT
```

### 6.2 Machine Code Output (Optional)

If targeting direct machine code:
- Maintain a `byte_offset` counter
- For each instruction, compute its encoding per 8080ISA.md
- Emit opcode bytes in correct order (low byte first for addresses)

## Phase 7: Implementation Checklist

### Stage 1: Minimal Codegen (Hello World)
- [ ] Generate code for simple variable declarations
- [ ] Generate code for global variable assignments
- [ ] Generate code for print/output statements (if supported)
- [ ] Generate basic halt/exit at end

### Stage 2: Expressions & Arithmetic
- [ ] Generate code for literal expressions
- [ ] Generate code for variable references
- [ ] Generate code for binary operations (PLUS, MINUS)
- [ ] Handle temporary register allocation for compound expressions
- [ ] Test: `a = 5 + 3; b = a + 2;`

### Stage 3: Control Flow
- [ ] Generate code for if statements
- [ ] Generate code for while loops
- [ ] Generate labels and jumps
- [ ] Test: `if (x < 5) { y = 1; } else { y = 2; }`

### Stage 4: Functions
- [ ] Load function arguments from fixed ARG 0-3 addresses (0x1000-0x1003)
- [ ] Generate function call code (write args to ARG slots, CALL, read return from RET slots)
- [ ] Store return values to fixed RET 0-1 addresses (0x1004-0x1005)
- [ ] Generate return statements (write result to RET slots, RET instruction)
- [ ] Test: `func add(a, b) -> uint8 { return a + b; } result = add(3, 4);`

### Stage 5: All Operators
- [ ] AND, OR, XOR operations
- [ ] LESS comparison
- [ ] Other binary operators as defined in grammar

### Stage 6: Type-Aware Codegen
- [ ] Handle uint8 vs uint16 (1-byte vs 2-byte operations)
- [ ] Handle int8 and int16 (signed operations if needed)
- [ ] Extended precision for 16-bit arithmetic

### Stage 7: Optimization (Optional)
- [ ] Register allocation improvements
- [ ] Dead code elimination
- [ ] Constant folding
- [ ] Peephole optimization

## Phase 8: Testing Strategy

### 8.1 Unit Tests
For each AST node type, verify:
- Input: Semantic-checked AST
- Output: Correct 8080 assembly
- Method: Compare generated instructions against expected

Example test:
```
Input AST:
  Program
    VariableDeclaration("x", Type::UINT8)
    Assignment("x", Literal(5))
    
Expected output (fragment):
  MVI A, 5
  STA 0x1000  ; or MOV B, A if x is in register B
```

### 8.2 Integration Tests
For each `.bml` test file in `tests/`:
1. Lex → Parse → Semantic Check → Codegen
2. Assemble generated code (use 8080 assembler if available)
3. Simulate or run on 8080 emulator
4. Verify correct output/behavior

### 8.3 Reference Tests
Compare against manual assembly for known programs:
- Fibonacci sequence
- Factorial
- Sum of array
- Etc.

## Phase 9: Debugging & Validation

### 9.1 Intermediate Output
Emit human-readable assembly with comments:
```asm
; Global variable: x (type: uint8) at memory 0x1000
; Global variable: y (type: uint16) at memory 0x1002

main:
    ; Assignment: x = 5
    MVI A, 5
    STA 0x1000
    
    ; Assignment: y = x + 10
    LDA 0x1000
    ADI 10
    STA 0x1002
    
    HLT
```

### 9.2 Trace/Debug Mode
Add option to emit detailed log during codegen:
- Variable locations and register assignments
- Fixed address accesses (ARG 0-3, RET 0-1)
- Jump targets and label definitions
- Function call/return sequences

### 9.3 Verification
After backend (if you add one):
- Use 8080 simulator to step through generated code
- Compare final register/memory state against expected
- Use debugger to spot assignment, control flow, or calling convention issues

## Phase 10: Advanced Topics (Future)

### 10.1 Calling Convention Improvements
- The current fixed-address calling convention (ARG 0-3, RET 0-1) works well for functions with up to 4 parameters
- If supporting more than 4 parameters: use memory stack or extended ARG slots
- If supporting multi-byte parameters: adjust memory layout or use register pairs (HL, DE, BC)
- Consider register preservation: document which registers are caller-save vs callee-save

### 10.2 Memory Management
- Local variable allocation (currently not supported; would need memory zone for local vars)
- Dynamic array or heap allocation (if language supports)
- Memory protection and bounds checking

### 10.3 Optimizations
- Loop unrolling
- Inlining
- Instruction selection improvements

### 10.4 Error Handling in Codegen
- Report assembly errors (e.g., address out of range)
- Suggest register allocation failures
- Validate instruction operands

## Summary

Code generation transforms a validated AST into Intel 8080 assembly instructions. The key steps are:

1. **Design** register allocation, memory layout, and calling conventions
2. **Structure** the code generator with clear visitor methods for each AST node type
3. **Implement** bottom-up: literals → variables → expressions → statements → functions
4. **Manage** registers and memory with spilling and allocation logic
5. **Generate** correct assembly with proper labels, jumps, and instruction encoding
6. **Test** incrementally with unit tests, integration tests, and simulators
7. **Debug** with detailed output and trace modes

Start with the simplest cases (literals and basic assignments) and incrementally add complexity (expressions, control flow, function calls).
