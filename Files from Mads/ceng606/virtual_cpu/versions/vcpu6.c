/*
 * Madzivire Chikwanda
 * 03/04/2014, version 5.0
 *
 * vcpu.c is an implementation of a virtual CPU user interface that has
 *	the following features:
 * - h/? command, displays a list of commands for help
 * - l/L command, loads a file into memory
 * - d/D command, does a memory dump
 * - t/T command, for instruction tracing
 * - executes immediate instructions
 * - g/G command, to run a program 
 *
 * The following assumptions were made for implementing memory & the register
 * set:
 * - memory is implemented using an unsigned char array & referenced memory is
 *	  one byte in size 
 * - for 32-bit registers, an unsigned long is used
 * - 16-bit registers are implemented using the unsigned short type
 *
 * The following instruction types and flags are implemented in this version:
 *		a.		Data Processing Instructions
 *		b.		Branch Instructions
 *					i.	Conditional Branch
 *					ii.Unconditional Branch
 *		c.		Immediate Instructions
 *		d.		Load/Store Instructions
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define	AUTHOR	"Madzivire Chikwanda"

/* Main Memory Macros */
#define	MEMSIZE	0x4000u
#define	MEMMASK	(MEMSIZE - 1)			  // memory mask
#define  MEM		(mem[ MAR & MEMMASK ]) // referenced memory

/* Constants */
#define	MAXLEN	0x100u		// max file name size
#define	MAX32		0xfffffffful	// max 32-bit value
#define	READ		"rb"			// read file in binary mode
#define	VER		"5.0"
#define	WORD		0x20u			// 32-bits
#define	HWORD		0x10u
#define	QWORD		8u
#define	word		4u				// four bytes = word
#define	nibble	4u
#define	_256bits	0x100u
#define	hiMSB32	((ALU & msBit32) >> 31u)

/* Bit masks for push/pull register list */
#define bit0		1u
#define bit1		0x02u
#define bit2		0x04u
#define bit3		0x08u
#define bit4		0x10u
#define bit5		0x20u
#define bit6		0x40u
#define bit7		0x80u

/* Instruction Types */
#define	Itype		(IR & 0xf000u)	// instruction type
#define	DPinst	0					// data processing instruction
#define	BRA		0xe000u
#define	BRL		0xf000u
#define	cndBr		0xc000u			// conditional branch
#define	LdStr		0x2000u			// load/store instruction
#define  PshPul	0xb000u			// push/pull stack

/* CCR condition codes */
#define	EQ			0
#define	NE			1u
#define	CS			2u
#define	CC			3u
#define	MI			4u
#define	PL			5u
#define	HI			6u
#define	LS			7u
#define	AL			8u

/* data processing op-codes */
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
#define	STP		0xd000u

/* immediate operations */
#define	mov		0x4000u
#define	cmp		0x5000u
#define	add		0x6000u
#define	sub		0x7000u

/* instruction fields */
#define	msBit32	0x80000000u							 // most significant bit
#define	lsBit		1u										 // least significant bit
#define  L        ((IR & 0x0800u) >> 0xb)        // Load/Store bit
#define  H        ((IR & 0x0400u) >> 0xa)        // High/Low bit
#define  B        ((IR & 0x0400u) >> 0xa)        // Byte/Word bit
#define  R        ((IR & 0x0100u) >> 0x8)        // Push/Pull bit
#define	sBit		((IR & 0X0100u) >> 0x10)       // logical shift right/left bit
#define	OPcode 	(IR & 0x0f00u)	  				 	 // data processing operation
#define	Rd			(regset.r[IR & 0x000f])
#define	RdNum		(IR & 0x000f)
#define	rd(num)  (regset.r[(num)])
#define	Rn			(regset.r[((IR & 0x00f0) >> 4)])
#define	RnNum		((IR & 0x00f0) >> 4)
#define	rn(num)  (regset.r[(num)])
#define  Reglist	(IR & 0X00ff)
#define	RnLSBb	(Rn & 0x000000ff)					// least significant byte of Rn
#define	imValue	((IR & 0x0ff0) >> 4u)
#define	byteMsk	(0x000000ff)
#define	_8bits	8u
#define	brCOND	((IR & 0x0f00u) >> 8u)			// branch condition code mask
#define	offset8b	(IR & 0x00ffu)						// masks for branch offset 8-bit & 12 bit
#define	offset12b (IR & 0x0fffu)

