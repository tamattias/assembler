; Print text macro
HELLOWORLD: .string "Hello world!"
macro hello
    prn HELLOWORLD
endm

; Empty string
EMPTYSTR: .string ""

; Integer data
SINGLEINT: .data 1234
SOMEDATA:  .data  -100, 55,  -230  

; Print hello world 3 times
hello
hello
hello

; Number at end of line.
prn #42

; Instruction without operands
stop