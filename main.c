/***************************************************** 
	Ryan Lachman
	810-113-407
	Virtual CPU Interface
	CENG 606 - Computer Systems Architecture

	Creating a Virtual CPU by setting an int array
	as memory and implementing registers and flags 
	along with a write, load file function and 
	memory dumps.
*****************************************************/


#define MEM			16384					//define mem alias as 16k (16 *1024)
#define STACK_SIZE	256						//define size of stack to use with push/pull
//#define _CRT_SECURE_NO_DEPRECATE			//avoid MS errors pertaining to certain functions
#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* vcpu memory */
unsigned char memory[MEM];
unsigned int  stack[STACK_SIZE];

/* general variable declarations: buffer, offset, length, starting address */
unsigned char buff[100];
unsigned int off;			//hold user offset
unsigned int len;			//hold user length
unsigned int startAddr;		//hold user start address for memory modify
#define colWidth	16
#define fetchShift	4
#define byteShift	8
#define MAX32		32
#define NMASK		0x80000000
#define MMASK		0xFF000000

unsigned long ALU;

/* Lab 6 - Registers */
unsigned long REG[16];
#define SP  (REG[13])		//stack pointer
#define LR  (REG[14])		//link register
#define PC  (REG[15])		//program counter

/* declare CCR flags, MBR, MAR, IR0, IR1, Stop flag, IR active flag */
unsigned long MBR;			//memory buffer register - buffer for all data
unsigned long MAR;			//memory address register - holds memory address
unsigned long IR;			//instruction registers 0, 1
unsigned short IR0;
unsigned short IR1;

unsigned int N_FLAG;
unsigned int C_FLAG;
unsigned int Z_FLAG;
unsigned int STOP_FLAG;	
unsigned int IR_FLAG;

/* Lab 7 - Instructions
	Instruction Types */
#define	RD		0x000F
#define RN		0x00F0

//Instruction	Value of first byte	    Name
#define DATA	0x0000					//Data Processing			
#define LOST	0x2000					//Load/Store
#define IMOP	0x4000					//Immediate Operations
#define IMAD	0x6000					//Immediate when affected by ADD mask: 0110 = 6
#define CBRA	0x8000					//Conditional Branch
#define PUPH	0xA000					//Push/Pull
#define UBRA	0xC000					//Unconditional Branch
#define STOP	0xE000					//Stop

/* Data Processing Instructions MASKS
	15 - 12 == 0000
	11 - 8  == Operation
	7  - 4  == Rn
	3  - 0  == Rd */
//Operation		Code (in Hex)			
#define AND		0x0					
#define EOR		0x1
#define SUB		0x2
#define SXB		0x3
#define ADD		0x4
#define ADC		0x5
#define LSR		0x6
#define LSL		0x7
#define TST		0x8
#define TEQ		0x9
#define CMP		0xA
#define ROR		0xB
#define ORR		0xC
#define MOV		0xD
#define BIC		0xE
#define MVN		0xF

/* Load/Store MASKS */
#define STOR	0x0
#define LOAD	0x1
#define L_MASK  0x0800
#define B_MASK	0x0400
#define K_MASK	0x1000

/* Immediate Instructions MASKS
	15 - 12 == 01OPCode
	11 - 4  == 8-bit Value 
	3  - 0  == Rd */
//Operations	Code
#define I_MOV		0x4000
#define I_CMP		0x5000
#define I_ADD		0x6000
#define I_SUB		0x7000
#define OPCO		0xF000
#define ImmRn		0x0FF0
#define IMMASK		0xC000

/* Conditional Instructions MASKS
	15 - 12 == 1000
	11 - 8  == condition
	7  - 0  -- 8-bit relative address */
//Suffix		Code		Flags					Meaning
#define EQ		0x0000		//Z set					Equal
#define NE		0x0001		//Z clear				Not Equal
#define CS		0x0002		//C set					unsigned higher or same
#define CC		0x0003		//C clear				unsigned lower
#define MI		0x0004		//N set					negative
#define PL		0x0005		//N clear				positive
#define HI		0x0008		//C set and Z clear		unsigned higher
#define LS		0x0009		//C clear or Z set		unsigned lower or same
#define AL		0x000E		//ignored				Always

#define CBRA_ADDR 0x00FF
#define CBRA_GET_ADDR(x)	(CBRA_ADDR & x)
#define BRANCH_MASK	0xF00

/* Push/Pull Instructions MASKS
	15 - 12 == 1010
	11 - 8  == LH-R
	7  - 0  == Register List */
