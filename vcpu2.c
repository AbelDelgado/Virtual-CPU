/* Author: Winfred Allotey
Date: 04/27/2015
Ceng 606 - Virtual CPU project
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>


//general macros
#define MEMORY 16384
#define CHARBUF 128
#define OFFSIZE 15
#define REGSIZE 16
#define BYTE 8 //8 bits = 1 byte
#define DBYTE 16 // 16 bits - 2 bytes
#define set 1
#define unSet 0
#define subPC 2

// ****************************************************************

//direct reference to registers
#define SP reg[13]
#define LR reg[14]
#define PC reg[15]
// ****************************************************************

//regDisplay()
//reference to registers for display
#define r_pc 15
#define r_lr 14
#define r_sp 13
#define true 1
#define false 0
// ***************************************************************

//Flag macros
#define MAX32 32
#define SZCHCK 0x1
#define SHFTSIGN 15
#define MSBMASK 0x80000000
#define MSBSHFT 31
// ****************************************************************

// Decode Macros. Used to test the operation type
#define SHFTOPMSB 13 //Shifts instruction so first 3 MSB can be tested
#define TESTBITS 0x7 // MASK used to test first 3 bits
#define CHKIMMOP 0x3 // checks which of the 3 bits are set

// RD and RN mask for Data processing, Load/Store, and immediate operation instructions
#define RD ir16val & 0x000F
#define RN (ir16val & 0x00F0) >> 4
// ****************************************************************
//IR0/IR1 result cases for various instructions
#define B_DATAPROC 0x0 // data processing bits
#define B_LOADSTORE 0x1 // Load/Store bits
#define B_IMM1 0x2 // Immediate Operation bits: can be either 010(0x2) or 011(0x3)
#define B_IMM2 0x3 
#define B_CONDBRANCH 0x4 // Conditional Branch bits
#define B_PSHPUL 0x5 // Push/Pull bits
#define B_UCONDBRANCH 0x6 // Unconditional Branch bits
#define B_STOP 0x7	// STOP bits

//Immediate operation instruction
#define IR8BITVAL 0x0FF0 //MASK for 8-bit value in immediate operation
#define OPCODE 12 //Shifts instruction so OpCode bits can be tested

//Result of op codes
#define MOVMASK 0x0  //move
#define CMPMASK 0x1	 //compare
#define ADDMASK 0x2  //add
#define SUBMASK 0x3  //sub
// ****************************************************************
// Load/Store instruction
#define LB_BITS 10 //Shifts instruction so L and B bits can be tested
#define LDSTR_OP 0x3 // Mask to test L & B bits

//Result of L & B bits. Stores/Loads either a word or byte
#define STOREWORD 0x0 
#define STOREBYTE 0x1 
#define LOADWORD 0x2
#define LOADBYTE 0x3
#define REGS_LSBMASK 0x000000FF // Mask for bottom of 32-bit registers
// ****************************************************************

//Unconditional Branch
#define KBITSHFT 12
#define TESTKBIT 0x1 // Mask to test k bit.
#define OFFSET12 0x0FFF //Mask for Offset12 bits

// K bit result
#define BRL 0x1
#define BRA 0x0
//*******************************************************************

//Conditional Branch
#define CONDSHIFT 8 
#define TESTCOND 0xF
#define SIGN8BITVAL 0x00FF

//Result of condition bits
#define BEQ 0x0000 // Branch if equal
#define BNE 0x0001 // Branch if not equal
#define BCS 0x0002 // Branch if unsigned higher or same
#define BCC 0x0003 // Branch if unsigned lower
#define BMI 0x0004 // Branch if negative
#define BPL 0x0005 // Branch if positive
#define BHI 0x0008 // Branch if unsigned higher
#define BLS 0x0009 // Branch if unsigned lower or same
#define BAL 0x000E // Branch if always

//*******************************************************************

//Data Processing
#define OPSHFT 8 // shifts instruction so Operation bit
#define TESTOP 0xFFFF
#define SIGN32BIT 0x80000000

//Result of Operation bits
#define OPAND 0x0000
#define OPEOR 0x0001
#define OPSUB 0x0010
#define OPSXB 0x0011
#define OPADD 0x0100
#define OPADC 0x0101
#define OPLSR 0x0110
#define OPLSL 0x0111
#define OPTST 0x1000
#define OPTEQ 0x1001
#define OPCMP 0x1010
#define OPROR 0x1011
#define OPORR 0x1100
#define OPMOV 0x1101
#define OPBIC 0x1110
#define OPMVN 0x1111
//********************************************************************

//push/pull instructions


// 16 - 32-bit registers
typedef struct 
{
	unsigned long reg[REGSIZE];
	unsigned long ALU;


	// 2 16-bit Instruction Register	
	unsigned short IR0;
	unsigned short IR1;    
	unsigned long IR;	// IR0 + IR1 registers	
	unsigned long MAR; 
	unsigned long MBR;

	//Active Instruction Register	
	unsigned int IR_Active;
	// Stop flag	
	unsigned int stop;

	//CCR Flags - SZC
	unsigned char sign;
	unsigned char zero;
	unsigned char carry; 
} registers;


//******Function Definitions
// CPU MAIN funcs
void writeFile();
void cmdHelp();
int loadFile ( unsigned int,bool);
void memDump (unsigned);
void memModify(unsigned);
void zeroRegisters();
void instructionCycle();
void regDisplay();
void go();

//Execute funcs
void fetch();
void decode(unsigned short);
void dataProcess(unsigned short);
void loadStore(unsigned short);
void immExecute(unsigned short);
void uCondBranch(unsigned short);
void pushPull();
void condBranch(unsigned short);
void pushPull();

//CCR flags functions
void ccrFlags(unsigned short,unsigned long);
void isSignZero(unsigned long);
int isCarry(unsigned long, unsigned long, unsigned);

unsigned char cpuMemory[MEMORY];
registers regs;



// Displays a menu at the start of execution.

int main(int argc, char * argv[ ])
{


	char  inputBuf[10] = ""; 
	char option; 
	bool fileLoaded = false;

	printf("%i MAIN",&cpuMemory);

	printf("Author: Winfred Allotey\n\n");

	printf("       This is a Virtual CPU		 \n");
	printf("To begin, input the letter of the option you wish to perform (capitals accepted).\n");

	printf("*****************************************\n");
	printf("** d - dump memory	               **\n");
	printf("** g - run the entire program          **\n");
	printf("** l - load a file into memory         **\n");
	printf("** m - memory modify	       	       **\n");
	printf("** q - quit		       	       **\n");
	printf("** r - display registers      	       **\n");
	printf("** t - trace - execute one instruction **\n");
	printf("** w - write file		       **\n");
	printf("** z - reset all registers to zero     **\n");
	printf("** ?, h - display list of command      **\n");
	printf("*****************************************\n\n");


	zeroRegisters();	//Zero registers upon startup
	while(1)
	{
		unsigned offset = 0;
		memset(inputBuf,NULL,sizeof inputBuf); // clear the buffer before use
		option = '\0'; // set the option to NULL during every loop
		fseek(stdin,0,SEEK_END); //set stdin to end

		printf("\n\nEnter a command letter: ");
		fgets(inputBuf,sizeof inputBuf,stdin);
		//inputBuf[strlen(inputBuf)+ 1] = '/0';
		sscanf(inputBuf,"%c",&option);

		//printf("%i", strlen(inputBuf));

		if((strlen(inputBuf)- 1) > 1)
		{
			printf("\n\nInvalid input. \nPlease enter a character for the option you would like to preform.\n\n");
		}

		else
		{
			option = toupper(option);

			switch(option)
			{
			case 'D': // memory dump
				printf("Enter starting offset of the memory (4 hex length): ");

				if (scanf("%x04",&offset) == 0)
				{
					printf("\n\ninvalid input. Please enter an offset.\n");
					break;
				}
				memDump(offset);
				break;

			case 'G': // Run entire program
				if(fileLoaded = true) go();
				else printf("There is no file loaded into the CPU");
				break;

			case 'L': //Load from memory
				loadFile(sizeof cpuMemory,fileLoaded);
				break;

			case 'M': // modify memory contents

				printf("Enter memory offset to start modify (4 hex length): ");
				if (scanf("%x04",&offset) == 0)
				{
					printf("\n\ninvalid input. Please enter an offset.\n");
				}
				else
					memModify(offset);

				break;

			case 'Q': // Quit
				printf("The virtual Cpu will now shutdown. Press enter to continue. ");
				getchar();
				exit(0);

			case 'R':  // Display registers
				regDisplay();
				break;

			case 'T': // Trace
				if(regs.stop != set)
				{
					instructionCycle();
					regDisplay();
					break;
				}
				else	printf("Stop flag set: no more instructions");
				break;


			case 'W': // Write to memory
				writeFile();
				break;

			case 'Z': // ZERO registers
				zeroRegisters();
				printf("\nRegisters and Flags cleared.");
				break;

			case '?':// Help Menu
			case 'H':
				cmdHelp();
				break;

			default:
				printf("\n\nUnknown command. Enter 'H' or '?' to refer to the list of commnads.\n\n");
				break;
			}
		}
	}
	getchar();
	return 0;	
}

//************************************************************************

void writeFile ()
{

	char fileNameBuf[CHARBUF] = "";
	int bytes = 0;
	FILE * fp;

	memset(fileNameBuf,0,sizeof fileNameBuf);
	fseek(stdin,0,SEEK_END);

	printf("\nEnter the name of the file you wish to write (max characters = 127): ");
	fgets(fileNameBuf,sizeof fileNameBuf, stdin);
	fileNameBuf[strlen(fileNameBuf) - 1] = '\0'; // null term the enter key

	printf("\n\n%s\n",fileNameBuf);
	//	printf("\n\n%s\n\n",fileNameBuf); To verify if the correct file name is stored.

	if ((fp = fopen(fileNameBuf,"wb")) == NULL)  perror("");// If it cant open print error msg

	printf("\nEnter the number of bytes you would like to write: ");

	if (scanf("%i",&bytes) == 0) 
	{
		printf("\n\nInvalid entry. Please enter an integer value: ");
	}

	// if(scanf("%i",&bytes) > MEMORY)
	//{
	//printf("\n\nYou have exceeded the maximum memory space. please enter a sufficient byte value: ");
	//}

	else 
	{
		fwrite(cpuMemory,sizeof(char),bytes,fp);	
		fileNameBuf[strcspn(fileNameBuf, "\r\n")] = 0;
		fclose(fp);
		printf("\n\nFile created. Name: %s Bytes: %i\n\n",fileNameBuf,bytes);

	}

}
//************************************************************************
void cmdHelp()
{
	printf("\n\nHelp - List of commands\n\n");
	printf("*****************************************\n");
	printf("** d - dump memory	               **\n");
	printf("** g - run the entire program          **\n");
	printf("** l - load a file into memory         **\n");
	printf("** m - memory modify	       	       **\n");
	printf("** q - quit		       	       **\n");
	printf("** r - display registers      	       **\n");
	printf("** t - trace - execute one instruction **\n");
	printf("** w - write file		       **\n");
	printf("** z - reset all registers to zero     **\n");
	printf("** ?, h - display list of command      **\n");
	printf("*****************************************\n\n");
}

//****************************************************************************
int loadFile(unsigned int max, bool loaded)
{

	char file[CHARBUF] = "";
	size_t bytes = 0;
	int fileSize = 0;
	char buf[CHARBUF] = "";
	FILE * fp;

	printf("Enter the name of the file you wish to load: ");
	fgets(file, sizeof file, stdin);
	file[strlen(file) -1] = '\0';

	if ((fp = fopen(file,"rb")) == NULL) perror("");
	else
	{
		fseek(fp,0,SEEK_END);
		fileSize = ftell(fp);

		fseek(fp,0,SEEK_SET);
		fread(cpuMemory,max,1,fp);
		bytes = ftell(fp);

		printf("file: %s\t bytes(integer | Hex): %i | %X)", file,bytes,bytes);

		fclose(fp);
	}
	loaded = true;
	return (int) bytes, loaded;
}

//****************************************************************************

void memDump (unsigned offset)
{
	int i = 0;
	int length = 0;
	unsigned int offsetValue = 0;
	unsigned int offsetIndex = offset;




	printf("Enter the length of bytes you wish to dump: ");

	if (scanf("%X",&length) == 0)
	{
		printf("\n\ninvalid input. Please enter an valid length.\n");
	}


	do{

		offsetValue = offsetIndex;

		printf("\n%04x\t", offsetIndex);


		for(i = 0;i < 0x10; i++, offsetIndex++)
		{
			if((offsetIndex < MEMORY) && (offsetIndex < (offset+length))) printf("%02x ",cpuMemory[offsetIndex]);
			else break;
		}

		offsetIndex = offsetValue;
		printf("\n    \t");

		for(i = 0; i < 16; i++, offsetIndex++)
		{
			if((offsetIndex < MEMORY) && (offsetIndex < (offset+length)))
			{
				if(!isspace(cpuMemory[offsetIndex]) && isprint(cpuMemory[offsetIndex])) printf(" %c ", cpuMemory[offsetIndex]);
				else printf(" . ");
			}
			else break;
		}
		printf("\n");

	} while((offset < MEMORY) && (offsetIndex < (offset+length)));
}

void memModify(unsigned offset)
{
	char offsetBuf[OFFSIZE] = "";


	printf("\nTo end memory modifcation Enter \".\". To continue press enter(no input).\n\n");

	while(1)
	{
		memset(offsetBuf,NULL,sizeof offsetBuf);
		fseek(stdin,0,SEEK_END);
		unsigned int newValue = 0;

		printf("%04x\t %02x - %c", offset, cpuMemory[offset],cpuMemory[offset]);

		if(offset < MEMORY)
		{

			printf("\nEnter a value: ");
			fgets(offsetBuf,sizeof offsetBuf,stdin);
			offsetBuf[strlen(offsetBuf) - 1] = '\0';


			if(strcmp(offsetBuf,".") == 0)
			{
				printf("\nMemory modification has ended.");
				break;
			}

			else if (strcmp(offsetBuf,"") == 0 || strcmp(offsetBuf,"\n") ==0)
			{
				offset++;
				continue;
			} 


			cpuMemory[offset] = (unsigned char) strtoul(offsetBuf,NULL,16);
			offset++;
			if(offset == MEMORY)
			{
				printf("End of memory reached.");
				break;
			}
			else	continue;

		}

	}

}

void zeroRegisters()
{

	//	for(i = 0; i < REGSIZE; i++)
	memset(&regs.reg,NULL, sizeof regs.reg);

	regs.ALU = 0;
	regs.IR0 = 0;
	regs.IR1 = 0;
	regs.IR = 0;
	regs.MAR = 0;
	regs.MBR = 0;
	regs.IR_Active = 0;
	regs.stop = 0;
	regs.sign = 0;
	regs.zero = 0;
	regs.carry = 0;
}

void fetch()
{
	int i;

	regs.MAR = regs.PC;

	/* PUSH 4 BYTES HERE TO MBR */
	for(i = 0; i < sizeof (unsigned long); i++,regs.MAR++)
	{
		regs.MBR = regs.MBR << BYTE;
		regs.MBR +=  cpuMemory[regs.MAR];
	}

	//store contents of MBR into IR
	regs.IR = regs.MBR;

	//Increment PC by 1

	regs.PC += sizeof (unsigned long);

}

