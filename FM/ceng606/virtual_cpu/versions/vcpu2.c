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

#define AUTHOR "Madzivire Chikwanda"
#define MEMSIZE 0x4000
#define MAXLEN 0x100		// max file name size
#define READ "rb"			// read file in binary mode
#define VER "2.0"
#define WORD 0x10


/* function prototypes */
void clrbuf (void);
void dump (void * memptr);
void exeopt (unsigned char opt, void * buffer);
void getparams(unsigned char * name, unsigned int * value);
void go (void);
void help (void);
int load (void * buffer, unsigned int max);
void MemDump (void * memptr, unsigned offset, unsigned length);
void reg (void);
void trace (void);
void reset (void);

/* global variables */
unsigned char mem[MEMSIZE];

int main (int argc, char *argv[])
{

	 unsigned char in, option;

	 printf("%s version %s\n", argv[0], VER);
	 printf("%s\n\n", AUTHOR);

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
void reg (void)
{
	 printf("display registers\n");
}

/* trace - execute one instruction */
void trace (void)
{
	 printf("trace - execute one instruction\n");
}

/* reset all registers to zero*/
void reset (void)
{
	 printf("reset all registers to zero\n");
}
