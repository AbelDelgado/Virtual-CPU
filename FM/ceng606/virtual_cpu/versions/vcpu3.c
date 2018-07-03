/*
 * Madzivire Chikwanda
 * 2/11/2014, version 2.0
 *
 * vcpu.c is an implementation of a virtual CPU user interface that has
 *	the following features:
 * - displays a list of commands for help
 * - loads a file into memory
 * - dumps memory
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define	AUTHOR	"Madzivire Chikwanda"
#define	MEMSIZE	0x4000
#define	MEMMASK	(MEMSIZE - 1)
#define	MAXLEN	0x100			// max file name size
#define	READ		"rb"			// read file in binary mode
#define	VER		"2.0"
#define	WORD		0x10			// 16-bits
#define	HWORD		0x8			// 8-bits
#define	nibble	0x4			// 4-bits

/* register set ids */
#define 	r0			0				// registers r1 - r12
#define 	r1			1
#define 	r2			2
#define 	r3			3
#define 	r4			4
#define 	r5			5
#define 	r6			6
#define 	r7			7
#define 	r8			8
#define 	r9			9
#define 	r10		10
#define 	r11		11 
#define 	r12		12
#define 	sp			13				// stack pointer
#define 	lr			14				// link register
#define 	pc			15				// program counter
#define  REGNUM	16				// total number of registers

/* CCR register condition codes masks */
#define ZERO		0b00000001;
#define SIGN		0b00000010;
#define CARRY		0b00000100;

/* register set */
struct REGSET {
	 unsigned long R[WORD];		// 16 32-bit registers, r1 - r12
	 unsigned long mar;			// 32-bit memory address register
	 unsigned long mbr;			// 32-bit memory buffer register
	 unsigned short ir;			// 16-bit instruction register
	 unsigned char ccr;			// 8-bit condition code register
} regset;

unsigned char mem[MEMSIZE];	// main memory

/* function prototypes */
void clrbuf (void);
void dump (void * memptr);
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

/* fetch() - a 16-bit instruction is fetched from main memory
 * 		  - the PC (program counter) has the address of the next 
 *				 instruction to be fetched
 *			  - the fetched instruction is put into the IR (insruction
 *			  - register)
 */
void fetch (void)
{
	regset.mar = regset.R[pc];
	regset.mbr = (unsigned long) mem[ regset.R[pc]++ ];
	regset.mbr <<= HWORD;
	regset.mbr |= (unsigned long) mem[ regset.R[pc]++ ];

	regset.ir = (unsigned short) regset.mbr;
	regset.ir |= (unsigned short) regset.mbr;   
}

/* getparams() - is a helper function for the dump() routine. It runs a
 *					  a prompt and set the value for parameter.
 *					- args are the parameter name and a pointer to save the
 *					  value of the parameter
 */
void getparams(unsigned char * name, unsigned int * value)
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

/* go() - run the entire program */
void go (void)
{
	 printf("go - run the entire program\n");
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
	 if((fptr = fopen(fname, READ)) == NULL)
	 {
		  perror("OOPS!");
		  return -1;
	 }else{// Load file into memory
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
 *				 - length from offset
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
					 printf("%02X ", memval[index]);
				else break;
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

/* display registers */
void reg (struct REGSET * regset)
{
	 int i;
	 for(i = 0; i < WORD; i++) // print registers r0 - r15
	 {
		  if(i % nibble == 0)
				printf("\n");
		  switch (i)
		  {
				case sp: printf("SP=%08x    ", regset->R[sp]);
							break;
				case lr: printf("LR=%08x    ", regset->R[lr]);
							break;
				case pc: printf("PC=%08x    ", regset->R[pc]);
							break;
				default: printf("R[%02i]=%08x ", i, regset->R[i]);
		  }
	 }
	 printf("\nMAR=%08x   ", regset->mar);
	 printf("MBR=%08x   ", regset->mbr);
	 printf("IR=%04x\t     ", regset->ir); // show IR & CCR registers
	 printf("CCR=%02x\n\n", regset->ccr);
}

/* trace - execute one instruction */
void trace (void)
{
	 fetch();
	 reg(&regset);
}

/* reset all registers to zero*/
void reset (struct REGSET * regset)
{
	 memset(regset, 0, sizeof regset);
	 reg(regset);
}
