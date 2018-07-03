// Name: Lin Aung
// DATE: May 7, 2014
// File: vcom.c
// Course: CENG 606
// Lab#: 10 (FINAL REPORT)
/* Description: This is the final stage of the program. The purpose of this project was to program a virtual cpu that will 
 *              run like a cpu would.  It will add, move, shift, load, pull, branch, etc... depending on what the
 *              instructions register(IR) is processed.
*/	        	

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

//Defining the names globally//
#define MAXSIZE 16384
#define MAXNAME 100
#define COLUMNS 16
#define HBYTE_BITS 4
#define BYTE_BITS 8
#define BYTE2_BITS 16
#define BYTE3_BITS 24
#define WORD_BITS 32
#define MEMMAX 0x4000	//max memory size
#define MEMMAR 0x3FFF	//memory mask
#define MEM (memory[MAR & MEMMAR])	//masked memory address
#define MAXREG 16
#define MAX32 0xFFFFFFFF	//Max 32 bit
#define MSB32	0x80000000	//most significant bit
#define LSB32	0x00000001	//least significant bit
#define registerCount 8
#define highRegister 8
#define WORD_BYTE 4



//Defining functions at the top so it won't give a warning.//
int load (void *memory, unsigned int max);
int iscarry(unsigned long op1,unsigned long op2, unsigned C);
void displayReg(void);
void printUsage(void);
void execute(void);
void go(void);
void IM_mov(void);
void IM_add(void);
void IM_sub(void);
void IM_cmp(void);
void set_NZFlag(void);
void LS_LoadBW(void);
void LS_StoreBW(void);
void DataP_add(void);
void DataP_sub(void);
void set_relAddress(void);
void set_BitShift(void);
void set_ROTATE(void);
void set_PUSH(void);
void set_PULL(void);

unsigned char memory[MAXSIZE];
FILE *fp;
unsigned char fileName[MAXNAME];
unsigned long size = 0;



//creating a global structure that contains all the registers//
//Edited in Lab 8.//
struct Registers
{
  unsigned long R[MAXREG];
  unsigned short IR;
  unsigned long MBR;
  unsigned long MAR;
  unsigned long ALU;
  unsigned short NF;
  unsigned short CF;
  unsigned short ZF;	
} reg; 
#define SP reg.R[13]
#define LR reg.R[14]
#define PC reg.R[15]
#define IR reg.IR
#define MBR reg.MBR
#define MAR reg.MAR
#define ALU reg.ALU
#define NF reg.NF
#define CF reg.CF
#define ZF reg.ZF

// Defining the operation's parameter
#define RD (IR & 0x000F)
#define RN ((IR & 0x00F0)>> HBYTE_BITS)
#define OPC (IR & 0xF000)
#define STP  0xD000

// Defining Opcode and variables for Immediate Instructions
//#define Immediate 0x4000
#define IM_OPC (IR & 0x7000) //declaring the OPC for immediate.
#define IM_VALUE ((IR & 0x0FF0)>> HBYTE_BITS) //declaring the immediate value
#define IM_MOV 0x4000 //declaring the move instruction
#define IM_CMP 0x5000 //declaring the compare instruction
#define IM_ADD 0x6000 //declaring the add instruction
#define IM_SUB 0x7000 //declaring the sub instruction
#define IM_STP 0xD000 //declaring the stop instruction

// Defining Opcode and variables for Load/Store Instructions
#define LoadStore 0x2000
#define LS_OPC (IR & 0xF800)
#define LS_STORE 0x2000
#define LS_LOAD 0x2800
#define LS_RD = (IR & 0x000F)
#define LS_RN = ((IR & 0x00F0) >> BYTE_BITS)
#define LS_BW ((IR & 0x0400) >> 10)

