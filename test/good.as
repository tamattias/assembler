; Print text macro
HELLOWORLD: .string "Hello world!"
macro hello
    prn HELLOWORLD
endm

    ; Indented comment

; Empty string
EMPTYSTR: .string ""

; Integer data
SINGLEINT: .data 1234
SOMEDATA:  .data  -100, 55,  -230  
.data 7, 8, 9

; Print hello world 3 times
hello
hello
hello

; Test external string
LabeledExternal: .extern  ExternalText 
prn ExternalText
prn LabeledExternal

; Number at end of line.
prn #42

; Instruction without operands
rts
stop