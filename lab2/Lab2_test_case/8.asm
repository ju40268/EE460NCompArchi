;8.asm test br

;.ORIG X3000
;ADD R5, R5, #2
;HERE ADD R5, R5,#-1;
;BRp HERE;
;ADD R1, R1, X1;
;halt
;.END


; XOR
.ORIG X3000
ADD R7, R7, #5 ;101
ADD R2, R2, #8 ;1000
XOR R1, R2, R7 ; 1101
NOT R4, R7; FFFA
halt
.END