// Defining Opcode and variables for Data Processing Instructions
#define DataPROC 0x0000
#define DP_OPC (IR & 0xFF00)
#define DP_AND 0x0000 //declaring the Data Processing's AND instruction
#define DP_EOR 0x0100 //declaring the Data Processing's EOR instruction
#define DP_SUB 0x0200 //declaring the Data Processing's SUB instruction
#define DP_SXB 0x0300 //declaring the Data Processing's SXB instruction
#define DP_ADD 0x0400 //declaring the Data Processing's ADD instruction
#define DP_ADC 0x0500 //declaring the Data Processing's ADC instruction
#define DP_LSR 0x0600 //declaring the Data Processing's LSR instruction
#define DP_LSL 0x0700 //declaring the Data Processing's LSL instruction
#define DP_TST 0x0800 //declaring the Data Processing's TST instruction
#define DP_TEQ 0x0900 //declaring the Data Processing's TEQ instruction
#define DP_CMP 0x0A00 //declaring the Data Processing's CMP instruction
#define DP_ROR 0x0B00 //declaring the Data Processing's ROR instruction
#define DP_ORR 0x0C00 //declaring the Data Processing's ORR instruction
#define DP_MOV 0x0D00 //declaring the Data Processing's MOV instruction
#define DP_BIC 0x0E00 //declaring the Data Processing's BIC instruction
#define DP_MVN 0x0F00 //declaring the Data Processing's MVN instruction


// Defining Condtional Branch Instructions.
#define ConBRANCH 0xC000
#define CB_OPC (IR & 0x0F00) //declaring the condition of Conditional Branch
#define CB_ADDRESS (IR & 0x00FF) //declaring the 8bit relative address 
#define CB_EQ 0x0000 //declaring the Conditional Branch Z set instruction
#define CB_NE 0x0100 //declaring the Conditional Branch Z clear instruction
#define CB_CS 0x0200 //declaring the Conditional Branch C set instruction
#define CB_CC 0x0300 //declaring the Conditional Branch C clear instruction
#define CB_MI 0x0400 //declaring the Conditional Branch N set instruction
#define CB_PL 0x0500 //declaring the Conditional Branch N clear instruction
#define CB_HI 0x0800 //declaring the Conditional Branch C set Z claer instruction
#define CB_LS 0x0900 //declaring the Conditional Branch C clear Z set instruction
#define CB_AL 0x0E00 //declaring the Conditional Branch Ignored instruction

// Defining Unconditional Branch Instructions.
#define UB_OFFSET (IR & 0x0FFF) //declaring the offset of unconditional branch.
#define UB_L0	0xE000 //declaring the Unconditional Branch with Branch Normal
#define UB_L1	0xF000 //declaring the Unconditional Branch with Branch Link.

// Defining Push/Pull Instructions.
#define PSH_PUL 0xB000 //declaring the push pull instruction
#define PSHPUL_OPC (IR & 0x0400) //declaring the condition at LH of push/pull
#define PSHPUL_COND (IR & 0x0800)
#define PSHPUL_EXTRA (IR & 0x0100)
#define PUSH 0x0000 //declaring the push condition
#define PULL 0x0800 //declaring the pull condition
#define PSHPUL_REG (IR & 0x00FF) //declaring the registers for push/pull

#define PSH_LOW 0x0000 //declaring the push low registers condition
#define PSH_HIGH 0x0400 //declaring the push high registers condition
#define PSH_LR 0x0100 //declaring the push LR condition.

#define PUL_LOW 0x0000 //declaring the pull low registers condition
#define PUL_HIGH 0x0400 //declaring the pull high registers condition
#define PUL_PC 0x0100 //declaring the pull PC condition

/***Function printUsage()***/
/* A function that show a help menu to how to use the proram
 *
 */
void printUsage()
{
  printf(
   "-------------------------------------------------------------\n"
   "|                      MENU SCREEN                          |\n"
   "|          -d (to dump memory)                              |\n"
   "|          -g (to run the entire program)                   |\n"
   "|          -l (to load a file into memory)                  |\n"
   "|          -q (to quit the program)                         |\n"
   "|          -r (to display registers)                        |\n"
   "|          -t (to execute one instruction at a time)        |\n"
   "|          -z (to reset all registers)                      |\n"
   "|          -h (display list of commands)                    |\n"
   "-------------------------------------------------------------\n");
}

/***Function dump()***/
/* This function will dump the memory according to the offset and
 * legnth defined by the user.
 */
