; Insert a macro before the error statements to make sure that line numbers
; are reported correctly after macro expansion.
macro bogus_macro
    Text inside this macro will not be used.
endm

; Label too long.
THISISAWAYTOOLONGLABELCONSISTINGOFTOOMANYCHARACTERS:

; Label has invalid characters.
UNDERSCORES_ARE_NOT_ALLOWED:

; Empty label
:

; Duplicate label
DUP:
DUP:

; .extern without Label
.extern

; Macro with extraneous text
macro one two
endm

; Instruction doesn't exist
garbage

; Too many operands.
add r1, r2, #3
stop #100

; Too few operands.
add r1
lea r2

; No operands.
sub
prn

; Incorrect index addressing
IndexedAccess:
add IndexedAccess[rtrash], r2
add IndexedAccess[ABCDEFG], r2
add IndexedAccess[5.5], r2
add IndexedAccess[r11], r2
add IndexedAccess[r-2], r2

; Invalid addressing mode
mov r1, #5

; Space between operands
add ABC DEF
add r1 r2
add #1 #2
add r2 #1
add #1 r2

; Bad data
.data 1, 2, 3, 4, hoedjevanpapier
.data 1, 2, 3,

; Bad string
.string 1, 2, 3, 4
.string "unterminated

; Invalid directive
.invalid

