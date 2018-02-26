
/*
     REFER TO THE SUBMISSION INSTRUCTION FOR DETAILS
 
        Name 1: Chia Ju, Chen
        UTEID 1: cc65542
 */

/***************************************************************/
/*                                                             */
/*   LC-3b Instruction Level Simulator                         */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3b machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
 *   MEMORY[A][1] stores the most significant byte of word at word address A
 *   */

#define WORDS_IN_MEM    0x08000
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;    /* run bit */


typedef struct System_Latches_Struct{

 int PC,        /* program counter */
   N,        /* n condition bit */
   Z,        /* z condition bit */
   P;        /* p condition bit */
 int REGS[LC_3b_REGS]; /* register file. */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {
 printf("----------------LC-3b ISIM Help-----------------------\n");
 printf("go               -  run program to completion         \n");
 printf("run n            -  execute program for n instructions\n");
 printf("mdump low high   -  dump memory from low to high      \n");
 printf("rdump            -  dump the register & bus values    \n");
 printf("?                -  display this help menu            \n");
 printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {

 process_instruction();
 CURRENT_LATCHES = NEXT_LATCHES;
 INSTRUCTION_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {
 int i;

 if (RUN_BIT == FALSE) {
   printf("Can't simulate, Simulator is halted\n\n");
   return;
 }

 printf("Simulating for %d cycles...\n\n", num_cycles);
 for (i = 0; i < num_cycles; i++) {
   if (CURRENT_LATCHES.PC == 0x0000) {
       RUN_BIT = FALSE;
       printf("Simulator halted\n\n");
       break;
   }
   cycle();
 }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed                 */
/*                                                             */
/***************************************************************/
void go() {
 if (RUN_BIT == FALSE) {
   printf("Can't simulate, Simulator is halted\n\n");
   return;
 }

 printf("Simulating...\n\n");
 while (CURRENT_LATCHES.PC != 0x0000)
   cycle();
 RUN_BIT = FALSE;
 printf("Simulator halted\n\n");
}

/***************************************************************/ 
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {
 int address; /* this is a byte address */

 printf("\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
 printf("-------------------------------------\n");
 for (address = (start >> 1); address <= (stop >> 1); address++)
   printf("  0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
 printf("\n");

 /* dump the memory contents into the dumpsim file */
 fprintf(dumpsim_file, "\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
 fprintf(dumpsim_file, "-------------------------------------\n");
 for (address = (start >> 1); address <= (stop >> 1); address++)
   fprintf(dumpsim_file, " 0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
 fprintf(dumpsim_file, "\n");
 fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {
 int k;

 printf("\nCurrent register/bus values :\n");
 printf("-------------------------------------\n");
 printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
 printf("PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
 printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
 printf("Registers:\n");
 for (k = 0; k < LC_3b_REGS; k++)
   printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
 printf("\n");

 /* dump the state information into the dumpsim file */
 fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
 fprintf(dumpsim_file, "-------------------------------------\n");
 fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
 fprintf(dumpsim_file, "PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
 fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
 fprintf(dumpsim_file, "Registers:\n");
 for (k = 0; k < LC_3b_REGS; k++)
   fprintf(dumpsim_file, "%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
 fprintf(dumpsim_file, "\n");
 fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {
 char buffer[20];
 int start, stop, cycles;

 printf("LC-3b-SIM> ");

 scanf("%s", buffer);
 printf("\n");

 switch(buffer[0]) {
 case 'G':
 case 'g':
   go();
   break;

 case 'M':
 case 'm':
   scanf("%i %i", &start, &stop);
   mdump(dumpsim_file, start, stop);
   break;

 case '?':
   help();
   break;
 case 'Q':
 case 'q':
   printf("Bye.\n");
   exit(0);

 case 'R':
 case 'r':
   if (buffer[1] == 'd' || buffer[1] == 'D')
       rdump(dumpsim_file);
   else {
       scanf("%d", &cycles);
       run(cycles);
   }
   break;

 default:
   printf("Invalid Command\n");
   break;
 }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {
 int i;

 for (i=0; i < WORDS_IN_MEM; i++) {
   MEMORY[i][0] = 0;
   MEMORY[i][1] = 0;
 }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {
 FILE * prog;
 int ii, word, program_base;

 /* Open program file. */
 prog = fopen(program_filename, "r");
 if (prog == NULL) {
   printf("Error: Can't open program file %s\n", program_filename);
   exit(-1);
 }

 /* Read in the program. */
 if (fscanf(prog, "%x\n", &word) != EOF)
   program_base = word >> 1;
 else {
   printf("Error: Program file is empty\n");
   exit(-1);
 }

 ii = 0;
 while (fscanf(prog, "%x\n", &word) != EOF) {
   /* Make sure it fits. */
   if (program_base + ii >= WORDS_IN_MEM) {
       printf("Error: Program file %s is too long to fit in memory. %x\n",
            program_filename, ii);
       exit(-1);
   }

   /* Write the word to memory array. */
   MEMORY[program_base + ii][0] = word & 0x00FF;
   MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
   ii++;
 }

 if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

 printf("Read %d words from program into memory.\n\n", ii);
}

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) { 
 int i;

 init_memory();
 for ( i = 0; i < num_prog_files; i++ ) {
   load_program(program_filename);
   while(*program_filename++ != '\0');
 }
 CURRENT_LATCHES.Z = 1;
 NEXT_LATCHES = CURRENT_LATCHES;

 RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {
 FILE * dumpsim_file;

 /* Error Checking */
 if (argc < 2) {
   printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
          argv[0]);
   exit(1);
 }

 printf("LC-3b Simulator\n\n");

 initialize(argv[1], argc - 1);

 if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
   printf("Error: Can't open dumpsim file\n");
   exit(-1);
 }

 while (1)
   get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code.
  You are allowed to use the following global variables in your
  code. These are defined above.
 
   MEMORY
 
   CURRENT_LATCHES
   NEXT_LATCHES
 
  You may define your own local/global variables and functions.
  You may use the functions to get at the control bits defined
  above.
 
  Begin your code here                        */

#include "math.h"

#define mask01 0x01
#define mask05 0x01F
#define mask06 0x03F
#define mask09 0x01FF
#define mask11 0x7FF

int getBit(int n,int ir) {
  int bitN;
  return (ir>>n)&mask01 ;
}

int getBitRange(int from,int to, int ir) {
	int mask, tFrom, tTo;
	tFrom = from;
	tTo = to ;
	if(from > to) {
		tFrom = to;
		tTo = from ;
	}
	mask = pow(2,tTo-tFrom+1)-1 ;
	return (ir>>tFrom)&mask ;
}

int signExtension(int n, signed int Num) {
	signed int res;
	int ns = 32 - n;
	res = (Num<<ns)>>ns ;
	return res;
}
void checkOpcode(int ir,int opcode){
  int tOpcode = getBitRange(12,15,ir);
  if(tOpcode != opcode) exit(1);
}

void setCC(int dr) {
	if (NEXT_LATCHES.REGS[dr]==0)  {
        NEXT_LATCHES.N=0;
        NEXT_LATCHES.Z=1;
        NEXT_LATCHES.P=0;
	} else if (getBitRange(15,15,NEXT_LATCHES.REGS[dr])==0)  {
        NEXT_LATCHES.N=0;
        NEXT_LATCHES.Z=0;
        NEXT_LATCHES.P=1;
	} else if(getBitRange(15,15,NEXT_LATCHES.REGS[dr])==1){
        NEXT_LATCHES.N=1;
        NEXT_LATCHES.Z=0;
        NEXT_LATCHES.P=0;
	}
}

void br(int ir){

    int pcOffset9,n,z,p,N,Z,P,opcode;
    checkOpcode(ir,0);
    pcOffset9 = getBitRange(0,8,ir);

    n = getBit(11,ir);
    z = getBit(10,ir);
    p = getBit(9,ir);

    N = CURRENT_LATCHES.N;
    Z = CURRENT_LATCHES.Z;
    P = CURRENT_LATCHES.P;

    pcOffset9  = signExtension(9,pcOffset9) << 1; 
    if((n&&N) || (z&&Z) || (p&&P))
    	NEXT_LATCHES.PC=Low16bits(NEXT_LATCHES.PC+pcOffset9);
}

void  add(int ir){

    int dr,sr1,sr2,imm5,opcode;
    signed int temp;
    checkOpcode(ir,1);
    dr   = getBitRange(9,11,ir);
    imm5 = getBitRange(0,4,ir);
    sr1  = getBitRange(6,8,ir);
    sr2  = getBitRange(0,2,ir);
    if(getBit(5,ir)) {
      NEXT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr1] + signExtension(5,imm5));
    } else {
      NEXT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr1] + CURRENT_LATCHES.REGS[sr2]);
    }
    setCC(dr);
}

void ldb(int ir) {
    int dr, baseR,bOffset6,addr, memIndex;
    checkOpcode(ir,2);    
    baseR    = getBitRange(6,8,ir);
    bOffset6 = getBitRange(0,5,ir);
    dr       = getBitRange(9,11,ir);
    addr     = Low16bits(CURRENT_LATCHES.REGS[baseR] + signExtension(6,bOffset6));
    memIndex = (addr >> 1);
    if(addr%2==0) {
    	NEXT_LATCHES.REGS[dr] = Low16bits(signExtension(8,getBitRange(0,7,MEMORY[memIndex][0])));
    } else {
    	NEXT_LATCHES.REGS[dr] = Low16bits(signExtension(8,getBitRange(0,7,MEMORY[memIndex][1])));
    }
    setCC(dr);
}
void stb(int ir){

	    int sr, baseR,bOffset6,addr, memIndex,regVal;
	    checkOpcode(ir,3);
	    sr       = getBitRange(9,11,ir);
	    baseR    = getBitRange(6,8,ir);
	    bOffset6 = getBitRange(0,5,ir);
	    addr     = Low16bits(CURRENT_LATCHES.REGS[baseR] + signExtension(6,bOffset6));
	    memIndex = (addr >> 1);

	    regVal   = getBitRange(0,7,CURRENT_LATCHES.REGS[sr]);
	    if(addr%2==0) {
	    	MEMORY[memIndex][0] = regVal;
	    } else {
	    	MEMORY[memIndex][1] = regVal;
	    }
}

void 	jsr(int ir){

	int currentPC, pcOffset11, baseR;
	checkOpcode(ir,4);

	pcOffset11 = getBitRange(0,10,ir);
	baseR      = getBitRange(6,8,ir);

	pcOffset11 = signExtension(11,pcOffset11)<<1;

	NEXT_LATCHES.REGS[7] = Low16bits(NEXT_LATCHES.PC);

	if(getBit(11,ir)) {
		/*JSR*/
		NEXT_LATCHES.PC = Low16bits(NEXT_LATCHES.PC + pcOffset11);
	} else {
		/*JSRR*/
		NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.REGS[baseR]) ;
	}

}

void	and(int ir){
    int dr,sr1,sr2,imm5,opcode;

    printf("Info： In func：and : ir==%x\n",ir);
    checkOpcode(ir,5);

    dr   = getBitRange(9,11,ir);
    sr1  = getBitRange(6,8,ir);
    sr2  = getBitRange(0,2,ir);
    imm5 = getBitRange(0,4,ir);

    if(getBit(5,ir)) {
      NEXT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr1] & signExtension(5,imm5));
    } else {
      NEXT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr1] & CURRENT_LATCHES.REGS[sr2]);
    }

    setCC(dr) ;

}
void ldw(int ir){

    int dr, baseR,bOffset6,addr, memIndex;

    printf("Info： In func： ldw  : ir==%x\n",ir);
    checkOpcode(ir,6);

    dr       = getBitRange(9,11,ir);
    baseR    = getBitRange(6,8,ir);
    bOffset6 = getBitRange(0,5,ir);
    bOffset6 = signExtension(6,bOffset6) << 1;
    addr     = Low16bits(CURRENT_LATCHES.REGS[baseR] + bOffset6);
    memIndex = addr >> 1;

    if(addr%2==0) {
    	NEXT_LATCHES.REGS[dr] = getBitRange(0,7,MEMORY[memIndex][1]);
    	NEXT_LATCHES.REGS[dr] = Low16bits((NEXT_LATCHES.REGS[dr]<<8) | getBitRange(0,7,MEMORY[memIndex][0]));
    } else {
    	printf("Error： In func： ldw : address 0x%x is odd for a word align memory!\n",addr);
    }
    setCC(dr);
}

void stw(int ir){

	    int sr, baseR,bOffset6,addr, memIndex,regVal;
	    checkOpcode(ir,7);
	    sr       = getBitRange(9,11,ir);
	    baseR    = getBitRange(6,8,ir);
	    bOffset6 = getBitRange(0,5,ir);
      bOffset6 = signExtension(6,bOffset6) << 1;
      addr     = Low16bits(CURRENT_LATCHES.REGS[baseR] + bOffset6);
	    memIndex = (addr >> 1);

	    regVal   = Low16bits(CURRENT_LATCHES.REGS[sr]);
	    if(addr%2==0) {
	    	MEMORY[memIndex][0] = getBitRange(0,7,regVal) ;
	    	MEMORY[memIndex][1] = getBitRange(8,15,regVal);
	    } else {
	    	printf("Error： In func： stw : address 0x%x is odd for a word align memory!\n",addr);
	    }
}

void	xor(int ir){
    int dr,sr1,sr2,imm5,opcode;
    checkOpcode(ir,9);
    dr   = getBitRange(9,11,ir);
    sr1  = getBitRange(6,8,ir);
    sr2  = getBitRange(0,2,ir);
    imm5 = getBitRange(0,4,ir);

    if(getBit(5,ir)) {
      NEXT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr1] ^ signExtension(5,imm5));
    } else {
      NEXT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr1] ^ CURRENT_LATCHES.REGS[sr2]);
    }
    setCC(dr);
}
void jmp(int ir){
	int baseR;
	checkOpcode(ir,12);
  baseR = getBitRange(6,8,ir);
  NEXT_LATCHES.PC=Low16bits(CURRENT_LATCHES.REGS[baseR]);
}
void  shf(int ir){

    int dr,sr,amount4,type;
    checkOpcode(ir,13);

    dr      = getBitRange(9,11,ir);
    sr      = getBitRange(6,8,ir);
    amount4 = getBitRange(0,3,ir);
    type    = getBitRange(4,5,ir);


    if(type==0){
        NEXT_LATCHES.REGS[dr]=Low16bits(CURRENT_LATCHES.REGS[sr]<<amount4);
    } else if(type==1) {
        NEXT_LATCHES.REGS[dr]=Low16bits(CURRENT_LATCHES.REGS[sr]>>amount4);
    } else if(type==3) {
        NEXT_LATCHES.REGS[dr]=Low16bits((CURRENT_LATCHES.REGS[sr]<<16)>>(amount4+16));
    }
    setCC(dr) ;
}

void 	lea(int ir){
    int dr,pcOffset9;
    checkOpcode(ir,14);
    dr        = getBitRange(9,11,ir);
    pcOffset9 = getBitRange(0,8,ir);
    pcOffset9 = signExtension(9,pcOffset9)<<1;
    NEXT_LATCHES.REGS[dr] = Low16bits(NEXT_LATCHES.PC + pcOffset9);

}
void trap(int ir){
    int trapvect8,memIndex;
    checkOpcode(ir,15);
    trapvect8 = getBitRange(0,7,ir) << 1;
    memIndex  = trapvect8 >> 1 ;
    NEXT_LATCHES.REGS[7]=Low16bits(NEXT_LATCHES.PC);
    NEXT_LATCHES.PC = getBitRange(0,7,MEMORY[memIndex][1]);
    NEXT_LATCHES.PC = Low16bits((NEXT_LATCHES.PC <<8) | getBitRange(0,7,MEMORY[memIndex][0]));
}

/****************************************/

void process_instruction(){
/*  function: process_instruction
   
    Process one instruction at a time
       -Fetch one instruction
       -Decode
       -Execute
       -Update NEXT_LATCHES
 */
/****************************************/

    int state,memIndex;
    int irLow, ir;

    memIndex = CURRENT_LATCHES.PC >> 1 ;
    ir = MEMORY[memIndex][1];
    irLow = MEMORY[memIndex][0];
    ir = Low16bits((ir<<8)|irLow);
    NEXT_LATCHES.PC = CURRENT_LATCHES.PC + 2;

    state = ir >> 12;
    switch (state){
       case 0:	br(ir);   break;
       case 1:	add(ir);  break;
       case 2:	ldb(ir);  break;
       case 3:	stb(ir);  break;
       case 4:	jsr(ir);  break;
       case 5:	and(ir);  break;
       case 6:	ldw(ir);  break;
       case 7:	stw(ir);  break;
       case 9:	xor(ir);  break;
       case 12: jmp(ir);  break;
       case 13:	shf(ir);  break;
       case 14:	lea(ir);  break;
       case 15:	trap(ir); break;
    }
    return;
}
