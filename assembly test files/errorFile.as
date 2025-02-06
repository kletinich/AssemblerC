jmp A(#5,#6)
mov B, r6
C: inc C
.extern D
bne D(r4,#4)
dec E
stop
.entry F
.entry C
.entry C
jmp (r5,r4)