void dump(unsigned char *memory, unsigned int offset, unsigned int length)
{
   
   	unsigned int rows = 0;
   	unsigned int leftover = 0;
   	unsigned int total = 0;
   	unsigned int count = 0;
   	unsigned int j = 0;
   	unsigned int k = 0;
   	unsigned int x = 0;
	unsigned int y = 0;

	k = offset;
	j = offset;
	rows = length / COLUMNS;
	leftover = length % COLUMNS;
	total = offset + length;


	while(count<rows)
	{
		if(offset >= MAXSIZE) //return to prompt if the offset go over the maximum size.
		{return;}
   		printf("%04x  ", offset);   //print out the offset
		for(x=0;x<COLUMNS;x++)
		{
     			printf("%02hhX  ",memory[j]);
     			j++;
		}
		printf("\n");
     		printf("     ");
		for(y=0;y<COLUMNS;y++)
		{
    			if(!isspace(memory[k]) && isprint(memory[k]))
              		{
                    		printf("%2c  ", memory[k]);  //prints out the contents in ascii
				k++;
              		}
              	else    //prints unprintable characters as "."
              	{
                    		printf(" .  ");
				k++;

              	}
		}
		printf("\n");
		offset = offset+COLUMNS;
		count++;
	}
	if(count<=rows && leftover>0)
	{
   		printf("%04x  ", j);   //print out the offset
		for(x=0;x<leftover;x++)
		{
     			printf("%02hhX  ",memory[j]);
     			j++;
		}
		printf("\n");
     		printf("     ");

		for(y=0;y<leftover;y++)
		{
 			if(!isspace(memory[k]) && isprint(memory[k]))
             		{
                   		printf("%2c  ", memory[k]);  //prints out the contents in ascii
				k++;
             		}
             		else   //prints unprintable characters as "."
             		{
                    		printf(" .  ");
				k++;

             		}
       	}
		printf("\n");
	}
  	printf("\n");
}

/***Function load()***/
/*Function load to load the file into a buffer.
 */
int load (void *memory, unsigned int max)
{
        memset(memory, 0, max);


       printf("Please enter a file name:\n"); //asking user to input the file name
       fscanf(stdin, "%s", fileName);
       fgetc(stdin);
       printf("filename: %s\n", fileName);
       fp = fopen(fileName, "rb");  //opening the file for reading

	if(fp == NULL)
       {
		printf("*******************WARNING*******************\n");
       		perror("------------Cannot open the file-------------\n");
       		printf("-----Please enter the correct file name.-----\n");
     	}
	else
	{
	      	size = fread(memory, 1, max, fp);
		printf("The number of bytes read in from file in hex: %x\n", size);	
       		printf("The number of bytes read in from file in decimal: %d\n", size);
	

     	}
	fclose(fp);
        	return size; //returning the size of the file back to main
}

void clear (void)
{    
  while ( getchar() != '\n' );
}

/***Function fetch()***/
/* This function will get an instruction from memory and excutes it.
 * It will set the next 16 bit instruction in MBR using PC.
 * IR is used for storing the fetched instruction.
 */
void fetch(void)
{
	MAR = PC++;			//increase the PC and store it into MAR
	MBR = (unsigned long) MEM;	//store the MEM into MBR
	MBR <<= BYTE_BITS;		//shift the MBR a byte(8bit) to the left.
	MAR = PC++;			//increase the PC and store it into MAR
	MBR = MBR |(unsigned long) MEM; //compare the MBR and MEM and store it into MBR.
	IR = (unsigned short) MBR; //store the MBR into IR.
	execute();	//call the function execute() to check out immediate instruction.
}

/***Function displayReg()***/
/* This funtion will display the registers.
 */
void displayReg(void)
{
	printf("-----------------------REGISTERS LIST------------------------\n");
	printf("R0: %08lX\t", reg.R[0]);
  	printf("R1: %08lX\t", reg.R[1]);
  	printf("R2: %08lX\t", reg.R[2]);
  	printf("R3: %08lX\t\n", reg.R[3]);
  	printf("R4: %08lX\t", reg.R[4]);
  	printf("R5: %08lX\t", reg.R[5]);
  	printf("R6: %08lX\t", reg.R[6]);
  	printf("R7: %08lX\t\n", reg.R[7]);
  	printf("R8: %08lX\t", reg.R[8]);
  	printf("R9: %08lX\t", reg.R[9]);
  	printf("R10: %08lX\t", reg.R[10]);
  	printf("R11: %08lX\t\n", reg.R[11]);
  	printf("R12: %08lX\t", reg.R[12]);
	printf("SP: %08lX\t", reg.R[13]);
	printf("LR: %08lX\t", reg.R[14]);
  	printf("PC: %08lX\t\n", reg.R[15]);
  	printf("MBR: %08lX\t", MBR);
  	printf("MAR: %08lX\t", MAR);
  	printf("ALU: %08lX\t", ALU);
  	printf("IR: %04X\t\n", IR);
	printf("NF: %04X\t", NF);
	printf("CF: %04X\t", CF);
	printf("ZF: %04X\t\n", ZF);
	printf("------------------------------------------------------------\n");

}

