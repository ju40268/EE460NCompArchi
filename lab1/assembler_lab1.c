/*
        Name 1: Chia-Ju, Chen
        UTEID 1: cc65542
*/

#include <stdio.h> /* standard input/output library */
#include <stdlib.h> /* Standard C Library */
#include <string.h> /* String operations library */
#include <ctype.h> /* Library for useful character operations */
#include <limits.h> /* Library for definitions of common variable type characteristics */
#include <stdint.h>
int isOpcode(char* op);
int isValidLabel(char* label);
int checkRegFormat(char* reg);
int toNum(char * pStr);
void insertSymbolTable(char* lLabel, int programCounter);
int searchSymbolTable(char* lLabel);
void insertMachineCode(int insValue);

void fillOp(char* lArg1);
void opcodeOp(char* lOpcode, char* lArg1, char* lArg2, char* lArg3, char* lArg4, int programCounter);
int readAndParse(FILE * pInfile, char * pLine, char ** pLabel, char ** pOpcode, char ** pArg1, char ** pArg2, char ** pArg3, char ** pArg4);
void firstPass(char* inputFileName);
void secondPass(char* inputFileName, char* outputFileName);



/* for symbol tabel */
#define MAX_LABEL_LEN 20
#define MAX_SYMBOLS 255
#define MAX_INS 65536 /* MAX_INS #? */
#define MAX_LINE_LENGTH 255
#define INS_LEN 6 /* final format of instruction, e.g. 0xFFFF */
#define BIN_INS_LEN 16 /* original format of instruction, e.g. 1001_1001_1001_1001 */

typedef struct {
  int address;
  char label[MAX_LABEL_LEN + 1];        /* Question for the reader: Why do we need to add 1? */
} TableEntry;

TableEntry symbolTable[MAX_SYMBOLS];

enum{DONE, OK, EMPTY_LINE};
char machineCodeList[MAX_INS][INS_LEN + 1];
int symbolTableEnding = 0;
int machineCodeEnding = 0;

int main(int argc, char* argv[]) {
    char *prgName   = NULL;
    char *iFileName = NULL;
    char *oFileName = NULL;
    prgName   = argv[0];
    iFileName = argv[1];
    oFileName = argv[2];
    firstPass(iFileName);
    secondPass(iFileName, oFileName);
    exit(0);
  }

void firstPass(char* inputFileName){
  char lLine[MAX_LINE_LENGTH + 1], *lLabel, *lOpcode, *lArg1, *lArg2, *lArg3, *lArg4;
  int lRet;
  int programCounter = 0;
  FILE * lInfile;
  lInfile = fopen(inputFileName, "r");  /* open the input file */
 /* for the first pass, we only have to do establishing symbol table.*/
  do{
    lRet = readAndParse( lInfile, lLine, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4 );
     if( lRet != DONE && lRet != EMPTY_LINE ){
        
        if(!strcmp(lOpcode, ".orig")){
          programCounter = toNum(lArg1);
          continue;
        }

        else if(strlen(lLabel) != 0){
          if(isValidLabel(lLabel) && (isOpcode(lOpcode) == 1 || !strcmp(lOpcode, ".fill"))){
            if(searchSymbolTable(lLabel) != -1) exit(4);
            insertSymbolTable(lLabel, programCounter);
            printf("lable:%s is inserted!\n", lLabel);
          }
        }
      /* not invalid then? exit ? */
      programCounter+=2;
    }
  }while( lRet != DONE );
  fclose(lInfile);
}

