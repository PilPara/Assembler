.entry LIST
.extern W

; Duplicated labels
MAIN: add r3, LIST
MAIN: add r4, LIST

; Illegal labels
1LABEL: prn #5
L@BEL: prn #5
data: lea W, r6
string: prn #5
extern: prn #5
entry: prn #5
r6: stop
main: prn #5

; Bad comma usage
mov, r1, r2
mov r3,,r4
mov r7 r0
mov r7, r1,
,mov r7, r2

; Operand/addressing errors
mov r3, #5
mov #4, #5
mov &LABEL, r2
cmp
cmp r1, r2, r3
cmp #5
cmp r3, #STR
add r1
add r1, r2, r3
add r1, #5
sub #5, r1
lea
lea LABEL, r1, r2
lea #5, r3
lea r3, r1

clr
clr r1, r2
clr #5

not
not #4
not r1, r2

inc
inc r1, r2
inc LABEL[r3]

jmp
jmp r1
jmp LABEL, r2

bne
bne r2
bne #10
bne LABEL, LABEL

jsr r1, r2

red
red #5

prn LABEL[r3]
prn LABEL, r3

rts r3
stop r4

; Data/strings
STR: .string "abcd" "efgh"
LIST: .data 6, -9, +17, , 12
.data 16777216
.data -16777217
STR3: .string abcdf"
STR5: .string  ,"abcd"