#define PPL			0x8000
#define PPR			0x4000
#define PPH			0x1000
#define H_MASK		0x0400
#define R_MASK		0x0100
#define REG_MASK	0x00FF

/* Unconditional Branch Instructions MASKS
	15 - 12 == 110K
	11 - 0  == Offset12 */
#define UOFF	0x0FFF

/* call functions */
int  LoadFile(void * memory, unsigned int max);
void WriteFile(void * memory);
void MemModify(void *toMem, unsigned startAddress);
void MemDump(void *memptr, unsigned offset, unsigned length);
void DisplayRegisters();
void ResetRegisters();
void fetch(void *toMem);
void execute(unsigned short ir, void *toMem);
void decode(void *toMem);
void Trace(void *toMem);
void cutIR();
void Go();
int isCarry(unsigned long op1, unsigned long op2, unsigned C);
void allFlags(unsigned long op1, unsigned long op2);
void nzFlags(unsigned long op1);

/***************************************
	Main function 
		contains main menu and switch 
		case 
***************************************/
int main(){

	ResetRegisters(REG);

	int choice = 0;
	Menu();

		while (1){

			choice = Prompt();

			switch (choice){

				case 'D': //dump memory
					printf("\nEnter the offset: ");
					fgets(buff, sizeof(buff), stdin);
					sscanf(buff, "%X", &off);
			
					/*if(off > sizeof(buff)){					
						perror("\nERROR with input!\n");
						return (-1);
					}*/

					printf("\nEnter the length: ");
					fgets(buff, sizeof(buff), stdin);
					sscanf(buff, "%X", &len);
				
					if(len > sizeof buff){
						perror("\nERROR with input!\n");
						return (-1);
					}

					MemDump(memory, off, len);
					break;

				case 'G': //go - run the entire program
					
					Go();
					break;

				case 'L': //load a file into memory
					LoadFile(memory, sizeof(memory));
					break;

				case 'M': //memory modify
					printf("\nEnter the starting address: ");
					fgets(buff, sizeof(buff), stdin);
					sscanf(buff, "%X", &startAddr);

					if(startAddr > sizeof(memory)){
						perror("\nERROR\n");
						return (-1);
					}

					MemModify(memory, startAddr);
					break;

				case 'Q': //quit
					Quit();
					break;

				case 'R': //display registers
					DisplayRegisters();
					break;

				case 'T': //trace - execute one instruction
					Trace(memory);
					break;

				case 'W': //write file
					WriteFile(memory);
					break;

				case 'Z': //reset all registers to zero
					ResetRegisters(REG);
					break;

				case '?': case 'H': //display list of commands
					Help();
					break;

				default: printf("\nInvalid entry!\nPlease enter a valid option!\n");
		}
	}
	return 0;
}

//---END Main function-------------------------------------------------------------------------------------------------------------------------------------------------


/*
 *	SUB FUNCTIONS	-----
 */

/************************************************************ 
	cutIR function
		cut IR in half to get both instructions
*************************************************************/
void cutIR(){

	IR0 = IR >> 16;
	IR1 = IR;
}

/*************************************************************
	fetch instruction function
		takes data loaded from memory into MAR and MBR 
		registers
*************************************************************/
void fetch(void *toMem){

	int i = 0;
	
		MAR = PC;
		
		for (i = 0; i < fetchShift; i++){
			//get contents of buffer register
			MBR = MBR << byteShift;
			MBR += ((unsigned char *)toMem)[MAR + i];
		}

		PC = PC + sizeof(IR);
		IR = MBR;
}

/*************************************************************
	decode function
		takes type of instruction and goes to appropriate
		function for instruction type execution
*************************************************************/
void decode(void *toMem){

		//If Instruction Register flag is set to zero then fetch and decode
		if (IR_FLAG == 0){				
			
			fetch(memory);
			cutIR();
			IR_FLAG = 1;
			execute(IR0, memory);
		}
		else{

			cutIR();
			IR_FLAG = 0;
			execute(IR1, memory);
		}
}

