.ORIG x2000
AND R0 R0 #0
ADD R2 R0 xf
LSHF R2 R2 #4
ADD R2 R2 xf
LEA R1 LABEL
LDW R1 R1 #0
STW R2 R1 #0
LDB R4 R1 #0
LABEL .FILL x4000
HALT

.END