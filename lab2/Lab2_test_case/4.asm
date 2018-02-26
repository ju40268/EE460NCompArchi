;4.asm test shf, xor, not,trap

.ORIG X3000;
ADD R7, R7, #-5; R7=-5;
ADD R4, R7, #15;  R4=10;
ADD R4, R4, X3; R4=13
ADD R3, R4, R7; R3=8;
XOR R3, R3, R4; R3=0X0005(0101)
XOR R4, R4, XF; R4=0X0002(0010)
NOT R5, R4;                  R5=0XFFFD    N=1
LSHF R3,R3,#2; R3=32=0X0014;
RSHFA R5,R5,#1;          R5=0XFFFe;    ???
RSHFL R5,R5,#1;  
LSHF R5,R5,#1; 
TRAP X25; R7=PC+2, PC=0x3018
.end