/*************************************************************
	execute instruction function
		executes instruction by determining instruction type 
		and then sending it to the respective function 
*************************************************************/
void execute(unsigned short ir, void *toMem){

	int i, j;
	unsigned val;
	unsigned short opCode;
	unsigned short instType = (ir & STOP);
	unsigned short Rn = (ir & RN) >> 4;
	unsigned short Rd = (ir & RD);
	unsigned short L = (ir & L_MASK) >> 11;					//Load/Store format to get L bit & for Push/Pull, 0 = PSH, 1 = PUL
	unsigned short B = (ir & B_MASK) >> 10;					//Load/Store format to get B bit
	unsigned short K = (ir & K_MASK) >> 12;					//Unconditional Branch format to get K bit
	unsigned short uOffset = (ir & UOFF);					//offset for Unconditional Branch
	char		   cOffset;									//offset for Conditional Branch				
	unsigned short condition = (ir & BRANCH_MASK) >> 8;		//condition for Conditional Branch
	unsigned short operation = (ir & BRANCH_MASK) >> 8;		//operation for Data Processing
	unsigned short H = (ir & H_MASK) >> 10;					//High/Low bit, 0 = low registers(0-7), 1 = high registers (8-15)
	unsigned short R = (ir & R_MASK) >> 8;					//R bit, 0 = no extra pulls or pushes, LR, 1 = pull PC/push LR
	unsigned char regList = (ir & REG_MASK);				//8 bit field with each bit corresponding to a register
	unsigned int M_MASK = MMASK;
	int baseReg = 0;

		//printf("\ntype is %X\n\n", instType);

		switch (instType){

			case DATA:	//data processing
				printf("\nThis is DATA\n\n");
				printf("\nThis is operation %X\n\n", operation);
				switch (operation){
					
					case AND:
						printf("\nThis is AND\n\n");
						ALU = (REG[Rd] & REG[Rn]);
						nzFlags(ALU);
						REG[Rd] = ALU;
						break;

					case EOR:
						printf("\nThis is EOR\n\n");
						ALU = (REG[Rd] ^ REG[Rn]);
						nzFlags(ALU);
						REG[Rd] = ALU;
						break;

					case SUB:
						printf("\nThis is SUB\n\n");
						ALU = REG[Rd] + (~REG[Rn] + 1);
						nzFlags(ALU);
						REG[Rd] = ALU;
						break;

					case SXB:
						printf("\nThis is SXB\n\n");
						REG[Rd] = (signed)REG[Rn];
						nzFlags(REG[Rd]);
						break;

					case ADD:
						printf("\nThis is ADD\n\n");
						ALU = REG[Rd] + REG[Rn];
						allFlags(REG[Rd], REG[Rn]);
						REG[Rd] = ALU;
						break;

					case ADC:
						printf("\nThis is ADC\n\n");
						ALU = REG[Rd] + REG[Rn] + C_FLAG;
						allFlags(ALU, REG[Rn]);
						REG[Rd] = ALU;
						break;

					case LSR:
						printf("\nThis is LSR\n\n");
						ALU = REG[Rd] >> REG[Rd];
						allFlags(ALU, REG[Rn]);
						REG[Rd] = ALU;
						break;

					case LSL:
						printf("\nThis is LSL\n\n");
						ALU = REG[Rd] << REG[Rd];
						allFlags(ALU, REG[Rn]);
						REG[Rd] = ALU;
						break;

					case TST:
						printf("\nThis is TST\n\n");
						ALU = REG[Rd] & REG[Rn];
						nzFlags(ALU);
						REG[Rd] = ALU;
						break;

					case TEQ:
						printf("\nThis is TEQ\n\n");
						ALU = REG[Rd] ^ REG[Rn] + 1;
						nzFlags(REG[Rd]);
						break;

					case CMP:
						printf("\nThis is CMP\n\n");
						ALU = REG[Rd] + (~REG[Rn] + 1);
						allFlags(ALU, REG[Rn]);
						break;

					case ROR:
						printf("\nThis is ROR\n\n");
						for (i = 0; i < REG[Rn]; i++){

							C_FLAG = REG[Rn] & 0x0001;
							if (C_FLAG){

								ALU = (REG[Rd] >> 1) | NMASK;
							}
						}

						allFlags(ALU, REG[Rn]);
						REG[Rd] = ALU;
						break;

					case ORR:
						printf("\nThis is ORR\n\n");
						ALU = REG[Rd] | REG[Rn];
						nzFlags(ALU);
						REG[Rd] = ALU;
						break;

					case MOV:
						printf("\nThis is MOV\n\n");
						REG[Rd] = REG[Rn];
						nzFlags(REG[Rd]);
						break;

					case BIC:
						printf("\nThis is BIC\n\n");
						ALU = REG[Rd] & ~REG[Rn];
						nzFlags(ALU);
						REG[Rd] = ALU;
						break;

					case MVN:
						printf("\nThis is MVN\n\n");
						REG[Rd] = ~REG[Rn];
						nzFlags(REG[Rd]);
						break;

					default:
						perror("\nInvalid Operation!\n\n");
						break;
				}
				break;

			case LOST:  //load/store
				printf("\nThis is LOAD/STORE\n\n");
					switch (L){
						
						case STOR:	//Store
							printf("\nSTORE, RN = %X\n\n", REG[Rn]);
							MAR = REG[Rn];										//set Rn as memory address 
																				
							if (B == 0){										//trasnfer Word to memory
								printf("\nB == 0\n\n", REG[Rn]);
								MBR = REG[Rd];									//set data in MBR to destination register
								//MAR = REG[Rn];

								for (i = 0; i < 4; i++){

									((unsigned char *)toMem)[MAR++] = (MBR & M_MASK) >> (8 * (3 - i));
									M_MASK = M_MASK >> byteShift;
								}

							}
							else if(B == 1){									//transfer byte to memory
								printf("\nB == 1\n\n", REG[Rn]);
								MAR = REG[Rn];
								//MBR = REG[Rd];								//set data in MBR to destination register
								((unsigned char *)toMem)[MAR] = MBR;
							}
							break;

						case LOAD:	//Load
							printf("\nLOAD %X\n\n", REG[Rn]);
							if (B == 1){										//transfer byte from memory
								printf("\nB == 1\n\n", REG[Rn]);
								MAR = REG[Rn] & 0x000000FF;
								MBR = ((unsigned char *)toMem)[MAR];
								REG[Rd]	= MBR;									//taking data from source register into memory buffer register
							}
							else if(B == 0){									//transfer word from memory
								printf("\nB == 0%X\n\n", REG[Rn]);
								MAR = REG[Rn];		
								MBR = REG[Rd];

								for (i = 4; i > 0; i--){

									MBR = MBR << byteShift;
									MBR += ((unsigned char *)toMem)[MAR ++];
									
								}

								REG[Rd] = MBR;
							}
							break;
					}
				break;

			case IMOP:	//immediate operations
			case IMAD:
				opCode = (ir & OPCO);
				val = (ir & ImmRn) >> 4;
				printf("\nThis is IMMEDIATE\n\n");

				switch (opCode){

					case I_MOV:	// MOV
						printf("\nThis is MOV\n\n");
						REG[Rd] = val;
						nzFlags(REG[Rd]);
						break;

					case I_CMP:	// CMP
						printf("\nThis is CMP\n\n");
						ALU = REG[Rd] - val;
						allFlags(ALU, val);
						break;

					case I_ADD:	// ADD
						printf("\nThis is ADD\n\n");
						ALU = REG[Rd] + val;					
						allFlags(ALU, val);
						REG[Rd] = ALU;
						break;

					case I_SUB:	// SUB
						printf("\nThis is SUB\n\n");
						ALU = REG[Rd] + (~val + 1);
						allFlags(ALU, val);
						REG[Rd] = ALU;
						break;

					default:
						perror("\nInvalid Operation!\n\n");
						break;
				}
				break;

			case CBRA:	//conditional branch
				printf("\nThis is CBRA\n\n");
				cOffset = CBRA_GET_ADDR(ir);
				//PC -= 2;
				switch (condition){

					case EQ:
						printf("\nThis is EQ\n\n");
						if (Z_FLAG == 1)
							PC += cOffset;
						
						break;

					case NE:
						printf("\nThis is NE\n\n");
						if (Z_FLAG == 0)
							PC += cOffset;
						printf("\nPC is %X, offset is %X\n\n", PC, cOffset);
						break;

					case CS:
						printf("\nThis is CS\n\n");
						if (C_FLAG == 1)
							PC += cOffset;
						break;

					case CC:
						printf("\nThis is CC\n\n");
						if (C_FLAG == 0)
							PC += cOffset;
						break;

					case MI:
						printf("\nThis is MI\n\n");
						if (N_FLAG == 1)
							PC += cOffset;
						break;

					case PL:
						printf("\nThis is PL\n\n");
						if (N_FLAG == 0)
							PC += cOffset;
						break;

					case HI:
						printf("\nThis is HI\n\n");
						if (C_FLAG == 1 && Z_FLAG == 0)
							PC += cOffset;
						break;

					case LS:
						printf("\nThis is LS\n\n");
						if (C_FLAG == 0 && Z_FLAG == 1)
							PC += cOffset;
						break;

					case AL:
						printf("\nThis is AL\n\n");
						PC += cOffset;
						break;

					default:
						perror("\nInvalid Operation!\n\n");
						break;
				}
				IR = 0;
				IR_FLAG = 0;
				break;

			case PUPH:	//push/pull
				printf("\nThis is PUPH\n\n");
				switch (L){

				case 0: //PSH
				
					if (H == 1){	//push low registers
						for (j = byteShift; j >= 0; j--){

							if ((regList >> j) == 1){

								MBR = REG[j];

								MAR = REG[Rn];										//set Rn as memory address 
								//trasnfer Word to memory
								printf("\nB == 0\n\n", REG[Rn]);
								MBR = REG[Rd];									//set data in MBR to destination register
								//MAR = REG[Rn];

								for (i = 0; i < 4; i++){

									SP = MAR;
									--SP;
									((unsigned char *)toMem)[SP] = (MBR & M_MASK) >> (8 * (3 - i));
									M_MASK = M_MASK >> byteShift;
								}
							}
						}

						if (R == 0){	//no extra pulls or pushes


						}
						else if (R == 1){	//push LR

							MBR = LR;

							MAR = REG[Rn];										//set Rn as memory address 
							//trasnfer Word to memory
							printf("\nB == 0\n\n", REG[Rn]);
							MBR = REG[Rd];									//set data in MBR to destination register
							//MAR = REG[Rn];

							for (i = 0; i < 4; i++){

								SP = MAR;
								--SP;
								((unsigned char *)toMem)[SP] = (MBR & M_MASK) >> (8 * (3 - i));
								M_MASK = M_MASK >> byteShift;
							}
						}
					}

					else if (H == 1){	//push high registers

						for (j = byteShift; j >= 0; j--){

							if ((regList >> j) == 1){

								MBR = REG[j] + 8;

								MAR = REG[Rn];										//set Rn as memory address 
								//trasnfer Word to memory
								printf("\nB == 0\n\n", REG[Rn]);
								MBR = REG[Rd];									//set data in MBR to destination register
								//MAR = REG[Rn];

								for (i = 0; i < 4; i++){

									SP = MAR;
									--SP;
									((unsigned char *)toMem)[SP] = (MBR & M_MASK) >> (8 * (3 - i));
									M_MASK = M_MASK >> byteShift;
								}
							}
						}

						if (R == 0){	//no extra pulls or pushes


						}
						else if (R == 1){	//push LR

							MBR = LR;

							MAR = REG[Rn];										//set Rn as memory address 
							//trasnfer Word to memory
							printf("\nB == 0\n\n", REG[Rn]);
							MBR = REG[Rd];									//set data in MBR to destination register
							//MAR = REG[Rn];

							for (i = 0; i < 4; i++){

								SP = MAR;
								--SP;
								((unsigned char *)toMem)[SP] = (MBR & M_MASK) >> (8 * (3 - i));
								M_MASK = M_MASK >> byteShift;
							}
						}
					}
					break;
				case 1: //PUL

					if (H == 0){	//pull low registers

						for (j = byteShift; j >= 0; j--){

							if ((regList >> j) == 1){

								MBR = REG[j];
								//transfer byte from memory
								printf("\nB == 1\n\n", REG[Rn]);
								MAR = REG[Rn] & 0x000000FF;
								MBR = ((unsigned char *)toMem)[MAR];
								REG[Rd] = MBR;										//taking data from source register into memory buffer register

								//transfer word from memory
								printf("\nB == 0%X\n\n", REG[Rn]);
								MAR = REG[Rn];
								MBR = REG[Rd];

								for (i = 4; i > 0; i--){

									SP = MAR;
									SP++;
									MBR = MBR << byteShift;
									MBR += ((unsigned char *)toMem)[SP];
								}
								REG[Rd] = MBR;
							}
						}

						if (R == 0){	//no extra pulls or pushes


						}
						else if (R == 1){	//pull LR

							MBR = REG[j];

							//transfer byte from memory
							printf("\nB == 1\n\n", REG[Rn]);
							MAR = REG[Rn] & 0x000000FF;
							MBR = ((unsigned char *)toMem)[MAR];
							REG[Rd] = MBR;									//taking data from source register into memory buffer register

							//transfer word from memory
							printf("\nB == 0%X\n\n", REG[Rn]);
							MAR = REG[Rn];
							MBR = REG[Rd];

							for (i = 4; i > 0; i--){

								MAR = SP;
								SP++;
								MBR = MBR << byteShift;
								MBR += ((unsigned char *)toMem)[SP];
							}
							LR = MBR;
						}
					}

					if (H == 1){	//pull high registers

						for (j = byteShift; j >= 0; j--){

							if ((regList >> j) == 1){

								MBR = REG[j] + 8;

								MAR = REG[Rn];										//set Rn as memory address 
								//trasnfer Word to memory
								printf("\nB == 0\n\n", REG[Rn]);
								MBR = REG[Rd];									//set data in MBR to destination register
								//MAR = REG[Rn];

								for (i = 0; i < 4; i++){

									SP = MAR;
									--SP;
									((unsigned char *)toMem)[SP] = (MBR & M_MASK) >> (8 * (3 - i));
									M_MASK = M_MASK >> byteShift;
								}
							}
						}

						if (R == 0){	//no extra pulls or pushes


						}
						else if (R == 1){	//push LR

							MBR = REG[j];

							//transfer byte from memory
							printf("\nB == 1\n\n", REG[Rn]);
							MAR = REG[Rn] & 0x000000FF;
							MBR = ((unsigned char *)toMem)[MAR];
							REG[Rd] = MBR;									//taking data from source register into memory buffer register

							//transfer word from memory
							printf("\nB == 0%X\n\n", REG[Rn]);
							MAR = REG[Rn];
							MBR = REG[Rd];

							for (i = 4; i > 0; i--){

								MAR = SP;
								SP++;
								MBR = MBR << byteShift;
								MBR += ((unsigned char *)toMem)[SP];
							}
							LR = MBR;
						}

					}
					
				}
				break;
			case UBRA:	//unconditional branch
				printf("\nThis is UBRA\n\n");
				IR_FLAG = 0;
				if (K == 0){
					
					PC = uOffset;
				}
				else if (K == 1){
					
					LR = PC;
					PC = uOffset;	
				}
				break;

			case STOP:	//stop
				printf("\nThis is STOP\n\n");
				STOP_FLAG = 1;
				break;

			default:	//error checking
				perror("Instruction type invalid.\n");
				break;
		}
}

