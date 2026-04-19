# BML Compiler
Brendan-Marlow Language Compiler
target -> 8080 assembly

function arguments and return values are stored in fixed memory spaces (starting at 0x1000).
```
[0x1000] ARG 0: 1B
[0x1001] ARG 1: 1B
[0x1002] ARG 2: 1B
[0x1003] ARG 3: 1B

[0x1004] RET 0: 1B
[0x1005] RET 1: 1B

[0x1006+] Global variables
```


Language Grammar
``` 
program -> declaration_list EOF  
  
declaration_list -> declaration declaration_list | ε  
declaration -> var_declaration | func_declaration  
  
var_declaration -> KW_VAR type IDENTIFIER SEMICOLON
  
func_declaration ->  
  KW_FUNC IDENTIFIER L_PAREN param_list R_PAREN ARROW type block  
  
type -> KW_UINT8 | KW_UINT16 | KW_INT8 | KW_INT16 | KW_VOID  
  
param_list -> param param_list_tail | ε  
param_list_tail -> COMMA param param_list_tail | ε  
param -> type IDENTIFIER  
  
block -> L_BRACE statement_list R_BRACE  
statement_list -> statement statement_list | ε  

statement -> assignment_statement 
  | if_statement
  | while_statement 
  | ret_statement  
  
assignment_statement -> IDENTIFIER ASSIGN expression SEMICOLON  

if_statement -> KW_IF L_PAREN expression R_PAREN block  

while_statement -> KW_WHILE L_PAREN expression R_PAREN block  

ret_statement -> KW_RET expression SEMICOLON  

binary_operator -> PLUS | MINUS | LESS | AND | OR | XOR

expression -> operand binary_tail
binary_tail -> binary_operator operand | ε

operand -> NUMBER | IDENTIFIER function_call_tail

function_call_tail -> L_PAREN argument_list R_PAREN | ε

argument_list -> expression argument_list_tail | ε
argument_list_tail -> COMMA expression argument_list_tail | ε
```
