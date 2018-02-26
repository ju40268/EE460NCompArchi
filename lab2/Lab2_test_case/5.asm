;5.asm

.orig x3000
ADD R0, R0, #0;
NOT R0, R0;
ADD R0, R0,#-2; 
RSHFL R1, R0, #1;
halt
.END