//***Function resetReg()***//
/* This function will reset all the registers to zero.
 */
void resetReg(void)
{
	memset(&reg, 0, sizeof(reg));   //reset the structure reg
	displayReg();			    //call the function displayReg to display the registers.
}

/***Function go()***/
/* The go function will executes each instruction one after another 
 * until it comes across a STP instruction
 */
void go(void)
{

	printf("You have selected the GO option\n");
	if(IR == STP)
	{
		printf("************************WARNING*************************\n");
		printf("-------You are currently at the end of the program.-----\n");
		printf("------------As you can see the IR is at D000.-----------\n");
		printf("-Please reset the registers to start the program again.-\n");
		printf("------------Enter 'z' to reset the registers------------\n\n");
		displayReg();	
	}
	if(fp == NULL)
	{
		printf("*******************WARNING*******************\n");
		printf("----------There is no file loaded!!!---------\n");
		printf("-----------Enter 'l' to load a file----------\n");
	}
	else
	{
		while(IR != STP)   	//keeps executing the trace function until a STP instruction is reached
		{
   			fetch();
			if(IR == STP)
			{
				printf("STP INSTURCTION DETECTED\n");
				displayReg();			    //call the function displayReg to display the registers.
			}
		}
	}
}

/***Function execute()***/
/* The execute function will look at the opcode of bits in 
 * the IR register to determine which instruction to execute.
 */
