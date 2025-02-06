.extern A
.extern B
.extern C
.entry D
.entry E
D: .data 1,2,3
E: .string "hello"
mov #5 , A
jmp E(A,B)
inc D
inc A
add D, A
stop