/*
 *	FLAG FUNCTIONS  -----
 */

/*****************************************************************
	isCarry()- determine if carry is generated by addition: 
	op1+op2+C 
	C can only have value of 1 or 0.
*****************************************************************/
int isCarry(unsigned long op1, unsigned long op2, unsigned C){

	if ((op2 == MAX32) && (C == 1))

		return(1);			// special case where op2 is at MAX32

	return((op1 > (MAX32 - op2 - C)) ? 1 : 0);
}

/*****************************************************************
	allFlags() - determines if all flags are generated
*****************************************************************/
void allFlags(unsigned long op1, unsigned long op2){

	nzFlags(op1);

	//use isCarry function to determine carry flag
	C_FLAG = isCarry(op1, op2, C_FLAG);
}

/*****************************************************************
	nzFlags() - determines if only sign and zero flags are 
				generated
*****************************************************************/
void nzFlags(unsigned long op1){

	//check if ALU is zero to determine zero flag
	if (op1 == 0){

		Z_FLAG = 1;
	}
	else if (op1 != 0){

		Z_FLAG = 0;
	}

	//mask ALU for most significant bit (EQ = 0x8000) to determine sign flag
	if ((NMASK & op1) == NMASK){

		N_FLAG = 1;
	}
	else if ((NMASK & op1) != NMASK){

		N_FLAG = 0;
	}
}

