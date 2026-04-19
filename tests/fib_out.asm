; Fixed calling convention addresses (reserved)
; ARG 0-3: 0000h-0003h
; RET 0-1: 0004h-0005h

; Global variables:
; curr at 0008h
; i at 000ch
; prev at 0006h
; temp at 000ah

; Program entry point
; Initialize stack pointer
LXI SP, 0f00h
JMP main


main:
; Function: main
; Function body:
; Assignment: prev = ...
MVI A, 00h
STA 0006h
; Assignment: curr = ...
MVI A, 01h
STA 0008h
; Assignment: i = ...
MVI A, 00h
STA 000ch
; While loop
loop_start_0:
LDA 000ch
MOV B, A
MVI A, 0ah
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
LDA 0006h
OUT 01h
; Assignment: temp = ...
LDA 0006h
MOV B, A
LDA 0008h
ADD B
STA 000ah
; Assignment: prev = ...
LDA 0008h
STA 0006h
; Assignment: curr = ...
LDA 000ah
STA 0008h
; Assignment: i = ...
LDA 000ch
MOV B, A
MVI A, 01h
ADD B
STA 000ch
JMP loop_start_0
loop_end_1:
MVI A, ffh
OUT 02h
; Return statement
MVI A, 00h
STA 0004h
HLT
