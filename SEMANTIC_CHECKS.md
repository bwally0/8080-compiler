# Semantic Checks for BML Compiler (Intel 8080 Target)

This document outlines semantic checks that should be performed on the AST before code generation to ensure correctness and compliance with the 8080 architecture and BML language specifications.

## 1. Symbol Table & Scope Checks

### 1.1 Undefined Variable Usage
- **Check**: Every variable referenced in expressions or assignments must be previously declared
- **Error**: "Undefined variable 'x' at line N"
- **Applies to**: Assignment statements, expressions, return statements
- **Example violation**: `n = undefined_var + 1;`

### 1.2 Undefined Function Calls
- **Check**: Every function call must reference a declared function
- **Error**: "Undefined function 'foo()' at line N"
- **Applies to**: Function call expressions
- **Example violation**: `result = nonexistent_func();`

### 1.3 Variable Redeclaration
- **Check**: Variables cannot be declared more than once in the same scope
- **Error**: "Variable 'x' already declared at line M"
- **Applies to**: Global variable declarations
- **Note**: For now, assume single global scope (no nested scopes)

### 1.4 Function Redeclaration
- **Check**: Functions cannot be declared more than once
- **Error**: "Function 'foo' already declared at line M"
- **Applies to**: Function declarations

## 2. Type Checking

### 2.1 Assignment Type Compatibility
- **Check**: The right-hand side of an assignment must be compatible with the variable's declared type
- **Error**: "Type mismatch in assignment: cannot assign 'uint16' to 'uint8' at line N"
- **Applies to**: Assignment statements
- **Note**: Consider implicit narrowing conversions (may want to warn or error)

### 2.2 Binary Operation Type Compatibility
- **Check**: Both operands in a binary operation must be of compatible types
- **Error**: "Type mismatch in binary operation: 'uint8' + 'uint16' at line N"
- **Special rules**:
  - Arithmetic ops (PLUS, MINUS): Both operands should be integer types
  - Comparison ops (LESS): Both operands should be integer types
  - Bitwise ops (AND, OR, XOR): Both operands should be integer types
- **Applies to**: All binary expressions

### 2.3 Return Type Compatibility
- **Check**: The expression in a return statement must match the function's declared return type
- **Error**: "Return type mismatch: function expects 'uint8' but got 'uint16' at line N"
- **Applies to**: Return statements within functions
- **Note**: VOID return type should have no expression (or empty return)

### 2.4 Function Argument Type Matching
- **Check**: Arguments passed to a function must match the declared parameter types
- **Error**: "Argument 1 type mismatch: expected 'uint16' but got 'uint8' at line N"
- **Applies to**: Function call expressions
- **Strictness**: Can be strict (exact match) or allow implicit promotions

### 2.5 All Code Paths Return (Functions with non-void return)
- **Check**: All code paths in a function with a non-void return type must have a return statement
- **Error**: "Function 'foo' must return a value on all code paths"
- **Applies to**: Function declarations with non-void return types
- **Complexity**: Within if/while blocks, ensure terminal return or guaranteed loop exit

## 3. Function Call Checks

### 3.1 Function Argument Count
- **Check**: Number of arguments in a function call must match the function's parameter count
- **Error**: "Function 'foo' expects 2 arguments but got 1 at line N"
- **Applies to**: Function call expressions

### 3.2 Calling Convention Constraints
- **Check**: Function arguments don't exceed fixed memory allocation (ARG 0-3: 1 byte each)
- **Error**: "Function has more than 4 arguments, exceeds calling convention (8080 only supports 4 ARG slots)"
- **Applies to**: Function declarations
- **Note**: Each parameter currently takes 1 byte, so max 4 parameters (or adjust if multi-byte params allowed)

### 3.3 Return Value Slots
- **Check**: Functions with large return types (e.g., uint16) don't exceed return value slots (RET 0-1: 1 byte each)
- **Error**: "Return type 'uint16' requires 2 bytes but only 2 return slots available in calling convention"
- **Applies to**: Function declarations
- **Note**: uint16 might need 2 slots; uint8/int8 need 1 slot

## 4. Literal Value Checks

### 4.1 Literal Overflow
- **Check**: Numeric literals must fit within the range of their inferred or assigned type
- **Error**: "Literal 256 does not fit in type 'uint8' (range 0-255) at line N"
- **Applies to**: Integer literals in assignments and expressions
- **Type ranges**:
  - uint8: 0-255
  - uint16: 0-65535
  - int8: -128-127
  - int16: -32768-32767