/*
 *	END Sub & flag functions  -----------------------------------------------------------------------------------------------------------------------------------------
 */


/*
 *	MAIN FUNCTIONS	-----
 */

/*************************************** 
	Menu function 
	displays name and options for CPU 
***************************************/
int Menu(){

		//system("cls");
		printf("			RYAN'S Virtual CPU\n");
		printf("-----------------------------------------------------------------\n");
		printf("	d			dump memory\n");
		printf("	g			go - run the entire program\n");
		printf("	l			load a file into memory\n");
		printf("	m			memory modify\n");
		printf("	q			quit\n");
		printf("	r			display registers\n");
		printf("	t			trace - execute one instruction\n");
		printf("	w			write file\n");
		printf("	z			reset all registers to zero\n");
		printf("	?, h			display list of commands\n");
		printf("------------------------------------------------------------------\n");

		return 0;
}

/*********************** 
	Prompt function 
	to take user input 
***********************/
int Prompt(){

	printf("\n-> ");
	
	//converts lowercase to uppercase while ignoring if input is already uppercase
	int choice = toupper(getchar()); 
	// while loop to flush input buffer when enter is hit after option input
	while (getchar() != '\n');		
	return choice;
}

/********************************************************************** 
	memory modify function
	m - memory modify 
		Prompts user for starting address and displays existing value.
		Then allows for user input of new value. Once value is entered 
		the next address in memory is given for modification.
**********************************************************************/
void MemModify(void *toMem, unsigned startAddress){
	
	int i = startAddress, j;
	unsigned value = 0x01;
	char period = 0;
	unsigned newVal = 0;
	unsigned char * ptMem = toMem;
	char buffer[8];

		for (j = 0; j < i; j++){
			//take user start address and prints the existing value to that address
			printf("\nStarting Address: 0x%04X\n", i);
			printf("\nExisting value: %02X\n", ptMem[i]);
			if(isprint(ptMem[i])){
				printf("       \t\t%c\n", ptMem[i]);
			}
			//checks if it is a period then notify user
			else{
				printf("       \t\t.\n");
			}

			printf("\nEnter a new value: ");
			fgets(buffer, sizeof(buffer), stdin);
			
			sscanf(buffer, "%X", &newVal);							//put hex value of buffer into newVal to send to MemDump function to output new dump
			sscanf(buffer, "%c", &period);

			//check if user input is a period, if so exit to main
			if(period == '.'){
				exit(0);
			}

			//set new value to current value pointed to in memory
			ptMem[i] = newVal;

			//display memory dump 
			printf("\n%04X\t", i);
			
			//print hex value once
			printf(" %02X ", newVal);
			printf("\n    \t");

			//print ascii value once
			if (isprint(newVal)){
				printf("  %c ", newVal);
			}
			//if not printable then replace hidden \n or \t with '.'
			else printf("  . ");
			printf("\n");
			i++;													//increment value position
		}
}

