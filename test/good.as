; Print text macro
HELLOWORLD: .string "Hello world!"
macro hello
    prn HELLOWORLD
endm

; Print hello world 3 times
hello
hello
hello

; Number at end of line.
prn #42

; Instruction without operands
stop