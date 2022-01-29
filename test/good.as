; Print text macro
HELLOWORLD: .string "Hello world!"
macro hello
    prn HELLOWORLD
endm

; Print hello world 3 times
hello
hello
hello

; Instruction without operands
stop