/********************************************************** 
	dump memory function
	d - displays memory dump 
		Displays a dump of memory that is 16 columns wide
		and is shown by offset and length
**********************************************************/
void MemDump(void *memptr, unsigned offset, unsigned length){

	unsigned int i, j;
	unsigned char * toMemptr = memptr;
	unsigned int addedLength = (offset + length);
	unsigned int colEnd = (offset + colWidth);

		//output user input in Hex
		printf("\nOffset: 0x%04X\n", offset);	
		printf("\nLength: 0x%04X\n", length);
	
		for (i = offset; i < addedLength; i += colWidth, colEnd += colWidth){
			printf("\n%04X\t", i);
			
			j = i;							
			while (j < colEnd){
				//print hex values according to max 16 columns
				printf(" %02X ", toMemptr[j]);
				j++;
			}
			
			printf("\n    \t");

			j = i;
			while (j < colEnd){
				//print ascii values according to max 16 columns if it is a valid printable character
				if (isprint(toMemptr[j])){
					printf("  %c ", toMemptr[j]);
				}
				//if not printable then replace hidden \n or \t with '.'
				else printf("  . ");
				j++;
			}
		}
		printf("\n");
}

/**************************************************************** 
	go function
	g - run the entire program 
		While the stop flag is not set the program will execute 
		and display registers
****************************************************************/
void Go(){

	while (!STOP_FLAG){

		decode(memory);
	}

	DisplayRegisters();
}