/* register set */
#define  REGNUM	16				// total number of registers
struct REGSET {
	 unsigned short N;			// sign flag
	 unsigned short C;			// carry flag
	 unsigned short Z;			// zero flag
	 unsigned short ir;			// 16-bit instruction register
	 unsigned long r[REGNUM];	// 16 32-bit registers, r1 - r12
	 unsigned long mar;			// 32-bit memory address register
	 unsigned long mbr;			// 32-bit memory buffer register
} regset;

#define	r0			0
#define	R0			(regset.r[0])
#define	r1			1
#define	R1			(regset.r[1])
#define	r2			2
#define	R2			(regset.r[2])
#define	r3			3
#define	R3			(regset.r[3])
#define	r4			4
#define	R4			(regset.r[4])
#define	r5			5
#define	R5			(regset.r[5])
#define	r6			6
#define	R6			(regset.r[6])
#define	r7			7
#define	R7			(regset.r[])
#define	r8			8
#define	R8			(regset.r[8])
#define	r9			9
#define	R9			(regset.r[9])
#define	r10		10
#define	R10		(regset.r[10])
#define	r11		11
#define	R11		(regset.r[11])
#define	r12		12
#define	R12		(regset.r[12])
#define	sp			13
#define 	SP			(regset.r[13])		// stack pointer
#define	lr			14
#define 	LR			(regset.r[14])		// link register
#define	pc			15
#define 	PC			(regset.r[15])		// program counter
#define	MAR		(regset.mar)
#define	MBR		(regset.mbr)
#define	IR			(regset.ir)
#define	N			(regset.N)
#define	Z			(regset.Z)
#define	C			(regset.C)

unsigned long ALU;
unsigned char mem[MEMSIZE];	// main memory

/* function prototypes */
void clrbuf (void);
void condiBr (void);
void dump (void * memptr);
void execute();
void exeopt (unsigned char opt, void * buffer);
void fetch (void);
void getparams (unsigned char * name, unsigned int * value);
void go (void);
void help (void);
int iscarry (unsigned long op1, unsigned long op2, unsigned c);
void itype (void);
int load (void * buffer, unsigned int max);
void loadMem (unsigned long memAddr,  unsigned short regNum, unsigned short byteflag);
void MemDump (void * memptr, unsigned offset, unsigned length);
void reg (void);
void reset (void);
unsigned short pp_regNum(unsigned short regListMsk);
void setN_Z_flg (void);
void setPC (unsigned long branchAddress);
void shiftRd (void);
void trace (void);
void writeMem (unsigned long dstAddr, unsigned short regNum, unsigned short byteflag);
void push (void);
void pull (void);

int main (int argc, char *argv[])
{

	 unsigned char in, option;

	 printf("%s version %s\n", argv[0], VER);
	 printf("%s\n\n", AUTHOR);

	 reset();

	 do	// run app loop
	 {
		  printf(">> ");

		  scanf(" %c", &in);
		  if (in != '\n')
		  {
				clrbuf();
				option = toupper(in);
				exeopt(option, mem);
		  }
	 }while(option != 'Q');

	 return 0;
}

/*
 * function definitions
 */

/*
 * condiBr () - check CCR codes & execute a conditional branch 
 */
void condiBr (void)
{
	 switch (brCOND)
	 {
		  case EQ: if (Z == 1)
					  {
							PC += (signed char) offset8b;
					  }
					  break;
		  case NE: if (Z == 0)
					  {
							PC += (signed char) offset8b;
					  }
					  break;
		  case CS: if (C == 1)
					  {
							PC += (signed char) offset8b;
					  }
					  break;
		  case CC: if (C == 0)
					  {
							PC += (signed char) offset8b;
					  }
					  break;
		  case MI: if (N == 1)
					  {
							PC += (signed char) offset8b;
					  }
					  break;
		  case PL: if (N == 0)
					  {
							PC += (signed char) offset8b;
					  }
					  break;
		  case HI: if ((C == 1) && (Z == 0))
					  {
							PC += (signed char) offset8b;
					  }
					  break;
		  case LS: if ((C == 0) && (Z == 1))
					  {
							PC += (signed char) offset8b;
					  }
					  break;
		  case AL: PC += (signed char) offset8b;
	 }
}

/* 
 * clrbuf () - clears the input buffer 
 */
void clrbuf ()
{
	 while(getchar() != '\n')
		  ;
}

/* dump () - initiates the dump memory process by starting a input prompt
 *				 for the offset & the length parmeters
 *			  - a pointer to the program memory is a required arg
 */
