# Code Generation Implementation Progress

## Current Status (Completed)

### Infrastructure ✅
- **InstructionEmitter** - All 50+ 8080 instructions
- **RegisterPool** - Register allocation with LRU tracking
- **MemoryManager** - Global variable memory management (starting at 0x0006)
- **CodeGenerator** - Main code generator class

### Code Generation Methods ✅
- Variable declarations and memory allocation
- Function declarations (parameter loading from ARG addresses)
- Statements: assignment, if, while, return
- Expressions: literals, variables, binary operations, function calls
- Label and jump generation for control flow
- Assembly output to `.asm` file

### Pipeline Integration ✅
- CodeGenerator now called in `main.cpp` after semantic analysis
- Assembly output written to `.bml` → `.asm` file

---

## Next Steps (In Priority Order)

### 1. **Test Stage 1: Simple Variable Assignment** 🎯
Test with the simplest possible program:

**File: `tests/simple.bml`**
```bml
var x: uint8;
x = 5;
```

**Expected output (`tests/simple.asm`)**:
```asm
; Fixed calling convention addresses (reserved)
; ARG 0-3: 0x0000-0x0003
; RET 0-1: 0x0004-0x0005

; Global variables:
; x at 0x0006

; Program entry point
JMP main

main:
    HLT
```

**Commands**:
```bash
cd c:\Users\Brendan\8080-compiler
mkdir -p build
cd build
cmake ..
cmake --build .
./bmlc ../tests/simple.bml
cat ../tests/simple.asm
```

### 2. **Fix LESS Operator** ⚠️

The LESS operator (binary `<`) currently only emits `CMP B` but doesn't set A to the result.

**To implement**:
In `codegen.cpp::generate_binary_op()`, after `CMP B`:
```cpp
case BinaryOperator::LESS:
    // CMP B sets flags: Z=equal, C=less than
    emit(InstructionEmitter::cmp("B"));
    
    // Set A = 1 if less (carry flag set), else 0
    // Instructions: 
    // - JNC label_not_less (jump if not carry)
    // - MVI A, 1 (set to true)
    // - JMP label_endif
    // label_not_less:
    // - MVI A, 0 (set to false)
    // label_endif:
    
    std::string label_not_less = get_unique_label("not_less");
    std::string label_endif = get_unique_label("endif");
    
    emit(InstructionEmitter::jnc(label_not_less));
    emit(InstructionEmitter::mvi("A", 1));
    emit(InstructionEmitter::jmp(label_endif));
    emit(InstructionEmitter::label(label_not_less));
    emit(InstructionEmitter::mvi("A", 0));
    emit(InstructionEmitter::label(label_endif));
    break;
```

### 3. **Test Stage 2: Arithmetic Expressions**

**File: `tests/arithmetic.bml`**
```bml
var a: uint8;
var b: uint8;
var result: uint8;
a = 5;
b = 3;
result = a + b;
```

**Expected**: Assignment with ADD instruction, stores result to memory address 0x000A

### 4. **Test Stage 3: Control Flow**

**File: `tests/simple_if.bml`**
```bml
var x: uint8;
var y: uint8;
x = 5;
if x {
    y = 1;
}
```

### 5. **Debug Output Option** (Optional Enhancement)

Add command-line flag to print debug info:
```cpp
// In CodeGenerator class
class CodeGenerator {
    bool debug_mode = false;  // Add this flag
    
    // In emit(), also print to stderr when debug_mode
    void emit(const std::string& instruction) {
        assembly.push_back(instruction);
        if (debug_mode) {
            std::cerr << "[CODEGEN] " << instruction << std::endl;
        }
    }
};
```

---

## Architecture Decisions Made

### Memory Layout ✅
```
0x0000-0x0003: ARG 0-3 (function arguments - reserved)
0x0004-0x0005: RET 0-1 (return values - reserved)
0x0006+:       Global variables (implicit memory addresses)
```

### Calling Convention ✅
- **Caller**: Write args to ARG 0-3, CALL, read RET 0-1
- **Callee**: Load parameters from ARG 0-3, write return to RET 0-1, RET

### Register Pool ✅
- Available: B, C, D, E, H, L (A is accumulator/temporary)
- LRU eviction for spilling (not yet needed with only 6 regs)
- All expressions pass results through A (8080 ALU design constraint)

---

## Known Limitations

1. **LESS operator** - Not yet fully implemented (CMP without result setting)
2. **16-bit types** - Basic structure present, not tested
3. **16-bit arithmetic** - Need to implement for uint16/int16
4. **Function parameters as local variables** - Currently load from ARG but could be more flexible
5. **No register spilling in practice** - Current tests won't fill all 6 registers
6. **No type-aware instruction selection** - All immediates treated as uint8

---

## Testing Checklist

- [ ] Stage 1: Simple assignment (`x = 5;`)
- [ ] Stage 1 extended: Variable-to-variable assignment
- [ ] Stage 2: Binary arithmetic (PLUS, MINUS)
- [ ] Stage 2 extended: Compound expressions (`x = a + b + c;`)
- [ ] Stage 3: If statements with condition
- [ ] Stage 3 extended: While loops
- [ ] Stage 4: Function calls with parameters
- [ ] Stage 4 extended: Function return values
- [ ] Stage 5: LESS and comparison operators
- [ ] Stage 5 extended: Bitwise operations (AND, OR, XOR)

---

## Files Modified

- ✅ `include/bmlc/codegen/codegen.hpp` - Header (const SymbolTable*)
- ✅ `include/bmlc/codegen/instruction_emitter.hpp` - Header
- ✅ `include/bmlc/codegen/register_pool.hpp` - Header
- ✅ `include/bmlc/codegen/descriptors.hpp` - Header
- ✅ `src/codegen/instruction_emitter.cpp` - Implementation
- ✅ `src/codegen/register_pool.cpp` - Implementation
- ✅ `src/codegen/codegen.cpp` - Implementation + entry point fix
- ✅ `src/main.cpp` - Added CodeGen pipeline integration
- ✅ `CMakeLists.txt` - Added codegen source files

---

## Next Session Actions

1. **Build and test** with `simple.bml`
2. **Fix LESS operator** if needed
3. **Add more test cases** for each stage
4. **Document any issues** found during testing
5. Consider **16-bit support** implementation