//Displays All registers and Flags
void regDisplay()
{
	int i;

	printf("\n");
	for (i = 0; i < REGSIZE ; i++)
	{
		switch(i)
		{

		case r_sp:
			printf("SP:%08lhx\t",regs.SP);
			continue;
		case r_lr:
			printf("LR:%08lhx\t",regs.LR);
			continue;
		case r_pc:
			printf("PC:%08lX\t",regs.PC);
			continue;
		default:
			printf("R%i:%08lhx\t",i,regs.reg[i]);
		}

	}
	printf("IR0:%04lhx\tIR1:%04lhx\tIR_Act:%lhx  Stop:%i SZC:%i%i%i\n",regs.IR0,regs.IR1,
		regs.IR_Active, regs.stop, regs.sign, regs.zero, regs.carry);
	return;
}

void ccrFlags(unsigned short ir16val,unsigned long ALU)
{
	isSignZero(ALU);
	regs.carry = isCarry(regs.ALU,(ir16val & IR8BITVAL),regs.carry);
}

void isSignZero(unsigned long ALU)
{
	if (ALU == 0x00000000)
	{
		regs.zero = set;
		regs.sign = unSet;
	}

	else if (ALU == 0x80000000)
	{
		regs.sign = set;
		regs.zero = unSet;
	}

}

