;7.asm test jmp

.ORIG x3000;
LEA R4, HERE;
JMP R4;
ADD R3, R3,#4;
HERE ADD R1,R1,#5;
halt
.END