void secondPass(char* inputFileName, char* outputFileName){
  char lLine[MAX_LINE_LENGTH + 1], *lLabel, *lOpcode, *lArg1, *lArg2, *lArg3, *lArg4;
  int lRet;
  int count = 0;
  int programCounter = 0;
  FILE * lInfile;
  FILE * pOutfile;
  lInfile = fopen(inputFileName, "r");  /* open the input file */
  pOutfile = fopen(outputFileName, "w");
 /* for the first pass, we only have to do establishing symbol table.*/
  do{
    lRet = readAndParse(lInfile, lLine, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4);
    if( lRet != DONE && lRet != EMPTY_LINE ){
      if(count == 0){
        printf("%s\n", lOpcode);
        if(strcmp(lOpcode, ".orig") == 0){
          programCounter = toNum(lArg1);
          if(programCounter % 2 == 0 && programCounter > 0 && programCounter < 65535) fillOp(lArg1);
          else {printf("exit in secondPass error code 3\n");
                  exit(3);
                }
        }else exit(4); /* no specified .ORIG */
      } 
      
      else{
        if(programCounter >= 65534){
          printf("instrution address exceeds lc3b address space, exit at errorcode 4\n");
          exit(4); 
        }
        if(!strcmp(lOpcode, ".orig")) exit(4); /* multiple .orig ?? what about multiple .end*/
        else if(strlen(lLabel) != 0 && isValidLabel(lLabel) && searchSymbolTable(lLabel) != -1){ /* if label is at Arg, do it later */
          if(!strcmp(lOpcode, ".fill")) fillOp(lArg1);
          else if(isOpcode(lOpcode) == 1) opcodeOp(lOpcode, lArg1, lArg2, lArg3, lArg4, programCounter);
          else exit(2); /* no opcode following a label is illegal. */
        }
        else if(!strcmp(lOpcode, ".end")){
          count = -1;
          break;
        }
        else if(!strcmp(lOpcode, ".fill")) fillOp(lArg1);
        else if(isOpcode(lOpcode) == 1) opcodeOp(lOpcode, lArg1, lArg2, lArg3, lArg4, programCounter);
        else {printf("exit at secondPass, error code 2\n");exit(2);} /* invalid opcode */

        programCounter+=2;
      }
      count+=1;
     }
  }while( lRet != DONE );

  if(count != -1) {printf("no .end specified, exit at error code 4\n"); exit(4);}
  int j;
  for(j = 0; j < machineCodeEnding; j++) fprintf(pOutfile, "0x%s\n", machineCodeList[j]);

  fclose(lInfile);
  fclose(pOutfile);
}

int readAndParse(FILE * pInfile, char * pLine, char ** pLabel, char ** pOpcode,
                 char ** pArg1, char ** pArg2, char ** pArg3, char ** pArg4){
  char * lRet, * lPtr;
  int i;
  if( !fgets( pLine, MAX_LINE_LENGTH, pInfile ) )
      return( DONE );

      /* convert entire line to lowercase */
  for( i = 0; i < strlen( pLine ); i++ )
      pLine[i] = tolower( pLine[i] );

  /* set all args to point null terminator of line? */
  *pLabel = *pOpcode = *pArg1 = *pArg2 = *pArg3 = *pArg4 = pLine + strlen(pLine);

  /* ignore the comments */
  lPtr = pLine;
  while( *lPtr != ';' && *lPtr != '\0' && *lPtr != '\n' )
    lPtr++;

  *lPtr = '\0'; /* replace comment or end of line with null terminator */

  if( !(lPtr = strtok( pLine, "\t\n ," ) ) )
    return( EMPTY_LINE );

  if( isOpcode( lPtr ) == -1 && lPtr[0] != '.' ){ /* found a label */
     *pLabel = lPtr;
      if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );
  }

  *pOpcode = lPtr;

  if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

  *pArg1 = lPtr;

  if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

  *pArg2 = lPtr;
  if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

  *pArg3 = lPtr;

  if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

  *pArg4 = lPtr;

  return( OK );
  }

void fillOp(char* lArg1){
  char machineCode[INS_LEN + 1];
  /* constant is valid? */
  int insValue = toNum(lArg1);
  if(insValue < 0 || insValue > 65535) exit(3); /* .FILL constant exceeds the range (0, 0xFFFF) */
  insertMachineCode(insValue);
}