int isCarry(unsigned long op1,unsigned long op2, unsigned C){
	if ((op2== MAX32)&&(C==1)) 
		return(1); // special case where op2 is at MAX32
	return((op1 > (MAX32 - op2 - C))?1:0);
}

void go()
{
	while(!regs.stop)
	{
		instructionCycle();
		regDisplay();
	}
	printf("Stop Flag set: no more instructions");
}

void instructionCycle()
{

	switch(regs.IR_Active)
	{
	case set: 
		printf("\nIR1:%lX\n",regs.IR1);
		regs.IR_Active = unSet;
		decode(regs.IR1);

		break;		 

	case unSet: 
		fetch();
		regs.IR0 = regs.IR >> DBYTE;
		regs.IR1 = regs.IR;

		printf("\nIR0:%lX\n",regs.IR0);
		regs.IR_Active = set;
		decode(regs.IR0);

		break;
	}
}

//Determines what type of instruction is in IR. tests the first 3 MSB in IR0/IR1
void decode(unsigned short ir16val)
{
	switch((ir16val >> SHFTOPMSB) & TESTBITS)
	{
	case B_DATAPROC:
		dataProcess(ir16val);
		break;

	case B_LOADSTORE:
		loadStore(ir16val);
		break;

	case B_IMM1:
	case B_IMM2:
		immExecute(ir16val);

		break;

	case B_CONDBRANCH:
		condBranch(ir16val);
		break;

	case B_PSHPUL:
		break;

	case B_UCONDBRANCH:
		uCondBranch(ir16val);
		break;

	case B_STOP:
		regs.stop = set;
		regDisplay();
		printf("\nStop flag set: no more instructions.");

		break;

	default:
		perror("\nInstruction Type Error: ");
		break;
	}

}