void dump (void *memptr)
{
	 unsigned int offset, length;

	 printf("[MEMORY DUMP]\n");
	 getparams("Offset", &offset); // get offset & length
	 getparams("Length", &length);
	 printf("Offset -> 0x%04X\n", offset);
	 if ((offset + length) > MEMSIZE)
		  length = (offset == 0) ? MEMSIZE : (MEMSIZE - offset);
	 printf("Length -> 0x%04X\n", length);

	 MemDump (memptr, offset, length);
}

/* 
 * execute () - decode and execute a fetched instruction 
 */
void execute (void)
{
	 switch (Itype)
	 {
		  // handle branch instructions
		  case BRA: PC = (unsigned) offset12b;
						break; 
		  case cndBr: condiBr();
						  break;
		  case BRL: LR = PC;
						PC = (unsigned) offset12b;
						break;
						// handle data processing instructions
		  case DPinst: switch (OPcode)
							{
								 case AND: ALU = Rd & Rn;
											  Rd = ALU;
											  break;
								 case EOR: ALU = Rd ^ Rn;
											  Rd = ALU;
											  break;
								 case SUB: ALU = Rd + ~Rn + 1;
											  C = iscarry(Rd, ~Rn, 1);
											  Rd = ALU;
											  break;
								 case SXB: ALU = (signed) RnLSBb;
											  Rd = ALU;
											  break;
								 case ADD: ALU = Rd + Rn;
											  C = iscarry(Rd, Rn, 0);
											  Rd = ALU;
											  break;
								 case ADC: ALU = Rd + Rn + iscarry(Rd, Rn, C);
											  C = iscarry(Rd, Rn, C);
											  Rd = ALU;
											  break;
								 case LSR: shiftRd();
											  break;
								 case LSL: shiftRd();
											  break;
								 case TST: ALU = Rd & Rn;
											  break;
								 case TEQ: ALU = Rd | Rn;
											  break;
								 case CMP: ALU = Rd + ~Rn + 1;
											  Rd = ALU;
											  break;
								 case ROR: ALU = Rd >> Rn;
											  Rd = ALU;
											  break;
								 case ORR: ALU = Rd | Rn;
											  Rd = ALU;
											  break;
								 case MOV: ALU = Rn;
											  Rd = ALU;
											  break;
								 case BIC: ALU = Rd & (~Rn);
											  break;
								 case MVN: ALU = ~Rn;
											  Rd = ALU;
											  break;
							}
							setN_Z_flg();
							break;

							// handle immediate instructions
		  case mov: ALU = imValue;
						Rd = ALU;
						setN_Z_flg();
						break;
		  case add: ALU = Rd + imValue;
						C = iscarry(Rd, Rn, 0);
						Rd = ALU;
						setN_Z_flg();
						break;
		  case sub: ALU = Rd + ~imValue + 1;
						C = iscarry(Rd, ~Rn, 1);
						Rd = ALU;
						setN_Z_flg();
						break;
		  case cmp: ALU = Rd + ~imValue + 1;
						C = iscarry(Rd, ~Rn, 1);
						setN_Z_flg();	
						break;

						// handle load/store insructions
		  case LdStr: switch (L)
						  {
								case 0: writeMem(Rn, RdNum, B);
										  break; 
								case 1: loadMem(Rn, RdNum, B);
										  break; 
						  }
						  break;
		 case PshPul: switch (L)
						  {
								case 0: push();
										  break;
								case 1: pull();
										  break;
						  }
	 }

}

/* exectopt() - executes user options by calling a function that
 * 				 corresponds to an option
 *				  - args are an option character & a pointer to a buffer
 */
void exeopt (unsigned char opt, void * buffer)
{
	 switch(opt)
	 {
		  case 'D': dump(buffer);
						break;
		  case 'G': go();
						break;
		  case 'L': load(buffer, MEMSIZE);
						break;
		  case 'Q': break;
		  case 'R': reg();
						break;
		  case 'T': trace();
						break;
		  case 'Z': reset();
						break;
		  case 'H':
		  case '?':  help();
						 break;
		  default: printf("Unrecognized command\n");
					  printf("Enter h or ? for command list\n");
	 }
}

/* fetch () - gets an instruction from main memory and executes it
 *			   - the address of the next 16-bit instruction to be fetched is set in 
 *			     the MBR using the PC
 *			   - the fetched instruction is put into the IR (insruction
 *			     register)
 */
void fetch (void)
{
	 MAR = PC++;
	 MBR = (unsigned long) MEM;
	 MBR <<= QWORD;
	 MAR = PC++;
	 MBR |= (unsigned long) MEM;
	 IR = ALU = (unsigned short) MBR;
}

