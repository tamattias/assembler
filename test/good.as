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
.data 7 , 8 , 9

; Print hello world 3 times
hello
hello
hello

; Test external string
IgnoredLabel: .extern  ExternalText 
prn ExternalText

; Number at end of line.
prn #42

; Instruction without operands
rts
stop