/******************************************************
	load file function
	l - load a file into memory 
		Takes a path of the file to load into memory.
		The file is open and read in binary. 
******************************************************/
int LoadFile(void * memory, unsigned int max){
	
	char fileName[80];  	//buffer to hold file path & name
	FILE * fd;				//file descriptor to read the file
	int n;					//variable to hold bytes of file
	int size;

		printf("\nEnter full path of file to load: ");
		//fgets(fileName, sizeof(fileName), stdin);		
		gets(fileName);

		//input string checking if anything valid was entered or if nothing was entered
		if (strlen(fileName) == 0){		
			perror("\nNo input given, enter a valid file name and path.\n");
			return (-1);
		}

		//open file to read in binary
		fd = fopen(fileName, "rb");	
		//check if the buffer is empty
		if (fd == NULL){
			//if buffer is empty then the file was not successfully opened
			perror("\nFile does not exist or is empty.\n");
			return (-1);
		}
		fseek(fd, 0L, SEEK_END);		//go to end of file to get size of file before putting in memory
		size = ftell(fd);				//put raw size into variable
		fseek(fd, 0L, SEEK_SET);		//return to beginning of file
		
		//if file is opened then read into memory
		n = fread(memory, sizeof(char), max, fd);	

		//check for truncate if the size of the byte variable is greater than the max memory
		if (size > max){	
			printf("\nThe file's size is truncated as it exceeds the maximum memory.\n");
		}

		//display the bytes of the file in decimal and hex
		fprintf(stdout, "\n%d Bytes read in decimal and %xH in Hex.\n", n, n);	

		fclose(fd);
}