void execute(void)
{
	//checking to see which function to execcute
   	switch(OPC)
	{
		case IM_MOV:  IM_mov();	//immediate move
			      set_NZFlag();
              		      break;        	
		case IM_CMP:  IM_cmp();	//immediate compare
                              CF = iscarry(reg.R[RD], ~IM_VALUE, 1);
			      set_NZFlag();
             		      break;
         	case IM_ADD:  IM_add();	//immediate add
                              CF = iscarry(reg.R[RD], IM_VALUE, 0);
			      set_NZFlag();
              		      break;
         	case IM_SUB:  IM_sub();	//immediate subtract
                              CF = iscarry(reg.R[RD], ~IM_VALUE, 1);
			      set_NZFlag();
			      break;			  
		case LoadStore:
			 	 switch(LS_OPC)
			  	 {
			 	 	case LS_LOAD:	 LS_LoadBW();  //load instruction		
             		 	 			 break;
			 	 	case LS_STORE: LS_StoreBW(); //store instruction
	             	        			 break;
			  	 }
				 break;
		//Dataprocessing Case
		case DataPROC:
				 //Data Processing instruction
			 	 switch(DP_OPC)
			  	 {
			 	 	case DP_AND:  //DATA and instruction
							ALU = reg.R[RD] & reg.R[RN]; 
							reg.R[RD] = ALU;
							set_NZFlag();
             		 	 			break;
			 	 	case DP_EOR:  //DATA eor instruction
							ALU = reg.R[RD] ^ reg.R[RN]; 
							ALU = reg.R[RD];
							set_NZFlag();
             		 	 			break;
			 	 	case DP_SUB:  //DATA subtract instruction
							DataP_sub();
							CF = iscarry(reg.R[RD], ~reg.R[RN], 1);
							set_NZFlag();
             		 	 			break;
			 	 	case DP_SXB:  //DATA signed extend byte instruction
							ALU = (signed char)reg.R[RN];
							reg.R[RD] = ALU;
							set_NZFlag();
             		 	 			break;
			 	 	case DP_ADD: 	//DATA add instrcution
							DataP_add();
							CF = iscarry(reg.R[RD], reg.R[RN], 1);
							set_NZFlag();
             		 	 	     		break;
			 	 	case DP_ADC:  //DATA add with carry instruction
							CF = iscarry(reg.R[RD], reg.R[RN], 1);
							ALU = reg.R[RD] + reg.R[RN] + CF;
							reg.R[RD] = ALU;
							set_NZFlag();
             		 	 			break;
			 	 	case DP_LSR: 	//DATA bitshift right instruction
							set_BitShift();
							set_NZFlag();
             		 	 			break;
			 	 	case DP_LSL:	//DATA bitshift left instruction
							set_BitShift();
							set_NZFlag();
             		 	 			break;
			 	 	case DP_TST:  //DATA testbits instruction
							ALU = reg.R[RD] & reg.R[RN];
							set_NZFlag();
             		 	 			break;
			 	 	case DP_TEQ:  //DATA test equivalence instruction
							ALU = reg.R[RD] | reg.R[RN];
							set_NZFlag();
             		 	 			break;
			 	 	case DP_CMP:  //DATA compare instruction
							ALU = reg.R[RD] + ~(reg.R[RN]) + 1;
							set_NZFlag();
             		 	 			break;
			 	 	case DP_ROR: 	//DATA rotate instruction
							set_ROTATE();
							set_NZFlag();
             		 	 			break;
			 	 	case DP_ORR:  //DATA or instruction
							ALU = reg.R[RD] | reg.R[RN];
							reg.R[RD] = ALU;
							set_NZFlag();
             		 	 			break;
			 	 	case DP_MOV:  //DATA move instruction
							ALU = reg.R[RN];
							reg.R[RD] = ALU;
							set_NZFlag();
             		 	 			break;
			 	 	case DP_BIC:  //DATA bit clear instruction
							ALU = reg.R[RD] & ~(reg.R[RN]);
							set_NZFlag();
             		 	 			break;
			 	 	case DP_MVN:  //DATA move not instruction
							ALU = ~reg.R[RN];
							reg.R[RD] = ALU;
							set_NZFlag();
             		 	 			break;
			  	 }
				 break;
		//Conditional Branch Case
		case ConBRANCH:
			switch(CB_OPC)
			{
				case CB_EQ:	if(ZF == 1)	{set_relAddress();}		
						break;
				case CB_NE:	if(ZF == 0)	{set_relAddress();}		
						break;
				case CB_CS:	if(CF == 1)	{set_relAddress();}		
						break;
				case CB_CC:	if(CF == 0)	{set_relAddress();}		
						break;
				case CB_MI:	if(NF == 1)	{set_relAddress();}		
						break;
				case CB_PL:	if(NF == 0)	{set_relAddress();}		
						break;
				case CB_HI:	if((CF == 1) && (ZF == 0))	{set_relAddress();}
						break;
				case CB_LS:	if((CF == 0) && (ZF == 1))	{set_relAddress();}		
						break;
				case CB_AL:	set_relAddress();	
						break;
				}
				break;
		//Uncoditional Branch Normally.
		case UB_L0:	PC = UB_OFFSET;
				break;
		//Uncoditional Branch with Link.
		case UB_L1:	LR = PC;
				PC = UB_OFFSET;
				break;
		case PSH_PUL:
			switch(PSHPUL_COND)
			{
				case PUSH:	//PUSH instruction	
						set_PUSH();
						break;
				case PULL:	//PULL instruction
						set_PULL();
						break;
			}
	}
}



/***Function IM_mov()***/
/* The mov function will move the contents stored in the immediate value
 * and store it into specified register according to the IM_RD.
 */
void IM_mov(void)
{
	//storing the value from immediate value and adding it into the target register
	ALU = IM_VALUE;
	reg.R[RD] = ALU;
}

/***Function IM_add()***/
/* The add function will add the contents stored in the immediate value
 * and add it into specified register according to the IM_RD.
 */
void IM_add(void)
{
	//getting the value from immediate value and adding it into the target register
	ALU = reg.R[RD] + IM_VALUE;
	reg.R[RD] = ALU;
}

/***Function IM_sub()***/
/* The sub function will subtract the contents stored in the immediate value
 * and subtract it from the specified register according to the IM_RD.
 */
void IM_sub(void)
{
	//getting the value from immediate value and subtracting it into the target register
	ALU = reg.R[RD]+ (~IM_VALUE) + 1;
	reg.R[RD] = ALU;
}

