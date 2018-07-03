#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MEMSIZE 16384 //THIS IS 16KB OR W/E I WANT IT TO BE 
#define	MEMMASK	(MEMSIZE - 1)	
#define MAXLEN 256 //THIS IS SO NO ONE WRITES A HUGE FILENAME 
#define TOTALREGISTERS 16 //For the general purpose registers inside the register struct 
#define	hiBit 0x80000000			// High bit mask for a 32-bit register
#define	MAX32 0xfffffffful
#define	Itype1 (INSTRUCTREG & 0xf000u) //instruction code
#define	Itype2 (INSTRUCTREG2 & 0xf000u) //instruction code

struct REGISTERS 
{
	unsigned long generalRegister[TOTALREGISTERS]; //The 16 [32 bit] registers
	unsigned short signFlag; 
	unsigned short 	carryFlag; 
	unsigned short zeroFlag; 
	unsigned long memBuffReg;		//32-bit memory Buffer Register 
	unsigned long memAddressReg;		//32-bit memory Address Register
	unsigned short instructionRegister1; 	//16-bit instruction Register
	unsigned short instructionRegister2; 	//16-bit instruction Register
	unsigned short stopFlag; 
	unsigned short activeInstructionFlag;
	unsigned char conditionCodeRegister;	
}registers; 

#define	r0		0
#define	R0		(registers.generalRegister[0])
#define	r1		1
#define	R1		(registers.generalRegister[1])
#define	r2		2
#define	R2		(registers.generalRegister[2])
#define	r3		3
#define	R3		(registers.generalRegister[3])
#define	r4		4
#define	R4		(registers.generalRegister[4])
#define	r5		5
#define	R5		(registers.generalRegister[5])
#define	r6		6
#define	R6		(registers.generalRegister[6])
#define	r7		7
#define	R7		(registers.generalRegister[])
#define	r8		8
#define	R8		(registers.generalRegister[8])
#define	r9		9
#define	R9		(registers.generalRegister[9])
#define	r10		10
#define	R10		(registers.generalRegister[10])
#define	r11		11
#define	R11		(registers.generalRegister[11])
#define	r12		12
#define	R12		(registers.generalRegister[12])
#define sp 		13	
#define STACKPOINTER	(registers.generalRegister[13])
#define lr		14
#define LINKREGISTER	(registers.generalRegister[14])
#define pc		15
#define PROGRAMCOUNTER	(registers.generalRegister[15])
#define SIGNFLAG	(registers.signFlag)
#define ZEROFLAG	(registers.zeroFlag)
#define CARRYFLAG	(registers.carryFlag) 
#define MEMBUFREG 	(registers.memBuffReg)
#define MEMADDREG 	(registers.memAddressReg)


#define INSTRUCTREG	(registers.instructionRegister1)
#define INSTRUCTREG2	(registers.instructionRegister2) 
#define STOPFLAG 	(registers.stopFlag)
#define ACTIVEINSTFLAG	(registers.activeInstructionFlag)
#define	CONDITIONCODEREGISTER	(registers.conditionCodeRegister)




// CCR flags
#define	ZERO			registers.zeroFlag			// zero flag
#define	CARRY			registers.carryFlag			// carry flag
#define	SIGN		registers.signFlag			// negative flag



//instructions
#define	loadOrStore	0x2000u  //code
#define	loadOrStoreBit ((IR & 0x0800u) >> 0xb) //value

#define	hiMSB32	((ALU & msBit32) >> 31)
#define	msBit32	0x80000000u









#define branchCondition1 	((INSTRUCTREG & 0x0f00u) >> 8u)
#define branchCondition2 	((INSTRUCTREG2 & 0x0f00u) >> 8u)
#define	offset8b		(INSTRUCTREG & 0x00ffu)	
#define	offset8b2		(INSTRUCTREG2 & 0x00ffu)

#define	offset12b 		(INSTRUCTREG & 0x0fffu)
#define	offset12b2 		(INSTRUCTREG2 & 0x0fffu)

#define	OPcode 			(INSTRUCTREG & 0x0f00u)	  
#define	OPcode2 		(INSTRUCTREG2 & 0x0f00u)	  


#define Rd  			(registers.generalRegister[INSTRUCTREG & RdMask])

#define Rd2 			(registers.generalRegister[INSTRUCTREG2 & RdMask])
#define Rn 			(registers.generalRegister[((INSTRUCTREG & 0x00f0) >> 4)])
#define Rn2 			(registers.generalRegister[((INSTRUCTREG2 & 0x00f0) >> 4)])
#define Imediate 		((INSTRUCTREG & imvMask)>> 4u )  
#define Imediate2 		((INSTRUCTREG2 & imvMask)>> 4u)  
#define	sBit			((INSTRUCTREG & 0X0100u) >> 0x10) 
#define	sBit2			((INSTRUCTREG2 & 0X0100u) >> 0x10)

