;6.asm test JSR JSRR

.ORIG X3000;
ADD R1,R1,#5;
JSR HERE
ADD R3, R3, #2;
HERE ADD R1,R1,#4;


LEA R3, HERE2
JSRR R3;
ADD R3, R3, #2;
HERE2 ADD R1,R1,#4;
halt
.END