/***Function IM_cmp()***/
/* the cmp function simply compare the contents stored in the immediate value and
 * the value from specifed register. To compare it the immediate value will subtract
 * from the register value and if it equal to zero they are the same. If it's not zero
 * then it's not the same value.
 */
void IM_cmp(void)
{
	//getting the value from immediate value and comparing it to the target register
	ALU = reg.R[RD] + (~IM_VALUE) + 1;
}

/***Function set_NZFlag()***/
/* the set_NZFlag function will set the flags depending on the ALU value.
 */
void set_NZFlag(void)
{
	ZF = (ALU == 0) ? 1: 0;
	NF = (((ALU & MSB32) >> 31) == 1) ? 1: 0; // checkfor ALU
}

/***Function LS_LoadBW()***/
/* this function will load either a byte or a word from memory to register
 */
void LS_LoadBW()
{	
	signed int count;
	MAR = reg.R[RN];	// RN: memory location.
	MBR = (unsigned long) MEM;
  	switch(LS_BW)
	{
		case 0: //Load Word instruction
			 for(count = 0; count < 3; count++)		//do it 3 times so it will load 1 byte each time.
			 {
				MBR <<= BYTE_BITS;			//shift the MBR 8bits to the left
				++MAR;						//look at the next address
				MBR |= MEM;
			 }
			 reg.R[RD] = MBR;				//load the MBR into register.
			 break;
		case 1: //Load Byte instruction
			 reg.R[RD] = MBR;					//load a byte of MBR into register.
			 break;
	}
}

/***Function LS_StoreBW()***/
/* this function will store either a byte or a word from register to memory.
 */
void LS_StoreBW()
{
	signed int wcount;

	MAR = reg.R[RN];	// RN: memory location.
	MBR = reg.R[RD];	// RD: the destination register.
  	switch(LS_BW)
	{
		case 0: //Store Word instruction
	   		 for(wcount = 3; wcount >= 0; wcount--)	//do it 4 times so it will store 1 byte each time
			 {
			 	MEM = (unsigned char) (MBR >> (BYTE_BITS * wcount));	//storing it to mem by looking at MBR 8 bits each time starting form the left
				++MAR;       						//look at the next address
              	 }
			 break;
		case 1: //Store Byte instruction
			 MEM = (unsigned char) MBR;					//store a byte of MBR into the memory
			 break;
	}
}

/*
  iscarry()- determine if carry is generated by addition: op1+op2+C
  C can only have value of 1 or 0.
*/ 
int iscarry(unsigned long op1,unsigned long op2, unsigned C)
{

   if ((op2== MAX32)&&(C==1)) 
      return(1); // special case where op2 is at MAX32
   return((op1 > (MAX32 - op2 - C))?1:0);
}

/***Function DataP_add()***/
/* the DataP_add function simply the same as immediate immediate add
 * but instead of the immediate values it uses the registers to add.
 */
void DataP_add()
{
	ALU = reg.R[RD] + reg.R[RN];
	reg.R[RD] = ALU;
}

/***Function DataP_sub()***/
/* the DataP_sub function simply the same as immediate immediate subtract
 * but instead of the immediate values it uses the registers to subtract.
 */
void DataP_sub()
{
	ALU = reg.R[RD]+ (~reg.R[RN]) + 1;
	reg.R[RD] = ALU;
}

/***Function set_relAddress()***/
/* this function will set the PC to offset of conditional branch 
 * 8 bit value is defined and branch to that location.
 */
void set_relAddress()
{
	PC = PC + (signed char)CB_ADDRESS;
}

/***Function set_BitShift()***/
/* this function will shfit the bits in the registers either left or right
 * depedning on the condition. Also it will put the least significant bit 
 * when shifting right and most significant bit when shifting left.
 */
void set_BitShift()
{

	unsigned int shiftCounter;

	shiftCounter = reg.R[RN];	//to see how many times to shift.
	ALU = reg.R[RD];		//storeing the reigster's values into ALU
	switch(DP_OPC)
	{
		case DP_LSR:					
			while(shiftCounter  > 0)
			{
				CF = (ALU & LSB32);	//storing the LSB to the carry flag.
				ALU >>= 1;		//shift one to the right.
				shiftCounter--;
			}
			break;
		case DP_LSL:	
			while(shiftCounter > 0)
			{
				CF = (ALU & MSB32);	//storeing the MSB to the carry flag.
				ALU <<= 1;		//shift one to the left.
				shiftCounter--;
			}
			break;
	}
	reg.R[RD] = ALU;	//store the end result into the target register.
}

