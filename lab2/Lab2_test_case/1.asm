;1.asm

        .ORIG x3000


;R0 store the source string
;R1 store the destination string
;R2 is the bridge of two string


        LEA R0, SRC;
        LDW R0,R0,#0;R0=X4000
        LDW R0,R0,#0;
        LEA R1, DES;

        LDW R1,R1,#0;

START   LDW R2, R0, X0;
        ADD R0, R0,X2;
        STB R2, R1, X0;
        ADD R1, R1,#1;
        ADD R2, R2,#0;

        BRNP START; if is /0,it's the end of the string,STOP
        HALT;

SRC     .FILL X4000;
DES     .FILL X4002;
.END