/* (IR0 or IR1) is masked with 0x0FF0(IR8BITVAL) then shifts 4 bytes
*  This operation gets the immediate, performs the imediate instruction
*  and finally stores it into the destination register.
*/
void immExecute(unsigned short ir16val)
{

	switch((ir16val >> OPCODE) & CHKIMMOP)
	{
	case MOVMASK:
		regs.reg[RD] = ((ir16val & IR8BITVAL) >> 4);
		printf("\nA move Instruction has been performed.");
		isSignZero(regs.ALU);

		break;

	case CMPMASK:
		printf("\nA compare Instruction has been performed.");	
		regs.ALU = regs.reg[RD] - ((ir16val & IR8BITVAL) >> sizeof (unsigned long));
		ccrFlags(ir16val,regs.ALU);
		break;

	case ADDMASK:
		//operation to get immediate value
		regs.ALU = regs.reg[RD] + ((ir16val & IR8BITVAL) >> sizeof (unsigned long));
		regs.reg[RD] = regs.ALU;
		ccrFlags(ir16val,regs.ALU);
		printf("\nAn add Instruction has been performed.");
		break;

	case SUBMASK:
		//operation to get immediate value
		regs.ALU = regs.reg[RD] + (~((ir16val & IR8BITVAL) >> sizeof (unsigned long) )+ 1);
		regs.reg[RD] = regs.ALU;
		ccrFlags(ir16val,regs.ALU);

		printf("\nA subtract Instruction has been performed.");
		break;
	}
}