#define	RnLSBb			(Rn & 0x000000ff)
#define	RnLSBb2			(Rn2 & 0x000000ff)
#define  Reglist		(INSTRUCTREG & 0X00ff)
#define  Reglist2		(INSTRUCTREG2 & 0X00ff)
#define	RdNum			(INSTRUCTREG & 0x000f)
#define	RdNum2			(INSTRUCTREG2 & 0x000f)

#define  LDST       		((INSTRUCTREG & 0x0800u) >> 0xb)        // Load/Store bit
#define  HIGH       		((INSTRUCTREG & 0x0400u) >> 0xa)        // High/Low bit
#define  BYTE        		((INSTRUCTREG & 0x0400u) >> 0xa)        // Byte/Word bit
#define  PSHPUL        		((INSTRUCTREG & 0x0100u) >> 0x8)        // Push/Pull bit

#define  LDST2        		((INSTRUCTREG2 & 0x0800u) >> 0xb)        // Load/Store bit
#define  HIGH2       		((INSTRUCTREG2 & 0x0400u) >> 0xa)        // High/Low bit
#define  BYTE2       		((INSTRUCTREG2 & 0x0400u) >> 0xa)        // Byte/Word bit
#define  PSHPUL2        	((INSTRUCTREG2 & 0x0100u) >> 0x8)        // Push/Pull bit



#define	rd(num) 		(registers.generalRegister[(num)])
#define	rn(num)  		(registers.generalRegister[(num)])
// CCR condition codes
#define	EQ			0
#define	NE			1u
#define	CS			2u
#define	CC			3u
#define	MI			4u
#define	PL			5u
#define	HI			6u
#define	LS			7u
#define	AL			8u	

// data processing op-codes 
#define	AND		0x0000u
#define	EOR		0x0100u
#define	SUB		0x0200u
#define	SXB		0x0300u
#define	ADD		0x0400u
#define	ADC		0x0500u
#define	LSR		0x0600u
#define	LSL		0x0700u
#define	TST		0x0800u
#define	TEQ		0x0900u
#define	CMP		0x0a00u
#define	ROR		0x0b00u
#define	ORR		0x0c00u
#define	MOV		0x0d00u
#define	BIC		0x0e00u
#define	MVN		0x0f00u
#define	STP		0x0e00u

// Instruction Types
#define	DPinst		0					// data processing instruction
#define	BRA		0xe000u
#define	BRL		0xf000u
#define	cndBr		0xc000u			// conditional branch
#define	LdStr		0x2000u			// load/store instruction
#define  pushOrPull		0xb000u			// push/pull stack
#define	STOP		0xE000



//immediate Instructions
#define	mov		0x4000
#define	cmp		0x5000
#define	add		0x6000
#define	sub		0x7000

#define	RdMask		0x000f	// mask for destination register
#define	imvMask	0x0ff0	// immediate value mask




// Bit masks for push/pull register list 
#define bit0		1u
#define bit1		0x02u
#define bit2		0x04u
#define bit3		0x08u
#define bit4		0x10u
#define bit5		0x20u
#define bit6		0x40u
#define bit7		0x80u

//Values
#define	lsBit 1u
#define	msBit32	0x80000000u
#define	QWORD		8u
#define	WORD		0x20u			// 32-bits
#define	HWORD		0x10u
#define	word		4u			// four bytes = word	


void displayRegisters (void);
void conditionalBranch (void);
int iscarry (unsigned long op1, unsigned long op2, unsigned c);
void writeIntoMemory (unsigned long memAddr, unsigned long regNum, unsigned short byteflag);
void setNorZflag (void);
int iscarry (unsigned long op1, unsigned long op2, unsigned c);
void loadIntoMemory (unsigned long memAddr,  unsigned long regNum, unsigned short byteflag);
void shiftRd (void);
unsigned short pp_regNum(unsigned short regListMask);
void pull (void);
void push (void);
void execute (void);
void fetch (void);
void trace (void);
void zeroAllRegisters (void);
int getOffset(int * finalAddress);
int getLength(int * finalLengthOfMessage);
void memoryDump(void * pointerToMemory, unsigned addressInMemory, unsigned lengthOfMessage);
void go (void);
int writeFile(void *memory);
int loadFile (void * memory, unsigned int max);
char *trimWhiteSpace(char *str);
int memoryModify(void *pointerToMemory, unsigned addressInMemory);
void displayFunctions();
void menuFunction();