void opcodeOp(char* lOpcode, char* lArg1, char* lArg2, char* lArg3, char* lArg4, int programCounter){
  int dr, sr, sr1, sr2, imm5, n, z, p, pcOffset9, pcOffset11, baseR, bOffset6, offset6, trapVect8, amount4;
  int nzp;
  int insValue;
  int addrOfLabel;
  unsigned int dr_, sr_;
  if(!strcmp(lOpcode, "add")){
    if(!checkRegFormat(lArg1) || !checkRegFormat(lArg2)) exit(4); /* unexpected operands */
    dr = lArg1[1] - '0';
    sr1 = lArg2[1] - '0';
    if(checkRegFormat(lArg3)){
      sr2 = lArg3[1] - '0';
      insValue = (1 & 15) << 12 | (dr & 7) << 9 | (sr1 & 7) << 6 | (sr2 & 7);
      insertMachineCode(insValue);
    }
    else{ /* imm5 */
      imm5 = toNum(lArg3);
      if(imm5 > 15 || imm5 < -16) exit(3); /* invalid constant */
      insValue = (1 & 15) << 12 | (dr & 7) << 9 | (sr1 & 7) << 6 | 1 << 5 | (imm5 & 31); /* correct? */
      insertMachineCode(insValue);
    }
  }

  else if(!strcmp(lOpcode, "lea")){
    if(!checkRegFormat(lArg1)) exit(4);
    dr = lArg1[1] - '0';
    isValidLabel(lArg2);
 /*   addrOfLabel = searchSymbolTable(lArg2); */
    if((addrOfLabel = searchSymbolTable(lArg2)) == -1) exit(1); /* undefined label */
    printf("this lea pc is %04X\n", programCounter);
    pcOffset9 = (addrOfLabel - (programCounter + 2)) >> 1; /* always even */
    printf("%d\n", pcOffset9);
    if(pcOffset9 > 255 || pcOffset9 < -256) exit(4); /* too far to set offset */
    insValue = (14 & 15) << 12 | (dr & 7) << 9 | (pcOffset9 & 511);
    insertMachineCode(insValue);
  }

  else if(!strcmp(lOpcode, "ldw")){
    if(!checkRegFormat(lArg1) || !checkRegFormat(lArg2)) exit(4);
    dr = lArg1[1] - '0';
    baseR = lArg2[1] - '0';
    offset6 = toNum(lArg3);
    if(offset6 > 31 || offset6 < -32) exit(3); /*invalid constant*/
    insValue = (6 & 15) << 12 | (dr & 7) << 9 | (baseR & 7) << 6 | (offset6 & 63);
    insertMachineCode(insValue);
  }

 else if(!strcmp(lOpcode, "trap")){
    if(lArg1[0] != 'x') {printf("trap vector is not a hex value, exit at error code 4\n"); exit(4);}
    trapVect8 = toNum(lArg1);
    if(trapVect8 > 255 || trapVect8 < 0) exit(3); /*invalid constants */
    insValue = (15 & 15) << 12 | (0 & 15) << 8 | trapVect8;
    insertMachineCode(insValue);
  }

  else if(lOpcode[0] == 'b' && lOpcode[1] == 'r'){
    isValidLabel(lArg1);
    if(!strcmp(lOpcode, "brn")) nzp = 4;
    else if(!strcmp(lOpcode, "brz")) nzp = 2;
    else if(!strcmp(lOpcode, "brp")) nzp = 1;
    else if(!strcmp(lOpcode, "br") || !strcmp(lOpcode, "brnzp")) nzp = 7;
    else if(!strcmp(lOpcode, "brzp")) nzp = 3;
    else if(!strcmp(lOpcode, "brnp")) nzp = 5;
    else if(!strcmp(lOpcode, "brnz")) nzp = 6;
 /*   addrOfLabel = searchSymbolTable(lArg1); */
    if((addrOfLabel = searchSymbolTable(lArg1)) == -1) exit(1); /* undefined label */
    pcOffset9 = (addrOfLabel - (programCounter + 2)) >> 1; /* always even */
    printf("%d\n", pcOffset9);
    if(pcOffset9 > 255 || pcOffset9 < -256) exit(4); /* too far to set offset */
    insValue = (0 & 15) << 12 | (nzp & 7) << 9 | (pcOffset9 & 511);
    insertMachineCode(insValue);  
  }

  else if(!strcmp(lOpcode, "halt")){
    if(strlen(lArg1) != 0) {printf("halt has operand, exit at error code 4\n"); exit(4);} /* unexpected operand */
    insValue = (15 & 15) << 12 | (0 & 15) << 8 | 37;
    printf("halt ins is 0x%04X\n", insValue);
    insertMachineCode(insValue);
  }

  else if(!strcmp(lOpcode, "and")){
    if(!checkRegFormat(lArg1) || !checkRegFormat(lArg2)) exit(4); /* unexpected operands */
    dr = lArg1[1] - '0';
    sr1 = lArg2[1] - '0';
    printf("ins and dr is %d sr1 is %d\n", dr, sr1);
    if(checkRegFormat(lArg3)){
      sr2 = lArg3[1] - '0';
      insValue = (5 & 15) << 12 | (dr & 7) << 9 | (sr1 & 7) << 6 | (sr2 & 7);
      insertMachineCode(insValue);
    }
    else{ /* imm5 */
      imm5 = toNum(lArg3);
      if(imm5 > 15 || imm5 < -16) exit(3); /* invalid constant */
      insValue = (5 & 15) << 12 | (dr & 7) << 9 | (sr1 & 7) << 6 | 1 << 5 | (imm5 & 31); /* correct? */
      insertMachineCode(insValue);
    }   
  }

  else if(!strcmp(lOpcode, "jmp")){
    if(!checkRegFormat(lArg1)) exit(4); /* unexpected operands */
    baseR = lArg1[1] - '0';
    insValue = (12 & 15) << 12 | (0 & 7) << 9 | (baseR & 7) << 6 | 0;
    insertMachineCode(insValue); 
  }

  else if(!strcmp(lOpcode, "ret")){
    if(strlen(lArg1) != 0) {printf("ret has operand, exit at error code 4\n"); exit(4);} 
    insValue = (12 & 15) << 12 | (0 & 7) << 9 | (7 & 7) << 6 | 0;
    insertMachineCode(insValue);
  }

  else if(!strcmp(lOpcode, "jsr")){
/*    addrOfLabel = searchSymbolTable(lArg1); */
    isValidLabel(lArg1);
    if((addrOfLabel = searchSymbolTable(lArg1)) == -1) exit(1); /* undefined label */
    pcOffset11 = (addrOfLabel - (programCounter + 2)) >> 1; /* always even */
    printf("%d\n", pcOffset11);
    if(pcOffset11 > 1023 || pcOffset11 < -1024) exit(4); /* too far to set offset */
    insValue = (4 & 15) << 12 | (1 & 1) << 11 | (pcOffset11 & 2047);
    insertMachineCode(insValue);      
  }

  else if(!strcmp(lOpcode, "jsrr")){
    if(!checkRegFormat(lArg1)) exit(4);
    baseR = lArg1[1] - '0';
    insValue = (4 & 15) << 12 | (0 & 1) << 9 | (baseR & 7) << 6 | 0;
    insertMachineCode(insValue);
  }

  else if(!strcmp(lOpcode, "ldb")){
    if(!checkRegFormat(lArg1) || !checkRegFormat(lArg2)) exit(4);
    dr = lArg1[1] - '0';
    baseR = lArg2[1] - '0';
    offset6 = toNum(lArg3);
    if(offset6 > 31 || offset6 < -32) exit(3); /*invalid constant*/
    insValue = (2 & 15) << 12 | (dr & 7) << 9 | (baseR & 7) << 6 | (offset6 & 63);
    insertMachineCode(insValue);
  }

  else if(!strcmp(lOpcode, "not")){
    if(!checkRegFormat(lArg1) || !checkRegFormat(lArg2)) exit(4);
      dr = lArg1[1] - '0';
      sr = lArg2[1] - '0';
      printf("'0' is %d, lArg1[1] is %d, lArg2[1] is %d\n", '0', lArg1[1], lArg2[1]);
      printf("ins not dr is %d sr is %d\n", dr, sr);
      insValue = (9 & 15) << 12 | (dr & 7) << 9 | (sr & 7) << 6 | (63 & 63);
      insertMachineCode(insValue);
  }

  else if(!strcmp(lOpcode, "nop")){
    if(strlen(lArg1) != 0) {printf("nop has operand, exit at error code 4\n"); exit(4);} 
    insertMachineCode(0);
  } 

  else if(!strcmp(lOpcode, "rti")){
    if(strlen(lArg1) != 0) {printf("rti has operand, exit at error code 4\n"); exit(4);} 
    insValue = (8 & 15) << 12 | 0;
    insertMachineCode(insValue);
  } 

  else if(lOpcode[1] == 's' && lOpcode[2] == 'h' && lOpcode[3] == 'f'){
    if(!checkRegFormat(lArg1) || !checkRegFormat(lArg2)) exit(4);
    dr = lArg1[1] - '0';
    sr = lArg2[1] - '0';
    amount4 = toNum(lArg3);
    if(amount4 > 15 || amount4 < 0) exit(3);
    if(!strcmp(lOpcode, "lshf")) insValue = (13 & 15) << 12 | (dr & 7) << 9 | (sr & 7) << 6 | 0 << 4 | amount4;
    else if(!strcmp(lOpcode, "rshfl")) insValue =  (13 & 15) << 12 | (dr & 7) << 9 | (sr & 7) << 6 | 1 << 4 | amount4;
    else if(!strcmp(lOpcode, "rshfa")) insValue =  (13 & 15) << 12 | (dr & 7) << 9 | (sr & 7) << 6 | 3 << 4 | amount4;
    insertMachineCode(insValue);
  }

  else if(!strcmp(lOpcode, "stb")){
    if(!checkRegFormat(lArg1) || !checkRegFormat(lArg2)) exit(4);
    sr = lArg1[1] - '0';
    baseR = lArg2[1] - '0';
    bOffset6 = toNum(lArg3);
    if(bOffset6 > 31 || bOffset6 < -32) exit(3);
    insValue = (3 & 15) << 12 | (sr & 7) << 9 | (baseR & 7) << 6 | (bOffset6 & 63);
    insertMachineCode(insValue);
  }

  else if(!strcmp(lOpcode, "stw")){
    if(!checkRegFormat(lArg1) || !checkRegFormat(lArg2)) exit(4);
    sr = lArg1[1] - '0';
    baseR = lArg2[1] - '0';
    offset6 = toNum(lArg3);
    if(offset6 > 31 || offset6 < -32) exit(3);
    insValue = (7 & 15) << 12 | (sr & 7) << 9 | (baseR & 7) << 6 | (offset6 & 63);
    insertMachineCode(insValue);
  }

  else if(!strcmp(lOpcode, "xor")){
    if(!checkRegFormat(lArg1) || !checkRegFormat(lArg2)) exit(4); /* unexpected operands */
    dr = lArg1[1] - '0';
    sr1 = lArg2[1] - '0';
    if(checkRegFormat(lArg3)){
      sr2 = lArg3[1] - '0';
      insValue = (9 & 15) << 12 | (dr & 7) << 9 | (sr1 & 7) << 6 | (0 & 7) << 3 | (sr2 & 7);
      insertMachineCode(insValue);
    }
    else{ /* imm5 */
      imm5 = toNum(lArg3);
      if(imm5 > 15 || imm5 < -16) exit(3); /* invalid constant */
      insValue = (9 & 15) << 12 | (dr & 7) << 9 | (sr1 & 7) << 6 | 1 << 5 | (imm5 & 31); /* correct? */
      insertMachineCode(insValue);
    }   
  }

  else {printf("exit at opcodeOp, error code 2\n");exit(2);} /* invalid opcode */

}

