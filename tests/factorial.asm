; Fixed calling convention addresses (reserved)
; ARG 0-3: 0000h-0003h
; RET 0-1: 0004h-0005h

; Global variables:
; acc at 0008h
; i at 000ah
; result at 0006h

; Program entry point
; Initialize stack pointer
LXI SP, 0f00h
JMP main


main:
; Function: main
; Function body:
; Assignment: result = ...
; Function call: factorial
MVI A, 05h
STA 0000h
CALL factorial
LDA 0004h
STA 0006h
; Return statement
MVI A, 00h
STA 0004h
HLT

factorial:
; Function: factorial
; Load parameter 0 from ARG 0
LDA 0000h
STA 000ch
; Function body:
; Assignment: acc = ...
MVI A, 01h
STA 0008h
; Assignment: i = ...
MVI A, 01h
STA 000ah
; While loop
loop_start_0:
LDA 000ah
MOV B, A
LDA 000ch
MOV C, A
MOV A, B
CMP C
JNC not_less_2
MVI A, 01h
JMP less_end_3
not_less_2:
MVI A, 00h
less_end_3:
CPI 00h
JZ loop_end_1
; Assignment: acc = ...
LDA 0008h
MOV B, A
LDA 000ah
ADD B
STA 0008h
; Assignment: i = ...
LDA 000ah
MOV B, A
MVI A, 01h
ADD B
STA 000ah
JMP loop_start_0
loop_end_1:
; Return statement
LDA 0008h
STA 0004h
RET
