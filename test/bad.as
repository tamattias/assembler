; Insert a macro before the error statements to make sure that line numbers
; are reported correctly after macro expansion.
macro bogus_macro
    Text inside this macro will not be used.
endm

; Label too long.
THISISAWAYTOOLONGLABELCONSISTINGOFTOOMANYCHARACTERS:

; Label has invalid characters.
UNDERSCORES_ARE_NOT_ALLOWED:

; Macro with extraneous text
macro one two
endm

; Too many operands.
add r1, r2, #3
stop #100

; Too few operands.
add r1
lea r2

; No operands.
sub