int isOpcode(char* op){
  if(!strcmp(op, "add") || !strcmp(op, "and") || !strcmp(op, "br") ||
           !strcmp(op, "brn") || !strcmp(op, "brz") || !strcmp(op, "brp") ||
           !strcmp(op, "brnz") || !strcmp(op, "brnp") || !strcmp(op, "brzp") ||
           !strcmp(op, "brnzp") || !strcmp(op, "halt") || !strcmp(op, "jmp") || !strcmp(op, "jsr") ||
           !strcmp(op, "jsrr") || !strcmp(op, "ldb") || !strcmp(op, "ldw") ||
           !strcmp(op, "lea") || !strcmp(op, "not") || !strcmp(op, "ret") ||
           !strcmp(op, "rti") || !strcmp(op, "lshf") || !strcmp(op, "rshfl") ||
           !strcmp(op, "rshfa") || !strcmp(op, "stb") || !strcmp(op, "stw") ||
           !strcmp(op, "trap") || !strcmp(op, "xor") || !strcmp(op, "nop"))
            return 1;
  else return -1;
}

int isValidLabel(char* label) {
  /*may need double check the error code might contain? */
  int i = 0; 

  if (!isalpha(label[0]) || label[0] == 'x') {
    printf("label doesn't start with letter other than x, exit at error code 4\n");
    exit(4);
  }

  else if (!strcmp(label, "in") || !strcmp(label, "out") ||
      !strcmp(label, "getc") || !strcmp(label, "puts")) {
    printf("label is IN/OUT/GETC/PUTS, exit at error code 4\n");
    exit(4);
  }
  else if(isOpcode(label) == 1 || !strcmp(label, ".orig") || !strcmp(label, ".end") || !strcmp(label, ".fill") || checkRegFormat(label) == 1){
    printf("label is opcode/pseudo code/register, exit at error code 4\n");
    exit(4);
  }

  while (label[i]) {
    if (!isalnum(label[i]) || i > 20) {
      printf("label isn't alphanumeric/exceeds 20 characters, exit at error code 4\n");
      exit(4);
    }
    i++;
  }
  return 1;
}

