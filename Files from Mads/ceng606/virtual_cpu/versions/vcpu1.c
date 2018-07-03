/*
 * Madzivire Chikwanda
 * 1/20/2014
 * 
 * vcpu.c is an implementation of a virtual CPU user interface.
 * Displays an options menu.
 *
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AUTHOR "Madzivire Chikwanda"
#define MEMSIZE 16384
#define LINEFD 0xA		// line feed character
#define MAXLEN 256		// max file name
#define READ "rb"			// read file in binary mode
#define VER "1.0"


/* function prototypes */
void clrbuf (void);
void dump (void);
void exeopt (unsigned char opt, void * buffer);
void go (void);
void help (void);
int load (void * buffer, unsigned int max);
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

	 do
	 {
		  printf(">> ");

		  scanf("%c", &in);
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

/* clrbuf() clears input buffer */
void clrbuf ()
{
	 while(getchar() != '\n')
		  ;
}

/* dump memory */
void dump (void)
{
	 printf("dump memory\n");
}

/* exectopt() executes user options */
void exeopt (unsigned char opt, void * buffer)
{
	 switch(opt)
	 {
		  case 'D': dump();
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

/* go - run the entire program */
void go (void)
{
	 printf("go - run the entire program\n");
}

/* display list of commands */
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

/* load a file into memory */
int load (void * buffer, unsigned int max)
{
	 FILE *fptr;
	 int bytecnt = 0;
	 unsigned char fname[MAXLEN];

	 /* get file name */
	 printf("enter file name: ");
	 fgets(fname, MAXLEN, stdin);
	 fname[strlen(fname) - 1] = '\0';   // remove line feed character

	 printf("%s -> opening file ...\n", fname);


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