//Loads from/Stores to memory a word or byte.
void loadStore(unsigned short ir16val)
{
	int i;
	unsigned long holder = 0;
	switch((ir16val >> LB_BITS) & LDSTR_OP)
	{

	case STOREWORD:
		printf("\nStore word\n");
		regs.MAR = regs.reg[RN];
		regs.MBR = regs.reg[RD];
		for(i = (sizeof (unsigned long)) - 1; i>= 0; i--,regs.MAR++)
			cpuMemory[regs.MAR]= (regs.MBR >> (BYTE * i));
		break;

	case STOREBYTE:
		printf("\nStore byte\n");
		regs.MAR = regs.reg[RN];
		regs.MBR = regs.reg[RD];
		cpuMemory[regs.MAR] = regs.MBR;
		break;

	case LOADWORD:
		printf("\nLoad word\n");
		regs.MAR = regs.reg[RN];
		regs.MBR = cpuMemory[RD];

		for(i = 0; i < 4; i++)
		{
			printf("\nMBRLOOP: %08lx\n", regs.MBR);
			regs.MBR <<= BYTE;
			regs.MBR += cpuMemory[regs.MAR++];
		}
		printf("\nMBR: %08lx\n", regs.MBR);
		holder = regs.MBR;
		printf("\n holder: %08lx\n", holder);
		regs.reg[RD] = holder;
		printf("\nRD: %08lx\n", regs.reg[RD]);
			
		break;

	case LOADBYTE:
		printf("\nLoad byte\n");
		regs.MAR = (regs.reg[RN] & REGS_LSBMASK);
		regs.MBR = cpuMemory[regs.MAR];
		break;
	}


}