/* getparams () - is a helper function for the dump() routine. It runs a
 *					   a prompt and set the value for parameter.
 *					 - args are the parameter name and a pointer to save the
 *					   value of the parameter
 */
void getparams (unsigned char * name, unsigned int * value)
{
	 unsigned char input[MAXLEN];

	 do {
		  printf("Enter %s : ", name);
		  fgets(input, MAXLEN, stdin);
		  sscanf(input, " %X", value);
		  if(*value > MEMSIZE)
				printf("Error: invalid hex input\n");
		  else break;
	 } while (1);
}

/* go () - clears registers & runs the entire program */
void go (void)
{
	 reset();		// execution start at memory location 0
	 do {
		  fetch();
		  if (Itype == STP)
		  {
				printf("\nbreak point reached... \n\n") ; 
				break;
		  }
		  execute();
	 } while(1);
	 reg();
}

/* help() - displays  a list of commands */
void help (void)
{
	 printf("\t------ command list ------\n");
	 printf("d\tdump memory\n");
	 printf("g\tgo - run the entire program\n");
	 printf("l\tload a file into memory\n");
	 printf("q\tquit\n");
	 printf("r\tdisplay registers\n");
	 printf("t\ttrace - execute one instruction\n");
	 printf("z\treset all registers to zero\n");
	 printf("?, h\tdisplay list of commands\n");
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

/* load() - loads a file into memory and displays a byte count for the
 *			   loaded file
 *        - args are a pointer to memory and the memory size
 *        - on success the byte count is returned and on fail -1 returns
 */
int load (void * buffer, unsigned int max)
{
	 FILE *fptr;
	 int bytecnt = 0;
	 unsigned char fname[MAXLEN];

	 /* get file name */
	 printf("[LOAD FILE]\nenter file name: ");
	 fgets(fname, MAXLEN, stdin);
	 fname[strlen(fname) - 1] = '\0';   // remove line feed character

	 printf("%s -> opening file ...\n", fname);
	 memset(buffer, 0, max);

	 /* Open file in read mode */
	 if ((fptr = fopen(fname, READ)) == NULL)
	 {
		  perror("OOPS!");
		  return -1;
	 } else {// Load file into memory
		  bytecnt = fread(buffer, 1, MEMSIZE, fptr);
		  printf("bytes: %xH , %d", (unsigned int)bytecnt, (unsigned int)bytecnt);
		  printf("  truncated: ");
		  (bytecnt == max) ? printf("y\n") : printf("n\n");

		  fclose(fptr);
	 }

	 return bytecnt;
}

/*
 * loadMem() - loads data from memory to a register
 *				 - args are a memory address, register number 0 - 15
 *					and a byte flag 0 or 1OA
 */
void loadMem (unsigned long memAddr,  unsigned short regNum, unsigned short byteflag)
{
	unsigned short bytecnt;
	MAR = memAddr;
	MBR = (unsigned long) MEM;

	if (byteflag == 1)
	{
		rd(regNum) = MBR;
		return;		// loads 1 byte to register
	}

	
	for (bytecnt = 1; bytecnt < word; bytecnt++)
	{
		MBR <<= _8bits;
		++MAR;
		MBR |= (unsigned long) MEM;
	}
	rd(regNum) = MBR;
}

/* MemDump() - displays memory content
 *				 - args are a pointer to memory, mempry offset, and the
 *				   length from offset
 */
void MemDump (void * memptr, unsigned offset, unsigned length)
{
	 unsigned int i, index = offset;
	 unsigned char * memval = memptr;

	 do {
		  unsigned int index_2 = index;
		  printf("%04x\t", index);
		  // print hex values
		  for (i = 0; i < HWORD; i++, index++)
		  {
				if (index < (offset + length))
					 printf("%02hhX ", memval[index]);

		  }

		  printf("\n    \t");
		  index = index_2;	// reset	index
		  // print characters
		  for (i = 0; i < HWORD; i++, index++)
		  {
				if (index < (offset + length))
				{
					 if (!isspace(memval[index]) && isprint(memval[index]))
						  printf(" %c ", memval[index]);
					 else printf(" . ");
				}
				else break;
		  }
		  printf("\n");
	 } while(index < (offset + length));
}

unsigned short pp_regNum(unsigned short regListMsk)
{
	unsigned short regNum = 0;

	switch (regListMsk)
	{
		case bit0: regNum = (H)? r8 : r0; 
				 	  break;
		case bit1: regNum = (H)? r9 : r1;
				 	  break;
		case bit2: regNum = (H)? r10 : r2;
				 	  break;
		case bit3: regNum = (H)? r11 : r3;
				 	  break;
		case bit4: regNum = (H)? r12 : r4;
				 	  break;
		case bit5: regNum = (H)? sp : r5;
				 	  break;
		case bit6: regNum = (H)? lr : r6;
				 	  break;
		case bit7: regNum = (H)? pc : r7;
				 	  break;
	}
	return regNum;
}

void pull (void)
{printf("\nPULLING\n");	
	unsigned short regCnt, regListMsk = 0x01u;

	for (regCnt = 0; regCnt < QWORD; regCnt++, regListMsk <<= 1) // check 8 register bits
	{
		if (Reglist & regListMsk)
		{printf("\treg%d\n",pp_regNum(regListMsk));
			loadMem(SP, pp_regNum(regListMsk), 0);
			SP += word;	// increament SP by 4 bytes
		}
	}

	if (R)		// extra pull
	{printf("\tr%d\n",pc);
		loadMem(SP, pc, 0);
		SP += word;
	}printf("\n");
}

void push (void)
{printf("\nPUSHING\n\n");
	unsigned short regCnt, regListMsk = 0x80u;

	if (R)		// extra push
	{
		SP -= word;
		writeMem(SP, lr, 0);
	}

	for (regCnt = 0; regCnt < QWORD; regCnt++, regListMsk >>= 1) // check 8 register bits
	{
		if (Reglist & regListMsk)
		{
			SP -= word;	// decreament SP by 4 bytes
			writeMem(SP, pp_regNum(regListMsk), 0);
		}
	}
}

/* 
 * reg () - displays register set 
 */
void reg (void)
{
	 int i;
	 for(i = 0; i < REGNUM; i++) // print registers r0 - r15
	 {
		  if (i != 0)
				(i % nibble == 0) ? printf("\n"): 0 ;
		  switch (i)
		  {
				case sp: printf("sp=%08lx ", SP);
							break;
				case lr: printf("lr=%08lx ", LR);
							break;
				case pc: printf("pc=%08lx", PC);
							break;
				default: printf("r%x=%08lx ", i, regset.r[i]);
		  }
	 }
	 printf("\nmar=%08lx ",MAR);
	 printf("mbr=%08lx ", MBR);
	 printf("ir=%04hx   ", IR); // show IR & set flags
	 (N == 1) ? printf("N "): printf("  ");
	 (Z == 1) ? printf("Z "): printf("  ");
	 (C == 1) ? printf("C\n"): printf("\n");
}

/*
 * reset () - clears all registers to zero and resets flags 
 */
void reset (void)
{
	 memset(&regset, 0, sizeof regset);
	 N = Z = C = 0;
	 reg();
}

/*
 * setN_Z_flg () - checks the ALU status and set the N and the Z flag
 *						 appropiately 
 */
void setN_Z_flg (void)
{
	 N = (hiMSB32 == 1) ? 1 : 0;
	 Z = (ALU == 0) ? 1 : 0;
}

/*
 * setPC () - sets pc to branch address
 */
void setPC (unsigned long branchAddress)
{
	 ALU = branchAddress;
	 PC = ALU; 
}

/*
 * shiftRd () - logical shift Rd. The shift left/right bit (sBit) dertermines if shift
 * 			 	 is left or right
 */
void shiftRd (void)
{
	 ALU = Rd;
	 if (sBit == 0) // shift right
	 {
		  C = ((ALU >> (Rn - 1) ) & lsBit);
		  ALU >>= Rn;
	 }
	 else				// shift left
	 {
		  C = ((ALU << (Rn - 1) ) & msBit32);
		  ALU <<= Rn;
	 }
	 Rd = ALU;
	 setN_Z_flg();
}

/* trace() - executes one instruction & displays registers content
 *			 	 by calling the fetch() routine & then reg() routine
 */
void trace (void)
{
	 fetch();
	 execute();
	 reg();
}

/*
 * writeMem() - writes data from a register to memory
 *				  - args are a memory address, register number 0 - 15
 *					 and a byte flag
 */
void writeMem (unsigned long memAddr, unsigned short regNum, unsigned short byteflag)
{
	signed short bytecnt;
	MAR = memAddr;
	MBR = rn(regNum);
	 if (byteflag) 	// store lsbyte of Rd 
	 {
		  MEM = (unsigned char) MBR;
		  return;
	 }

	 for (bytecnt = word - 1 ; bytecnt >= 0; bytecnt--) // store word
	 {
		  MEM = (unsigned char) (MBR >> (_8bits * bytecnt));
		  ++MAR;
	 }
}