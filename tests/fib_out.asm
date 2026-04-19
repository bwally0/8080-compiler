; Fixed calling convention addresses (reserved)
; ARG 0-3: 1000h-1003h
; RET 0-1: 1004h-1005h

; Global variables:
; curr at 1008h
; i at 100ch
; prev at 1006h
; temp at 100ah

; Program entry point
; Initialize stack pointer
LXI SP, 0f00h
JMP main


main:
; Function: main
; Function body:
; Assignment: prev = ...
MVI A, 00h
STA 1006h
; Assignment: curr = ...
MVI A, 01h
STA 1008h
; Assignment: i = ...
MVI A, 00h
STA 100ch
; While loop
loop_start_0:
LDA 100ch
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
LDA 1006h
OUT 01h
; Assignment: temp = ...
LDA 1006h
MOV B, A
LDA 1008h
ADD B
STA 100ah
; Assignment: prev = ...
LDA 1008h
STA 1006h
; Assignment: curr = ...
LDA 100ah
STA 1008h
; Assignment: i = ...
LDA 100ch
MOV B, A
MVI A, 01h
ADD B
STA 100ch
JMP loop_start_0
loop_end_1:
MVI A, ffh
OUT 02h
; Return statement
MVI A, 00h
STA 1004h
HLT