void uCondBranch(unsigned short ir16val)
{
	regs.PC - subPC; 

	switch((ir16val & KBITSHFT) & TESTKBIT)
	{
	case BRA:
		regs.PC = (ir16val & OFFSET12);
		regs.IR_Active = 0;
		break;

	case BRL:
		regs.LR = regs.PC;
		regs.PC = OFFSET12;
		regs.IR_Active = 0;
		break;
	}
}

void condBranch(unsigned short ir16val)
{ 
	signed short branchAddr = ir16val & SIGN8BITVAL;
	signed long offsetBra = (signed long) branchAddr;


	printf("\nbrachaddr: %X\n",branchAddr);
	printf("\noffset: %X\n",offsetBra);

	switch((ir16val >> CONDSHIFT) & TESTCOND)
	{
	case BEQ:
		if(regs.zero == set) 
		{	
			regs.PC += (offsetBra);
		}
		break;

	case BNE:
		if(regs.zero == unSet) 
		{
			regs.PC+= (offsetBra);	
		}
		break;

	case BCS:
		if(regs.carry == unSet)
		{
			regs.PC+= (offsetBra);
		}
		break;

	case BCC:
		if(regs.carry == unSet) 
		{
			regs.PC+= (offsetBra);
		}
		break;

	case BMI:
		if(regs.sign == set) 
		{
			regs.PC+= (offsetBra);
		}
		break;

	case BPL:
		if(regs.sign == unSet) 
		{
			regs.PC+= (offsetBra);
		}
		break;

	case BHI:
		if(regs.carry == set && regs.zero == unSet) 
		{
			regs.PC+= (offsetBra);
		}
		break;

	case BLS:
		if(regs.carry == unSet && regs.zero == set) 
		{
			regs.PC+= (offsetBra);
		}
		break;

	case BAL:
		{
			regs.PC+= (offsetBra);
		}
		break;
	}

	if(regs.IR_Active == 1)
	{
		regs.PC -= subPC; 
		regs.IR_Active = 0;
	}
	regs.IR = 0;
}