/***Function set_ROTATE()***/
/* this function will rotate the bits in the registers to the right
 * and the bit that shifted out of the regiister will store into
 * the carry flag and and that bit will also go to the
 * most significant bit of the register.
 */
void set_ROTATE()
{
	unsigned int shiftCounter;

	shiftCounter = reg.R[RN];	//to see how many times to shift.
	ALU = reg.R[RD];		//storeing the reigster's values into ALU
	while(shiftCounter  > 0)
	{
		CF = (ALU & LSB32);  //storing the LSB to the carry flag.
		ALU >>= 1;		//shift one to the right.
		if(CF == 1)		//if the carryy flag contain 1 then put it to MSB.
		{
			ALU = ALU | MSB32; //or ALU with MSB32 so the MSB will contain 1.
		}
		shiftCounter--;
	}
	reg.R[RD] = ALU;	//store the end result into the target register.
}

/***Function set_PUSH()***/
/* this function will push the registers in the register list into the
 * stack pointer. When pushing it will pre decrement the SP and store
 * the values into the memory that SP is pointing to. If there is an
 * extra push it will push LR into memory first then push the registers.
 */
void set_PUSH()
{
unsigned int LSBCounter;
unsigned int counter;
signed int wcount;

	counter = 0;		//couting up to 8 times for register list.
	LSBCounter = 7;	//starting register number.

	if(PSHPUL_EXTRA == PSH_LR)	//do this if extra push is set.
	{ 
printf("Pushing Extra\n");

		SP -= WORD_BYTE;	//decrement the sp by a word.
		MAR = SP;		//location of memory at SP.
		MBR = LR;		//LR store into MBR.
		for(wcount = 3; wcount >= 0; wcount--)	//do it 4 times so it will store 1 byte each time
		{
			MEM = (unsigned char) (MBR >> (BYTE_BITS * wcount));	//storing it to mem by looking at MBR 8 bits each time starting form the left
			++MAR;       						//look at the next address
                }
printf("before end Extra\n");

	}
printf("end Extra\n");


	while(counter < registerCount)
	{
		if(((PSHPUL_REG >> LSBCounter) & LSB32) == 1)
		{
			SP -= WORD_BYTE;	//decrement the sp by a word.
			MAR = SP;		//location of memory at SP.
			switch(PSHPUL_OPC)	
			{	
				case PSH_LOW:	//for Low registers list.
printf("Pushing low\n");
printf("Register pushed: %d.\n", LSBCounter);
				MBR = reg.R[LSBCounter];	//the destination register.
	   		 	for(wcount = 3; wcount >= 0; wcount--)	//do it 4 times so it will store 1 byte each time
			 	{	
			 		MEM = (unsigned char) (MBR >> (BYTE_BITS * wcount));	//storing it to mem by looking at MBR 8 bits each time starting form the left
					++MAR;       						//look at the next address

              	 	}
				break;
			
				case PSH_HIGH: //for High registers list.
				MBR = reg.R[LSBCounter + highRegister];	//the destination register.
printf("Pushing high\n");
printf("Register pushed: %d.\n", LSBCounter+highRegister);
		   		for(wcount = 3; wcount >= 0; wcount--)	//do it 4 times so it will store 1 byte each time
			 	{
			 		MEM = (unsigned char) (MBR >> (BYTE_BITS * wcount));	//storing it to mem by looking at MBR 8 bits each time starting form the left
					++MAR;       						//look at the next address
              	 	}
				break;	
			}
		}
		LSBCounter--;
		counter++;
	}
}


/***Function set_PULL()***/
/* this function will pull the value from the memory into the register
 * list from where the stack pointer is point to. When pulling it will
 * post increment the SP and load the value into the registers. If there 
 * is an extra pull it will pull PC after all the registers are pulled.
 */