int checkRegFormat(char* reg){
  if(!strcmp(reg, "r0") || !strcmp(reg, "r1") || !strcmp(reg, "r2") || !strcmp(reg, "r3")
        || !strcmp(reg, "r4") || !strcmp(reg, "r5") || !strcmp(reg, "r6") || !strcmp(reg, "r7"))
            return 1;
  return 0;
}

void insertSymbolTable(char* lLabel, int programCounter){
    if(symbolTableEnding >= MAX_SYMBOLS) exit(4);
    strcpy(symbolTable[symbolTableEnding].label, lLabel);
    symbolTable[symbolTableEnding].address = programCounter;
    symbolTableEnding++;
}


int searchSymbolTable(char* lLabel){
  int i;
  for(i = 0; i < symbolTableEnding; i++){
   if(strcmp(lLabel, symbolTable[i].label) == 0)
     return symbolTable[i].address;     
  }
  return -1;
}

void insertMachineCode(int insValue){
  char ins[BIN_INS_LEN + 1];
  sprintf(ins, "%04X", insValue);
  strcpy(machineCodeList[machineCodeEnding], ins);
  machineCodeEnding++;
}

int toNum( char * pStr ){
   char * t_ptr;
   char * orig_pStr;
   int t_length,k;
   int lNum, lNeg = 0;
   long int lNumLong;

   orig_pStr = pStr;
   if( *pStr == '#' ){				/* decimal */
     pStr++;
     if( *pStr == '-' ){				/* dec is negative */
       lNeg = 1;
       pStr++;
     }
     t_ptr = pStr;
     t_length = strlen(t_ptr);
     for(k=0;k < t_length;k++){
       if (!isdigit(*t_ptr)){
	 		 	printf("Error: invalid decimal operand, %s\n",orig_pStr);
	 			exit(4);
       }
       t_ptr++;
     }
     lNum = atoi(pStr);
     if (lNeg)
       lNum = -lNum;

     return lNum;
   }else if( *pStr == 'x' ){	/* hex     */
     pStr++;
     if( *pStr == '-' ){				/* hex is negative */
       lNeg = 1;
       pStr++;
     }
     t_ptr = pStr;
     t_length = strlen(t_ptr);
     for(k=0;k < t_length;k++){
       if (!isxdigit(*t_ptr)){
	 		 	printf("Error: invalid hex operand, %s\n",orig_pStr);
	 			exit(4);
       }
       t_ptr++;
     }
     lNumLong = strtol(pStr, NULL, 16);    /* convert hex string into integer */
     lNum = (lNumLong > INT_MAX)? INT_MAX : lNumLong;
     if( lNeg )
       lNum = -lNum;
     return lNum;
   }else{
		 	printf( "Error: invalid operand, %s\n", orig_pStr);
			exit(4);  /* This has been changed from error code 3 to error code 4, see clarification 12 */
   }
}
