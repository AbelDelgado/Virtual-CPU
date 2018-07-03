/*
 * Madzivire Chikwanda
 * 05/03/2014, version 3.0
 *
 * vcpu.c is an implementation of a virtual CPU user interface that has
 *	the following features:
 * - displays a list of commands for help
 * - loads a file into memory
 * - memory dump
 * - instruction tracing
 *
 * The following assumptions were made for implementing memory & the register
 * set:
 * - memory is implemented using an unsigned char array & referenced memory is
 *	  one byte in size 
 * - for 32-bit registers, an unsigned long is used
 * - 16-bit registers are implemented using the unsigned short type
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define	AUTHOR	"Madzivire Chikwanda"
#define	MEMSIZE	0x4000
#define	MEMMASK	(MEMSIZE - 1)			  // memory mask
#define  MEM		(mem[ MAR & MEMMASK ]) // masked memory address
#define	MAXLEN	0x100			// max file name size
#define	READ		"rb"			// read file in binary mode
#define	VER		"3.0"
#define	WORD		0x10
#define	HWORD		0x8
#define	nibble	0x4

#define	ITYPE		0xf000		// instruction type mask

/* CCR condition codes */
#define	EQ			0b0000
#define	NE			0b0001
#define	CS			0b0010
#define	CC			0b0011
#define	MI			0b0100
#define	PL			0b0101
#define	HI			0b1000
#define	LS			0b1001
#define	AL			0b1110

// CCR flags
#define	Z			1			// zero flag
#define	C			2			// carry flag
#define	N			3			// negative flag

#define	ckZflg	((Rd == 0) ? CCR |= Z : 0)
#define	ckNflg	( ((Rd & hiBit) != 0) ? CCR |= N : 0)	 

/* IR data processing op-codes */
#define	AND		0x0000
#define	EOR		0x0100
#define	SUB		0x0200
#define	SXB		0x0300
#define	ADD		0x0400
#define	ADC		0x0500
#define	LSR		0x0600
#define	LSL		0x0700
#define	TST		0x0800
#define	TEQ		0x0900
#define	CMP		0x0a00
#define	ROR		0x0b00
#define	ORR		0x0c00
#define	MOV		0x0d00
#define	BIC		0x0e00
#define	MVN		0x0f00

/* IR immediate instructions op-codes */
#define	mov		0x4000
#define	cmp		0x5000
#define	add		0x6000
#define	sub		0x7000
#define	RdMask	0x000f	// mask for destination register
#define	imvMask	0x0ff0	// immediate value mask
#define	Rd			(regset.R[ IR & RdMask])		// destnation register
#define	imValue	( (IR & imvMask) >> nibble)	// immediate value
#define	STOP		0xd000
#define	EXIT		((IR & ITYPE) == STOP ? 1 : 0)


/* register set */
#define 	R0			(regset.R[0])		// registers r1 - r12
#define 	R1			(regset.R[1])
#define 	R2			(regset.R[2])
#define 	R3			(regset.R[3])
#define 	R4			(regset.R[4])
#define 	R5			(regset.R[5])
#define 	R6			(regset.R[6])
#define 	R7			(regset.R[7])
#define 	R8			(regset.R[8])
#define 	R9			(regset.R[9])
#define 	R10		(regset.R[10])
#define 	R11		(regset.R[11])
#define 	R12		(regset.R[12])
#define 	SP			(regset.R[13])		// stack pointer
#define	sp			13
#define 	LR			(regset.R[14])		// link register
#define	lr			14
#define 	PC			(regset.R[15])		// program counter
#define	pc			15
#define  REGNUM	16						// total number of registers
#define	MAR		(regset.mar)
#define	MBR		(regset.mbr)
#define	IR			(regset.ir)
#define	CCR		(regset.ccr)
#define	hiBit		0x80000000			// high bit mask for a 32-bit register

struct REGSET {
	 unsigned long R[WORD];		// 16 32-bit registers, r1 - r12
	 unsigned long mar;			// 32-bit memory address register
	 unsigned long mbr;			// 32-bit memory buffer register
	 unsigned short ir;			// 16-bit instruction register
	 unsigned char ccr;			// 8-bit condition code register
} regset, RESETreg = {0};

unsigned char mem[MEMSIZE];	// main memory

/* function prototypes */
void clrbuf (void);
void dump (void * memptr);
void execute();
void exeopt (unsigned char opt, void * buffer);
void fetch (void);
void getparams (unsigned char * name, unsigned int * value);
void go (void);
void help (void);
int load (void * buffer, unsigned int max);
void MemDump (void * memptr, unsigned offset, unsigned length);
void reg (struct REGSET * regset);
void trace (void);
void reset (struct REGSET * regset);