### 4.2 Negative Literals with Unsigned Types
- **Check**: Negative literals should not be assigned to unsigned types without explicit handling
- **Warning/Error**: "Negative literal assigned to unsigned type 'uint8' at line N"
- **Applies to**: Assignments and expressions with unsigned types

## 5. Control Flow Checks

### 5.1 Unreachable Code After Return
- **Check**: Code after a return statement in a block is unreachable
- **Warning**: "Unreachable code after return statement at line N"
- **Applies to**: Statements following return statements in the same block

### 5.2 Infinite Loops (Optional)
- **Check**: Detect obvious infinite loops (e.g., `while (1)` or `while (true)`)
- **Warning**: "Infinite loop detected at line N" (only if loop has no exits)
- **Note**: This is optional; may be intentional for event loops

### 5.3 Empty Loop Bodies
- **Check**: While loops with empty bodies might be unintentional
- **Warning**: "While loop has empty body at line N"
- **Applies to**: While statements with no statements in body

## 6. Expression & Operator Checks

### 6.1 Operator Validity for Type
- **Check**: Certain operators might not be valid for certain types on the 8080
- **Error**: "Operator 'MOD' not supported for type 'uint8' on 8080"
- **Note**: Depends on which operators are supported; adjust if more operators are added

### 6.2 Division by Zero (Static Detection)
- **Check**: Detect constant expressions that divide by zero
- **Warning**: "Division by zero at line N"
- **Applies to**: Binary operations with DIVIDE operator and constant 0
- **Note**: Only for obvious cases; dynamic detection at runtime is needed

## 7. Variable Usage Checks

### 7.1 Unused Variables
- **Check**: Variables declared but never used
- **Warning**: "Unused variable 'x' at line N"
- **Applies to**: Global and local variables

### 7.2 Unused Functions
- **Check**: Functions declared but never called
- **Warning**: "Unused function 'foo' at line N"
- **Applies to**: Top-level functions
- **Exception**: `main()` function should be considered "used"

## 8. 8080 Architecture-Specific Checks

### 8.1 Register Allocation Feasibility
- **Check**: Total variable allocation doesn't exceed 8080 register and memory limits
- **Warning/Error**: "Too many variables declared; memory constraints may be exceeded"
- **Note**: Calculate total bytes needed and compare against available memory

### 8.2 Instruction Operand Constraints
- **Check**: Expressions don't create intermediate values too large for registers
- **Example**: `(uint16_a + uint16_b) + uint16_c` might overflow without careful allocation
- **Note**: This is a code-generation concern but could be flagged during semantic analysis

### 8.3 Pointer/Address Generation Warnings
- **Check**: If address-of or pointer operations are added later, validate their use
- **Note**: Placeholder for future features

## 9. Language Feature Checks

### 9.1 Main Function Requirement
- **Check**: Compiler should have a `main()` function with return type `uint8`
- **Error**: "No main() function defined" or "main() must return uint8"
- **Applies to**: Program-level validation

### 9.2 No Main Function Entry Point (Optional)
- **Check**: At least one function should be defined (for compilation to be meaningful)
- **Warning**: "No functions defined in program"

## 10. Scope & Lifetime Checks

### 10.1 Variable Initialization Tracking
- **Check**: Flag usage of uninitialized variables
- **Warning**: "Variable 'x' might be uninitialized before use at line N"
- **Applies to**: Variables in assignments and expressions
- **Complexity**: Requires data-flow analysis

### 10.2 Function Parameter Scope
- **Check**: Ensure function parameters are available within function body
- **Check**: Ensure function parameters don't shadow global variables (or warn if they do)
- **Applies to**: Statements in function bodies

## Execution Order

Suggested order for semantic checking:

1. **Build symbol table**: Collect all global variable and function declarations
2. **Check declarations**: Verify no redeclarations
3. **Check main function exists**: Error if missing
4. **Visit each function and statement**:
   - Check variable references are defined
   - Check function calls reference defined functions
   - Check type compatibility
   - Check argument counts
   - Check return type compatibility
5. **Graph traversal (optional)**:
   - Detect unreachable code
   - Check all code paths return
   - Warn about unused variables/functions

## Implementation Notes

- Store all symbol information (name, type, location) for error messages
- Use source location info for helpful error messages with line/column numbers
- Consider implementing a `SemanticAnalyzer` class with visitor pattern for AST traversal
- Create specific error and warning types for different check categories
- Consider a "strict mode" vs "warnings only" flag for optional checks