void set_PULL()
{
printf("PULLING AREA!\n");

	unsigned int counter;
	unsigned int MSBCounter;
	signed int count;



	counter = 0;		//couting up to 8 times for register list.
	MSBCounter = 0;	//starting register number.
	while(counter < registerCount)
	{
		if(((PSHPUL_REG >> MSBCounter) & LSB32) == 1)
		{
			MAR = SP;	//memory location at SP pointing to.
			MBR = (unsigned long) MEM;	//storing the memory into MBR
			switch(PSHPUL_OPC)
			{
				case PUL_LOW:	//for Low registers list.
			 	for(count = 0; count < 3; count++)		//do it 4 times so it will load 1 byte each time.
			 	{
					MBR <<= BYTE_BITS;			//shift the MBR 8bits to the left
					++MAR;					//look at the next address
					MBR |= MEM;				//mask the MBR with MEM.
			 	}
				reg.R[MSBCounter] = MBR;
printf("Pulling low\n");
printf("Register pulled: %d.\n", MSBCounter);	
				break;
				
				case PUL_HIGH: //for High registers list.
			 	for(count = 0; count < 3; count++)		//do it 4 times so it will load 1 byte each time.
			 	{
					MBR <<= BYTE_BITS;			//shift the MBR 8bits to the left
					++MAR;					//look at the next address
					MBR |= MEM;				//mask the MBR with MEM.
			 	}
				reg.R[MSBCounter + highRegister] = MBR;	
printf("Pulling high\n");
printf("Register pulled: %d.\n", MSBCounter + highRegister);
				break;

				case PUL_PC:	
				break;
			}
			SP += WORD_BYTE;	//increment the SP by a word
		}
		MSBCounter++;	//go to the next register
		counter++;	//go to the next counter.
	}

	if(PSHPUL_EXTRA == PUL_PC)	//do this if the extra pull is set.
	{ 
printf("Pulling Extra\n");

		MAR = SP;	//memory location at SP pointing to.
		MBR = (unsigned long) MEM;	//storing the memory into MBR
		for(count = 0; count < 3; count++)		//do it 4 times so it will load 1 byte each time.
		{
			MBR <<= BYTE_BITS;			//shift the MBR 8bits to the left
			++MAR;					//look at the next address
			MBR |= MEM;				//mask the MBR with MEM.
		}
		PC = MBR;	//store the MBR into PC	

	SP += WORD_BYTE;	//increment the SP by a word
	}
}


/* This is the main part of the program. It will excute the programs
 * according to the user input.
 */
int main()
{
	unsigned char input, option;
	unsigned int offset;
	unsigned int length;
	unsigned int fsize;


	printf("(((((Welcome to Lin Aung's Computer Architect Virtual CPU)))))\n");
	printUsage();
	displayReg();
	
	while (input!='q')
	{
	printf("\n-Please enter a command\n");
       scanf(" %c", &input);
      	option = toupper(input);

	switch(option)
	{
	case 'D': printf("Dump file selected\n");
		   printf("Please enter the offset\n");
		   scanf("%x", &offset);
		   while(offset > MAXSIZE)
		   {
			printf("ERROR HEX\n");
			printf("Please enter the offset\n");
	        	scanf("%x", &offset);
			printf("The entered offset in HEX %x\n", (unsigned int) offset);

		   }		
	          printf("Please enter the length to be dumped\n");
	          scanf("%x", &length);
		   printf("The entered offset in HEX is %x\n", (unsigned int) offset);
	 	   printf("The entered length in HEX is %x\n", (unsigned int) length);
		   dump(memory, offset, length);
       	   break;
	
	case 'G': go();
                 break;

	case 'L': printf("You have selected the Load option\n");
                 fsize = load(memory, MAXSIZE);  //passing the buffer, size variable and file into the load function
                 		printf("Total file size = %d", fsize);

                 break;
      	
	case 'Q': printf("---------------------\n");
		   printf("Program terminated!!!\n");
		   printf("Good Bye!!!\n");
		   printf("---------------------\n");
		   break;
      	
	case 'R': printf("Displaying contents of the registers\n"); 
                 displayReg();	//call the function displayReg to display the registers.
        	   break;
      	
	case 'T': fetch();	//call the function fetch() to get the next set of instruction.
		   displayReg();	//call the function displayReg() to display the registers.  	
	          break;
      	
	case 'Z': printf("You have decided to reset the registers\n");
		   resetReg();  //reseting the registers
                 break;

      	case 'H': printUsage();
		   break;

       default: printf("Unrecognized command\n");
                printf("Enter h for command list\n");
		
	}
	}
   return 0;
}