int main (int argc, char *argv[])
{

	 unsigned char in, option;

	 printf("%s version %s\n", argv[0], VER);
	 printf("%s\n", AUTHOR);

	 reset(&regset);

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
 *function definitions
 */

/* clrbuf() - clears the input buffer */
void clrbuf ()
{
	 while(getchar() != '\n')
		  ;
}

/* dump() - initiates the dump memory process by starting a input prompt
 *				for the offset & the length parmeters
 *			 - a pointer to the program memory is a required arg
 */
void dump (void *memptr)
{
	 unsigned int offset, length;

	 printf("[MEMORY DUMP]\n");
	 getparams("Offset", &offset); // get offset & length
	 getparams("Length", &length);
	 printf("Offset -> 0x%04X\n", offset);
	 printf("Length -> 0x%04X\n", length);

	 MemDump (memptr, offset, length);
}

/* execute() - decode and execute the instruction held by IR
 */
void execute (void)
{
	 CCR &= 0; // clear flags
	 switch (IR & ITYPE)
	 {
		  // handle immediate instructions
		  case mov: Rd = imValue;
						ckNflg;
						ckZflg;
						printf("\nMOV\n");
						break;
		  case add: Rd += imValue;
						ckNflg;
						ckZflg;
						printf("\nADD\n");
						break;
		  case sub: Rd -= imValue;
						ckNflg;
						ckZflg;
						printf("\nSUBTRACT\n");
						break;
		  case cmp: ((Rd - imValue) == 0) ? (CCR |= Z) : 0;
						(((Rd - imValue) & hiBit) == 0)	? (CCR |= N) : 0;
						printf("\nCOMP\n");
						break;			  
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
		  case 'R': reg(&regset);
						break;
		  case 'T': trace();
						break;
		  case 'Z': reset(&regset);
						break;
		  case 'H':
		  case '?':  help();
						 break;
		  default: printf("Unrecognized command\n");
					  printf("Enter h or ? for command list\n");
	 }
}

/* fetch() - gets an instruction from main memory and executes it
 *			  - the address of the next 16-bit instruction to be fetched is set in 
 *			    the MBR using the PC
 *			  - the fetched instruction is put into the IR (insruction
 *			    register)
 */
void fetch (void)
{
	 MAR = PC++;
	 MBR = (unsigned long) MEM;
	 MBR <<= HWORD;
	 MAR = PC++;
	 MBR |= (unsigned long) MEM;
	 IR = (unsigned short) MBR;

	 // TODO call execute() here
}

/* getparams() - is a helper function for the dump() routine. It runs a
 *					  a prompt and set the value for parameter.
 *					- args are the parameter name and a pointer to save the
 *					  value of the parameter
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

/* go() - clears registers & runs the entire program */
void go (void)
{
	 reset(&regset);
	 while(!EXIT)
	 {
		  printf("Executing %hhX\n");
		  execute();
	 }
	 reg(&regset);
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
		  printf("Byte count (hex): %X\n", (unsigned int)bytecnt);
		  printf("Byte count (dec): %d\n", (unsigned int)bytecnt);
		  printf("Truncated: ");
		  if(bytecnt == max)
				printf("yes\n");
		  else printf("no\n");

		  fclose(fptr);
	 }

	 return bytecnt;
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
		  for(i = 0; i < WORD; i++, index++)
		  {
				if ((index < MEMSIZE) && (index < (offset + length)))
					 printf("%02hhX ", memval[index]);
				
		  }

		  printf("\n    \t");
		  index = index_2;	// reset	index
		  // print characters
		  for(i = 0; i < WORD; i++, index++)
		  {
				if ((index < MEMSIZE) && (index < (offset + length)))
				{
					 if(!isspace(memval[index]) && isprint(memval[index]))
						  printf(" %c ", memval[index]);
					 else printf(" . ");
				}
				else break;
		  }
		  printf("\n");
	 } while((index < MEMSIZE) && (index < (offset + length)));
}

/* reg() - displays register set 
 *			- arg is a pointer to the register set structure
 */
void reg (struct REGSET * regset)
{
	 int i;
	 for(i = 0; i < WORD; i++) // print registers r0 - r15
	 {
		  if(i % nibble == 0)
				printf("\n");
		  switch (i)
		  {
				case sp: printf("SP=%08lx    ", regset->R[sp]);
							break;
				case lr: printf("LR=%08lx    ", regset->R[lr]);
							break;
				case pc: printf("PC=%08lx    ", regset->R[pc]);
							break;
				default: printf("R[%02i]=%08lx ", i, regset->R[i]);
		  }
	 }
	 printf("\nMAR=%08lx   ", regset->mar);
	 printf("MBR=%08lx   ", regset->mbr);
	 printf("IR=%04hhx\t     ", regset->ir); // show IR & CCR registers
	 printf("CCR=%02hx\n\n", regset->ccr);
}

/* reset() - clears all registers to zero 
 *			  - arg is a pointer to the register set structure
 */
void reset (struct REGSET * regset)
{
	 //memset(regset, 0, sizeof regset);
	 *regset = RESETreg;
	 reg(regset);
}

/* trace() - executes one instruction & displays registers content
 *			 	 by calling the fetch() routine & then reg() routine
 */
void trace (void)
{
	 fetch();
	 execute();
	 reg(&regset);
}