/*************************** 
	quit function
	q - quits the program 
***************************/
int Quit(){

	exit(0);
}

/*************************************************************** 
	display registers function
	r - displays registers 
		Displays the registers, special registers,
		the sign, zero and carry flag, the MBR, MAR
		and 2 instruction registers and the stop and IR flags.
		These are displayed in common register display format.
***************************************************************/
void DisplayRegisters(){

	unsigned int i, j = 0;
	unsigned int max = 12;

		for (i = 0; i <= max; i++){
			fprintf(stdout, "R%i: %08lX", j, REG[i]);
			j++;
			printf("\t");
		}

		fprintf(stdout, "\nSP:  %08lX  ", SP);
		fprintf(stdout, " LR:  %08lX  ", LR);
		fprintf(stdout, " PC:  %08lX\n", PC);

		fprintf(stdout, "\t\t\t\t\t\tN: %i ", N_FLAG);
		fprintf(stdout, "Z: %i ", Z_FLAG);
		fprintf(stdout, "C: %i\n", C_FLAG);

		fprintf(stdout, "MBR: %08lX ", MBR);
		fprintf(stdout, "  MAR: %08lX\n", MAR);
		fprintf(stdout, "IR0: %04hX ", IR0);
		fprintf(stdout, "IR1: %04hX\n", IR1);
		fprintf(stdout, "Stop: %i ", STOP_FLAG);
		fprintf(stdout, "  IRAF: %i\n", IR_FLAG);
}

/**********************************************************
	trace function 
	t - execute one instruction 
		Single steps the program before display registers.
**********************************************************/
void Trace(void *toMem){

	decode(toMem);

	DisplayRegisters();
}

/*************************************************************************** 
	write file function 
	w - write a file 
		Prompts user for name of file. Then prompts for the size of bytes
		to write to the file.
***************************************************************************/
void WriteFile(void * memory){

	char fileNameW[80];			//buffer to hold filename 
	int writeByte;				//variable to hold number of bytes
	FILE * fp;					//file descriptor for writing to file
	int w;						//variable to hold bytes of file
	
		printf("\nEnter the name of the file to write: ");
		gets(fileNameW);	

		//input string checking if anything valid was entered or if nothing was entered
		if (strlen(fileNameW) == 0){	
			perror("\nNo input given, enter a valid file name.\n");
			return(-1);
		}

		//open file to write in binary
		fp = fopen(fileNameW, "wb");	

		//take user input of size of bytes to write to the file
		printf("Enter the size of bytes to write to file: ");	
		scanf("%i", &writeByte);				

		//check if user enters anything less than 0 for bytes to write
		if (writeByte < 0){					
			//printf("\nNo bytes added to file.\n");
			perror("\nNo bytes added to file.\n");
			return (-1);
		}

		//write the file into memory
		w = fwrite(memory, sizeof(char), writeByte, fp);	
		printf("\nFile written with %d bytes.\n", w);
		getchar();
		fclose(fp);
}

/************************************* 
	reset registers function
	z - reset all registers to zero 
*************************************/
void ResetRegisters(){

	int i;

		for (i = 0; i <= colWidth; i++){
			REG[i] = 0;
		}
	
		SP  = 0;
		LR  = 0;
		PC  = 0;
		MBR = 0;
		MAR = 0;
		IR  = 0;
		N_FLAG = 0;
		C_FLAG = 0;
		Z_FLAG = 0;
		STOP_FLAG = 0;
		IR_FLAG = 0;
}

/************************************* 
	help function
	?, h - display list of commands 
*************************************/
int Help(){

	//system("cls");
	printf("	\n\n	d			dump memory\n");
	printf("	g			go - run the entire program\n");
	printf("	l			load a file into memory\n");
	printf("	m			memory modify\n");
	printf("	q			quit\n");
	printf("	r			display registers\n");
	printf("	t			trace - execute one instruction\n");
	printf("	w			write file\n");
	printf("	z			reset all registers to zero\n");
	printf("	?, h			display list of commands\n");
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------