void dataProcess(unsigned short ir16val)
{
	int i;
	switch((ir16val >> OPSHFT) & TESTOP)
	{
	case OPAND:
		regs.ALU = (regs.reg[RD] & regs.reg[RN]);
		regs.reg[RD] = regs.ALU;
		ccrFlags(ir16val,regs.ALU);
		break;

	case OPEOR:
		regs.ALU = (regs.reg[RD]  ^ regs.reg[RN]);
		regs.reg[RD] = regs.ALU;
		ccrFlags(ir16val,regs.ALU);
		break;

	case OPSUB:
		regs.ALU = (regs.reg[RD] - regs.reg[RN]);
		regs.reg[RD] = regs.ALU;
		ccrFlags(ir16val,regs.ALU);
		break;

	case OPSXB:
		regs.ALU = ((signed)regs.reg[RN]);
		regs.reg[RD] = regs.ALU;
		ccrFlags(ir16val,regs.ALU);
		break;

	case OPADD:
		regs.ALU = (regs.reg[RD] + regs.reg[RN]);
		regs.reg[RD] = regs.ALU;
		ccrFlags(ir16val,regs.ALU);
		break;

	case OPADC:
		regs.ALU = (regs.reg[RD] + regs.reg[RN] + regs.carry);
		regs.reg[RD] = regs.ALU;
		ccrFlags(ir16val,regs.ALU);
		break;

	case OPLSR:
		regs.ALU = (regs.reg[RD] >> regs.reg[RN]);
		regs.reg[RD] = regs.ALU;
		ccrFlags(ir16val,regs.ALU);
		break;

	case OPLSL:
		regs.ALU = (regs.reg[RD] << regs.reg[RN]);
		regs.reg[RD] = regs.ALU;
		ccrFlags(ir16val,regs.ALU);
		break;

	case OPTST:
		regs.ALU = (regs.reg[RD] & regs.reg[RN]);
		ccrFlags(ir16val,regs.ALU);
		break;

	case OPTEQ:
		regs.ALU = (regs.reg[RD] ^ regs.reg[RN]);
		regs.reg[RD] = regs.ALU;
		ccrFlags(ir16val,regs.ALU);
		break;

	case OPCMP:
		regs.ALU = (regs.reg[RD] - regs.reg[RN]);
		ccrFlags(ir16val,regs.ALU);
		break;

	case OPROR:
		for(i = 0; i < (regs.reg[RD]); i++)
		{
			regs.carry = regs.reg[RN] >> 1;
			if(regs.carry == set) regs.ALU = SIGN32BIT;
		}
		ccrFlags(ir16val,regs.ALU);
		break;

	case OPORR:
		regs.ALU = (regs.reg[RD] | regs.reg[RN]);
		ccrFlags(ir16val,regs.ALU);
		break;

	case OPMOV:
		regs.ALU = regs.reg[RN];
		regs.reg[RD] = regs.ALU;
		ccrFlags(ir16val,regs.ALU);
		break;

	case OPBIC:
		regs.ALU = (regs.reg[RD] &~ regs.reg[RN]);
		regs.reg[RD] = regs.ALU;
		ccrFlags(ir16val,regs.ALU);
		break;

	case OPMVN:
		regs.ALU =  ~ regs.reg[RN];
		regs.reg[RD] = regs.ALU;
		ccrFlags(ir16val,regs.ALU);
		break;
	}

}

void pushPull(registers * regis, unsigned short ir)
{

}
