; Fixed calling convention addresses (reserved)
; ARG 0-3: 0000h-0003h
; RET 0-1: 0004h-0005h

; Global variables:
; f1 at 0008h
; f2 at 000ah
; i at 000eh
; n at 0006h
; r1 at 000ch

; Program entry point
; Initialize stack pointer
LXI SP, 0f00h
JMP main


main:
; Function: main
; Function body:
; Assignment: n = ...
; Function call: fibonacci
MVI A, 0ah
STA 0000h
CALL fibonacci
LDA 0004h
STA 0006h
; Return statement
MVI A, 00h
STA 0004h
HLT

fibonacci:
; Function: fibonacci
; Load parameter 0 from ARG 0
LDA 0000h
STA 0010h
; Function body:
; If statement
LDA 0010h
MOV B, A
MVI A, 02h
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
JZ else_0
; Return statement
LDA 0010h
STA 0004h
RET
JMP endif_1
else_0:
endif_1:
; Assignment: f1 = ...
MVI A, 00h
STA 0008h
; Assignment: f2 = ...
MVI A, 01h
STA 000ah
; Assignment: i = ...
MVI A, 01h
STA 000eh
; While loop
loop_start_4:
LDA 000eh
MOV B, A
LDA 0010h
MOV C, A
MOV A, B
CMP C
JNC not_less_6
MVI A, 01h
JMP less_end_7
not_less_6:
MVI A, 00h
less_end_7:
CPI 00h
JZ loop_end_5
; Assignment: r1 = ...
LDA 0008h
MOV B, A
LDA 000ah
ADD B
STA 000ch
; Assignment: f1 = ...
LDA 000ah
STA 0008h
; Assignment: f2 = ...
LDA 000ch
STA 000ah
; Assignment: i = ...
LDA 000eh
MOV B, A
MVI A, 01h
ADD B
STA 000eh
JMP loop_start_4
loop_end_5:
; Return statement
LDA 000ah
STA 0004h
RET