unsigned char memory [MEMSIZE];

	int input;
	char option;
	char buf[BUFSIZ];
	char buf2[BUFSIZ];
	char bufferHolder [BUFSIZ];

	int addressInMemory;
	int lengthOfMessage;
	char exitCharacterForMemoryModify = '.';
	int instructionRegister0Done;
	
	unsigned long ALU;
	
	/******************************************************************
	  displayRegisters()- Displays the current values for the registers
	 *******************************************************************/	
	void displayRegisters (void)
	{
		int i;
		for (i=0; i < TOTALREGISTERS; i++)
		{
			if (i != 0)
			{
				(i % 4u == 0) ? printf("\n"): 0 ;
			}
			switch (i)
			{
				case sp: 
					printf("sp=%08lx ", STACKPOINTER);							
					break;
				case lr: 
					printf("lr=%08lx ", LINKREGISTER);
					break;
				case pc:
					printf("pc=%08lx ", PROGRAMCOUNTER);
					break;
				default: 
					printf("r%x=%08lx ", i, registers.generalRegister[i]);
			}
		}
		printf("\nmar=%08lx ", MEMADDREG);
		printf("mbr=%08lx ", MEMBUFREG);
		printf("ir1=%04hx  ", INSTRUCTREG);
		printf("ir2=%04hx  ", INSTRUCTREG2);	
		printf("CCR=%02hx\n\n", CONDITIONCODEREGISTER);
		
		(SIGN == 1) ? printf("N "): printf("  ");
	 	(ZERO == 1) ? printf("Z "): printf("  ");
		(CARRY == 1) ? printf("C\n"): printf("\n");

		printf("\n");
	
	}
	/******************************************************************
	  conditionalBranch()- When called, it must check the CCR codes and 
	  executes a conditional branch 
	 *******************************************************************/		

	void conditionalBranch (void)
	{
		if (instructionRegister0Done == 0)
		{
			//PROGRAMCOUNTER-=2;
			switch (branchCondition1)
			{
				case EQ: 
					if (ZERO == 1)
					{
						PROGRAMCOUNTER += (signed char) offset8b;
					}
					break;

				case NE: 
					if (ZERO == 0)
					{
						PROGRAMCOUNTER += (signed char) offset8b;
					}
					break;

				case CS: 
					if (CARRY == 1)
					{
						PROGRAMCOUNTER += (signed char) offset8b;
					}
					break;

				case CC: 
					if (CARRY == 0)
					{
						PROGRAMCOUNTER += (signed char) offset8b;
					}
					break;

				case MI:
					if (SIGN == 1)
					{
						PROGRAMCOUNTER += (signed char) offset8b;
					}
					break;
	
				case PL: 
					if (SIGN == 0)
					{
						PROGRAMCOUNTER += (signed char) offset8b;
					}
					break;

			 	case HI: 
					if ((CARRY == 1) && (ZERO == 0))
					{
						PROGRAMCOUNTER += (signed char) offset8b;
					}
					break;
				case LS: 
					if ((CARRY == 0) && (ZERO == 1))
					{
						PROGRAMCOUNTER += (signed char) offset8b;
					}
					break;

			 	case AL: 
					PROGRAMCOUNTER += (signed char) offset8b;
			}
		}
		else
		{ 
			//PROGRAMCOUNTER-=2;
			switch (branchCondition2)
			{
				case EQ: 
					if (ZERO == 1)
					{
						PROGRAMCOUNTER += (signed char) offset8b2;
					}
					break;

				case NE: 
					if (ZERO == 0)
					{
						PROGRAMCOUNTER += (signed char) offset8b2;
					}
					break;

				case CS: 
					if (CARRY == 1)
					{
						PROGRAMCOUNTER += (signed char) offset8b2;
					}
					break;

				case CC: 
					if (CARRY == 0)
					{
						PROGRAMCOUNTER += (signed char) offset8b2;
					}
					break;

				case MI:
					if (SIGN == 1)
					{
						PROGRAMCOUNTER += (signed char) offset8b2;
					}
					break;
	
				case PL: 
					if (SIGN == 0)
					{
						PROGRAMCOUNTER += (signed char) offset8b2;
					}
					break;

			 	case HI: 
					if ((CARRY == 1) && (ZERO == 0))
					{
						PROGRAMCOUNTER += (signed char) offset8b2;
					}
					break;
				case LS: 
					if ((CARRY == 0) && (ZERO == 1))
					{
						PROGRAMCOUNTER += (signed char) offset8b2;
					}
					break;

			 	case AL: 
					PROGRAMCOUNTER += (signed char) offset8b2;
			 }

		}
	}

	/******************************************************************
	  iscarry()- determine if carry is generated by addition: op1+op2+C
	  C can only have value of 1 or 0.
	 *******************************************************************/		
	int iscarry (unsigned long op1, unsigned long op2, unsigned c)
	{
		 if ((op2 == MAX32) && (c==1))
			  return(1); // special case where op2 is at MAX32
		 return((op1 > (MAX32 - op2 - c))?1:0);
	}

	/******************************************************************
	  setNorZflag()- Checks the ALU status, and checks/sets the N and Z flags
	 *******************************************************************/	
	void setNorZflag (void)
	{
		 SIGN = (hiMSB32 == 1) ? 1 : 0;
		 ZERO = (ALU == 0) ? 1 : 0;
	}
	/******************************************************************
	  writeIntoMemory()- Writes data from a register into memory
	 *******************************************************************/	
	void writeIntoMemory (unsigned long memAddr, unsigned long regNum, unsigned short byteflag)
	{
		signed short byteCount;
		MEMADDREG = memAddr;
		MEMBUFREG = rn(regNum);
		 if (byteflag == 1) 	// store lsbyte of Rd 
		 {
			  (memory[ MEMADDREG & MEMMASK ]) = (unsigned char) MEMBUFREG;
			  return;
		 }

		 for (byteCount = 4u - 1 ; byteCount >= 0; byteCount--) // store word
		 {
			  (memory[ MEMADDREG & MEMMASK ]) = (unsigned char) (MEMBUFREG >> (8u * byteCount));
			  ++MEMADDREG;
		 }
	}
	/******************************************************************
	  loadIntoMemory()- Loads data from memory into a register
	 *******************************************************************/		
	void loadIntoMemory (unsigned long memAddr,  unsigned long regNum, unsigned short byteflag)
	{
		unsigned short byteCount;
		MEMADDREG = memAddr;
		MEMBUFREG = (unsigned long) (memory[ MEMADDREG & MEMMASK ]);

		if (byteflag == 1)
		{
			rd(regNum) = MEMBUFREG;
			return;		// loads 1 byte to register
		}

	
		for (byteCount = 1; byteCount < 4u; byteCount++)
		{
			MEMBUFREG <<= 8u;
			++MEMADDREG;
			MEMBUFREG |= (unsigned long) (memory[ MEMADDREG & MEMMASK ]);
		}
		rd(regNum) = MEMBUFREG;
	} 	
	

	/******************************************************************
	  shiftRd()- Preforms the logical shift to Rd. The sBit determines if
	  it will shift it left or right
	 *******************************************************************/		

	void shiftRd (void)
	{
		if (instructionRegister0Done == 0)
		{
			ALU = Rd;
			if (sBit == 0) // shift right
			{
				CARRY = ((ALU >> (Rn - 1) ) & lsBit);
				ALU >>= Rn;
			}
			else				// shift left
			{
				CARRY = ((ALU << (Rn - 1) ) & msBit32);
				ALU <<= Rn;
			}
			Rd = ALU;
			setNorZflag();
		}
		else 
		{
			ALU = Rd2;
			if (sBit2 == 0) // shift right
			{
				CARRY = ((ALU >> (Rn2 - 1) ) & lsBit);
				ALU >>= Rn2;
			}
			else				// shift left
			{
				CARRY = ((ALU << (Rn2 - 1) ) & msBit32);
				ALU <<= Rn2;
			}
			Rd2 = ALU;
			setNorZflag();
		}
	}
	
	/******************************************************************
	   pp_regNum()- Returns a register number from a push or pull register list
	 *******************************************************************/	
	unsigned short pp_regNum(unsigned short regListMask)
	{
		unsigned short regNum = 0;
		
		if (instructionRegister0Done == 0)
		{
			switch (regListMask)
			{
				case bit0: regNum = (HIGH)? r8 : r0; 
						 	  break;
				case bit1: regNum = (HIGH)? r9 : r1;
						 	  break;
				case bit2: regNum = (HIGH)? r10 : r2;
						 	  break;
				case bit3: regNum = (HIGH)? r11 : r3;
						 	  break;
				case bit4: regNum = (HIGH)? r12 : r4;
						 	  break;
				case bit5: regNum = (HIGH)? sp : r5;
						 	  break;
				case bit6: regNum = (HIGH)? lr : r6;
						 	  break;
				case bit7: regNum = (HIGH)? pc : r7;
						 	  break;
			}
			return regNum;	
		}
		else 
		{
			switch (regListMask)
			{
				case bit0: regNum = (HIGH2)? r8 : r0; 
						 	  break;
				case bit1: regNum = (HIGH2)? r9 : r1;
						 	  break;
				case bit2: regNum = (HIGH2)? r10 : r2;
						 	  break;
				case bit3: regNum = (HIGH2)? r11 : r3;
						 	  break;
				case bit4: regNum = (HIGH2)? r12 : r4;
						 	  break;
				case bit5: regNum = (HIGH2)? sp : r5;
						 	  break;
				case bit6: regNum = (HIGH2)? lr : r6;
						 	  break;
				case bit7: regNum = (HIGH2)? pc : r7;
						 	  break;
			}
			return regNum;	
		}
		
	}
	/******************************************************************
	   pull()- Pulls data from the stack and writes it to registers on a 
	   register list
	 *******************************************************************/	
	void pull (void)
	{
		unsigned short regCount, regListMask = 0x01u;

		for (regCount = 0; regCount < QWORD; regCount++, regListMask <<= 1) // check 8 register bits
		{
			if (Reglist & regListMask)
			{printf("\treg%d\n",pp_regNum(regListMask));
				loadIntoMemory(STACKPOINTER, pp_regNum(regListMask), 0);
				STACKPOINTER += word;	// increament SP by 4 bytes
			}
		}

		if (instructionRegister0Done == 0)
		{
			if (PSHPUL)		// extra pull
			{
				loadIntoMemory(STACKPOINTER, pc, 0);
				STACKPOINTER += word;
			}
		}
		else
		{
			if (PSHPUL2)		// extra pull
			{
				loadIntoMemory(STACKPOINTER, pc, 0);
				STACKPOINTER += word;
			}
		}
	}


	/******************************************************************
	   push()- Pushes a register list on to the stack
	 *******************************************************************/	
	void push (void)
	{
		unsigned short regCount, regListMask = 0x80u;
		
		if (instructionRegister0Done == 0)
		{
			if (PSHPUL)		// extra push
			{
				STACKPOINTER -= word;
				writeIntoMemory(STACKPOINTER, lr, 0);
			}
		}
		else	
		{
			if (PSHPUL2)		// extra push
				{
					STACKPOINTER -= word;
					writeIntoMemory(STACKPOINTER, lr, 0);
				}
		}
		for (regCount = 0; regCount < QWORD; regCount++, regListMask >>= 1) // check 8 register bits
		{
			if (Reglist & regListMask)
			{
				STACKPOINTER -= word;	// decreament SP by 4 bytes
				writeIntoMemory(STACKPOINTER, pp_regNum(regListMask), 0);
			}
		}
	}		

						
	/******************************************************************
	   execute()- Decodes and executes a fetched instruction
	 *******************************************************************/	
	void execute (void)
	{
		if (instructionRegister0Done == 0)
		{
			switch (INSTRUCTREG & 0xf000)
			{
				case BRA: 
					PROGRAMCOUNTER = (unsigned) offset12b;
					break; 
				case cndBr: 					
					conditionalBranch();
					break;
				case BRL: LINKREGISTER = PROGRAMCOUNTER;
					PROGRAMCOUNTER = (unsigned) offset12b;
					break;
				case DPinst:
					switch (OPcode)
					{
						case AND:
							ALU = Rd & Rn;
							Rd = ALU;
							break;
								
						case EOR: 
							ALU = Rd ^ Rn;
							Rd = ALU;
							break;
			
						case SUB: 
							ALU = Rd + ~Rn + 1;
							CARRY = (unsigned short) iscarry(Rd, ~Rn, 1);
							Rd = ALU;
							break;
		
						case SXB: 
							ALU = (signed) RnLSBb;
							Rd = ALU;
							break;
						case ADD: 
							ALU = Rd + Rn;
							CARRY = (unsigned short) iscarry(Rd, Rn, 0);
							Rd = ALU;
							break;
						case ADC: 
							ALU = Rd + Rn + iscarry(Rd, Rn, CARRY);
						 	CARRY = (unsigned short) iscarry(Rd, Rn, CARRY);
							Rd = ALU;
							break;
						case LSR: 
							shiftRd();
						 	break;
						case LSL: 
							shiftRd();
							break;
						case TST:
							ALU = Rd & Rn;
						 	break;
						case TEQ:
							ALU = Rd | Rn;
							break;
						case CMP:
							ALU = Rd + ~Rn + 1;
							Rd = ALU;
							break;
						case ROR:
							ALU = Rd >> Rn;
							Rd = ALU;
							break;
						case ORR:
							ALU = Rd | Rn;
						 	Rd = ALU;
						 	break;
						case MOV:
							ALU = Rn;
							Rd = ALU;
							break;
						case BIC: 
							ALU = Rd & (~Rn);
							break;
						case MVN:
							ALU = ~Rn;
							Rd = ALU;
							break;
					}
					setNorZflag();
					break;


//				#define Rd			(registers.generalRegister[INSTRUCTREG & RdMask])
//				#define Imediate 		((INSTRUCTREG & imvMask))  
//				#define imvMask		0x0ff0	// immediate value mask
//				#define RdMask		0x000f	// mask for destination register


				case mov:					
					printf("\nMOV\n");							 										
					
					ALU = 	Imediate;
					Rd = ALU;

					setNorZflag();					
					break;
				case add:
					printf("\nADD\n");					
					ALU = Rd + Imediate;
					CARRY = iscarry(Rd, Rn, 0);

					Rd = ALU;
					setNorZflag();
					break;
				case sub:
					printf("\nSUB\n");									
					ALU = Rd + ~Imediate + 1;
					CARRY = iscarry(Rd, ~Rn, 1);
					Rd = ALU;
					setNorZflag();
					break;

				case cmp:
					printf("\nCOMP\n");					
					ALU = Rd + ~Imediate + 1;
					CARRY = iscarry(Rd, ~Imediate, 1);
					setNorZflag();	
					break;

				case loadOrStore:
					switch (((INSTRUCTREG & 0x0800u) >> 0xb))
					{
						case 0: 
							writeIntoMemory(Rn, RdNum2, BYTE);
							break; 
						case 1: 
							loadIntoMemory(Rn, RdNum2, BYTE);
							break;  							
					}
					break;	
				case pushOrPull:
					switch (LDST)
					{
						case 0: 
							push();
							break;
						case 1: 
							pull();
							break;
					}
				
			}
			instructionRegister0Done = 1;
		}
		else 
		{
			switch (INSTRUCTREG2 & 0xf000)
			{
				case BRA: 
					PROGRAMCOUNTER = (unsigned) offset12b2;
					break; 
				case cndBr: 					
					conditionalBranch();
					break;
				case BRL: LINKREGISTER = PROGRAMCOUNTER;
					PROGRAMCOUNTER = (unsigned) offset12b2;
					break;
				case DPinst:
					switch (OPcode2)
					{
						case AND:
							ALU = Rd2 & Rn2;
							Rd2 = ALU;
							break;
								
						case EOR: 
							ALU = Rd2 ^ Rn2;
							Rd2 = ALU;
							break;
			
						case SUB: 
							ALU = Rd2 + ~Rn2 + 1;
							CARRY = (unsigned short) iscarry(Rd2, ~Rn2, 1);
							Rd2 = ALU;
							break;
		
						case SXB: 
							ALU = (signed) RnLSBb2;
							Rd2 = ALU;
							break;
						case ADD: 
							ALU = Rd2 + Rn2;
							CARRY = (unsigned short) iscarry(Rd2, Rn2, 0);
							Rd2 = ALU;
							break;
						case ADC: 
							ALU = Rd2 + Rd2 + iscarry(Rd2, Rn2, CARRY);
						 	CARRY = (unsigned short) iscarry(Rd2, Rn2, CARRY);
							Rd2 = ALU;
							break;
						case LSR: 
							shiftRd();
						 	break;
						case LSL: 
							shiftRd();
							break;
						case TST:
							ALU = Rd2 & Rn2;
						 	break;
						case TEQ:
							ALU = Rd2 | Rn2;
							break;
						case CMP:
							ALU = Rd2 + ~Rn2 + 1;
							Rd2 = ALU;
							break;
						case ROR:
							ALU = Rd2 >> Rn2;
							Rd2 = ALU;
							break;
						case ORR:
							ALU = Rd2 | Rn2;
						 	Rd2 = ALU;
						 	break;
						case MOV:
							ALU = Rn2;
							Rd2 = ALU;
							break;
						case BIC: 
							ALU = Rd2 & (~Rd2);
							break;
						case MVN:
							ALU = ~Rn2;
							Rd2 = ALU;
							break;
					}
					setNorZflag();
					break;

				case 0x4000:					
					printf("\nMOV\n");							 										
					ALU = 	Imediate2;
					Rd2 = ALU;
					setNorZflag();					
					break;
				case 0x6000:
					printf("\nADD\n");					
					ALU = Rd2 + Imediate2;
					CARRY = iscarry(Rd2, Rd2, 0);
					Rd2 = ALU;
					setNorZflag();
					break;
				case 0x7000:
					printf("\nSUB\n");									
					ALU = Rd2 + ~Imediate2 + 1;
					CARRY = iscarry(Rd2, ~Rd2, 1);
					Rd2 = ALU;
					setNorZflag();
					break;

				case 0x5000:
					printf("\nCOMP\n");					
					ALU = Rd2 + ~Imediate2 + 1;
					CARRY = iscarry(Rd2, ~Imediate2, 1);
					setNorZflag();	
					break;

				case loadOrStore:
					switch (((INSTRUCTREG & 0x0800u) >> 0xb))
					{
						case 0: 
							writeIntoMemory(Rd2, RdNum, BYTE);
							break; 
						case 1: 
							loadIntoMemory(Rd2, RdNum, BYTE);
							break;  							
					}
					break;	
				case pushOrPull:
					switch (LDST2)
					{
						case 0: 
							push();
							break;
						case 1: 
							pull();
							break;
					}
				
			}
			instructionRegister0Done = 0;

		}
	}
	
	
	/******************************************************************
	   fetch()- Retrives two instructions from memory 
	 *******************************************************************/	
	void fetch (void)
	{
		registers.memAddressReg = registers.generalRegister[pc];
		registers.memBuffReg = (unsigned long) memory [registers.generalRegister[pc]++];
		registers.memBuffReg <<= 0x8;
		registers.memBuffReg |= (unsigned long) memory [registers.generalRegister[pc]++];
			

		INSTRUCTREG = (unsigned short) registers.memBuffReg;
		INSTRUCTREG |= (unsigned short) registers.memBuffReg;

		registers.memBuffReg = (unsigned long) memory [registers.generalRegister[pc]++];
		registers.memBuffReg <<= 0x8;
		registers.memBuffReg |= (unsigned long) memory [registers.generalRegister[pc]++];
			

		INSTRUCTREG2 = (unsigned short) registers.memBuffReg;
		INSTRUCTREG2 |= (unsigned short) registers.memBuffReg;

		displayRegisters();
	}

	/******************************************************************
	   trace()- Preforms a fetch and executes the instructions in memory
	 *******************************************************************/	
	void trace (void)
	{
		fetch();
		execute();
		displayRegisters();
		execute();
	}

	/******************************************************************
	   zeroAllRegisters()- Makes all registers equal to zero
	 *******************************************************************/	
	void zeroAllRegisters (void)
	{
		memset(&registers, 0, sizeof registers);
	}

	/******************************************************************
	   getOffset()- Asks user for address to start memory dump
	 *******************************************************************/	
	int getOffset(int * finalAddress)
	{
		unsigned char inputAddress[MAXLEN];
		printf("Enter offset address:");
		fgets(inputAddress, MAXLEN, stdin);
		sscanf(inputAddress, "%X", finalAddress);
	}

	/******************************************************************
	   getLength()- Asks user for how long memory dump should go up to
	 *******************************************************************/
	int getLength(int * finalLengthOfMessage)
	{
		unsigned char inputLength[MAXLEN];
		printf("Enter length of message:");
		fgets(inputLength, MAXLEN, stdin);
		sscanf(inputLength, "%X", finalLengthOfMessage);
	}

	/******************************************************************
	   memoryDump()- Asks user for how long memory dump should go up to
	 *******************************************************************/
	void memoryDump(void * pointerToMemory, unsigned addressInMemory, unsigned lengthOfMessage)
	{
		unsigned int startPoint, index = addressInMemory;
		unsigned char * addressToMemory = pointerToMemory;
		unsigned int totalLengthOfMessage = addressInMemory + lengthOfMessage;

		printf("Address: 0x%04X\n",addressInMemory);
		printf("Length: 0x%04X\n", lengthOfMessage);

		while (index < (totalLengthOfMessage))
		{
			unsigned int indexStartingPoint = index;
			printf("%04x\t", index);
			for (startPoint = 0; startPoint < 16; startPoint++, index++)
			{
				if (index < (totalLengthOfMessage))
				{
					printf(" %02X", addressToMemory[index]);

				}
				else break;
			}
	
			index = indexStartingPoint;
			printf("\n  \t");
			
			for (startPoint = 0; startPoint < 16; startPoint++, index++)
			{
				if (index < (totalLengthOfMessage))
				{
					if(!isspace(addressToMemory[index]) && isprint(addressToMemory[index]))
					{	
						printf(" %c ", addressToMemory[index]);
					}
					else printf(" . ");
				}
				else break;				
			}
			printf("\n");
		}
		
	}
	/******************************************************************
	   go()- Runs the program stored in memory
	 *******************************************************************/
	void go (void)
	{
		zeroAllRegisters();		// execution start at memory location 0
		do {
			fetch();
			if (Itype1 == STOP)
			{
				printf("\nbreak point reached... \n\n") ; 
				break;
			}	  
			execute();

			if (Itype2 == STOP)
			{
				printf("\nbreak point reached... \n\n") ; 
				break;
			}	
			execute();			 
		} while(1);

		displayRegisters();
	}

	/******************************************************************
	   writeFile()- Creates a file with a size and name the user specifies 
	 *******************************************************************/
	int writeFile(void *memory)
	{
		unsigned char fname[MAXLEN];
		unsigned char numberOfBytes[MAXLEN];
		int writeByte;
		int writeToFile;
		
		/*get file name*/
		printf("Enter file name: ");
		fgets (fname, MAXLEN, stdin);
		fname[strlen(fname) - 1] = '\0';
		printf("***%s*** -> opening file ...\n", fname);

		FILE *f = fopen(fname, "w");
		if (f == NULL)
		{
			printf("Error opening file: Invalid Name \n");
			fclose(f);
			return 1;
		}
		
		printf("Opening %s", fname);
		printf("Enter number of bytes: ");
		fgets(numberOfBytes, MAXLEN, stdin);
		int finalByte = atoi(numberOfBytes);
		
		if (finalByte < 0)
		{
			printf("\nNot a valid size\n");
			fclose(f);
			return 1;
		}	
		if (finalByte > MEMSIZE)
		{	
			printf("Size too big. Closing file\n");	
			fclose(f);
			return 1;
		}
		
		writeToFile = fwrite (memory, sizeof(char), finalByte, f);
		printf("Writing a file with %d bytes\n", writeToFile);
		fclose(f);
	}

	
	/******************************************************************
	 loadFile()- Loads a file in the same directory as the cpu.c into memory
	 *******************************************************************/
	int loadFile (void * memory, unsigned int max)
	{
		FILE *fptr;
		int byteCount = 0;
		unsigned char fname[MAXLEN];
		
		//Get file name		
		printf("Enter file name: ");
		fgets(fname, MAXLEN, stdin);
		fname[strlen(fname) - 1] = '\0';
		printf("%s -> opening file...\n", fname);

		//Open file in Read mode
		if ((fptr = fopen(fname, "rb")) == NULL)
		{
			perror("OPPS!");
			return -1;
		}
		else
		{
			byteCount = fread(memory, 1, MEMSIZE, fptr);
			printf("Byte Count (hex): %X\n", (unsigned int)byteCount);
			printf("Byte Count (dec): %d\n", (unsigned int)byteCount);
			printf("Truncated: ");
			if (byteCount ==max)
				printf("Yes\n");
			else
				printf("No\n");
			fclose(fptr);
		}
		return byteCount;
	}
	/******************************************************************
	 trimWhiteSpace()- Removes white spaces from a string
	 *******************************************************************/
	char *trimWhiteSpace(char *str)
	{
		char *end;
		//Trim leading space
		while(isspace(*str)) str++;
		
		if (*str == 0) // All spaces?
			return str;
		//Trim trailing Space
		end = str + strlen(str) - 1;
		while(end > str && isspace(*end)) end--;

		*(end+1) = 0;
		return str;

	}

	
	/******************************************************************
	 memoryModify()- Allows user to change byte by byte what is in memory
	 *******************************************************************/
	int memoryModify(void *pointerToMemory, unsigned addressInMemory)
	{
		char inputForMemoryModify[BUFSIZ];
		int finalString;
	
		unsigned int startPoint, index = addressInMemory;
		unsigned char * addressToMemory = pointerToMemory;
	
		printf("Address: 0x%04X\n",addressInMemory);
		printf("Value:   %X\n",addressToMemory[index]);
		
		while(1)
		{
			printf("Enter a new HEX value: ");
			fgets (inputForMemoryModify, 4, stdin);
			inputForMemoryModify[strlen(inputForMemoryModify) - 1] = '\0';

			if (*inputForMemoryModify == '.')
			{
				printf("\n");
				return 1;
			}
			sscanf(inputForMemoryModify, "%X", &finalString);
			printf("You entered: ***%X***\n",finalString);
			
			addressToMemory[index] = finalString;
			printf("New Value:  %c\n", addressToMemory[index]);
			index++;
		}
	}
	/******************************************************************
	 displayFunctions()- Displays help menu
	 *******************************************************************/
	void displayFunctions()
	{
		printf("These are the commands you can enter:\n");
		printf("d - Memory Dump\n");
		printf("g - Run the loaded Program\n");
		printf("l - load a file into Memory\n");
		printf("m - Modify what is in memory\n");
		printf("q - Quit\n");
		printf("r - Display Registers\n");
		printf("t - Trace through program loaded in memory\n");
		printf("w - Write a new file\n");
		printf("z - Reset all registers to zero\n");
		printf("h,? - Display this help page ");
	}
	
	/******************************************************************
	 menuFunction()- Presents user with options to select
	 *******************************************************************/
	void menuFunction()
	{
		zeroAllRegisters();	
		displayRegisters();
		while(1)
		{
			fflush(stdout);
			printf(">:");
			
			if (fgets (buf, sizeof(buf), stdin) != NULL)
			{
				bufferHolder[1] =* trimWhiteSpace(buf);	
				option = (char)(bufferHolder[1]);
				option = (tolower(option));
			}
			
			switch (option)
			{
				case 'd':
					printf("Memory Dump\n");
					getOffset(&addressInMemory);
					getLength(&lengthOfMessage);
					memoryDump(&memory,addressInMemory, lengthOfMessage);
					break;
				case 'q':
					exit(0);
				case 'h':
					displayFunctions();
					break;
				case 'g':
					printf("Run Loaded Program");	
					go();
					break;
				case 'l':
					loadFile(memory, sizeof(memory));
					break;
				case 'm':
					getOffset(&addressInMemory);
					memoryModify(&memory, addressInMemory);
					break;
				case 'r':
					displayRegisters();
					break;
				case 't':
					trace();
					break;
				case 'w':
					writeFile(memory);
					break;
				case 'z':
					zeroAllRegisters();				
					break;
				case '?':
					displayFunctions();
					break;
			}
		}
	}	



int main ()
{
					
			
	printf("Name: Abel Delgado\n");
	printf("Enter h for help\n");
	menuFunction();
}
