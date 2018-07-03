/* Ryan Lachman
   810-113-407
   Virtual CPU Interface
   CENG 606 - Computer Systems Architecture
   */

#define _CRT_SECURE_NO_DEPRECATE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char memory[16 * 1024];
int LoadFile(void * memory, unsigned int max);
void WriteFile(void * memory);

/* Main function 
	contains main menu and switch case */
int main(){

	
	int choice = 0;

	Menu();

	while (1){

		choice = Prompt();

		switch (choice){

			case 'D': //dump memory
				printf("\nnot yet implemented\n");
				break;

			case 'G': //go - run the entire program
				printf("\nnot yet implemented\n");
				break;

			case 'L': //load a file into memory
				LoadFile(memory, sizeof(memory));
				break;

			case 'M': //memory modify
				printf("\nnot yet implemented\n");
				break;

			case 'Q': //quit
				Quit();
				break;

			case 'R': //display registers
				printf("\nnot yet implemented\n");
				break;

			case 'T': //trace - execute one instruction
				printf("\nnot yet implemented\n");
				break;

			case 'W': //write file
				WriteFile(memory);
				break;

			case 'Z': //reset all registers to zero
				printf("\nnot yet implemented\n");
				break;

			case '?': case 'H': //display list of commands
				Help();
				break;

			default: printf("\nInvalid entry!\nPlease enter a valid option!\n");
		}
	}
	return 0;
}

/* Menu function 
	displays name and options for CPU */
int Menu(){

		system("cls");
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

/* Prompt function 
	to take user input */
int Prompt(){

	printf("\n-> ");
	
	int choice = toupper(getche()); //converts lowercase to uppercase while ignoring if input is already uppercase
	while (getchar() != '\n');		// while loop to flush input buffer when enter is hit after option input
	//getchar();
		
	return choice;
}

/* dump memory function
	d - displays memory dump */
int DumpMemory(){

}

/* go function
	g - run the entire program */
int Go(){

}







/* load file function
	l - load a file into memory */
int LoadFile(void * memory, unsigned int max){

	char fileName[300];		//buffer to hold file path & name
	FILE * fd;				//file descriptor to read the file
	int n;					//variable to hold bytes of file
	int size;

		printf("\nEnter full path of file to load: ");
		gets_s(fileName, sizeof(fileName));			

		if (strlen(fileName) == 0){		//input string checking if anything valid was entered or if nothing was entered

			perror("\nNo input given, enter a valid file name and path.\n");
			return (-1);
		}

		fopen_s(&fd, fileName, "r");	//open file to read
		fseek(fd, 0L, SEEK_END);		//go to end of file to get size of file before putting in memory
		size = ftell(fd);				//put raw size into variable
		fseek(fd, 0L, SEEK_SET);		//return to beginning of file

		if (fd == NULL){				//check if the buffer is empty

		    //if buffer is empty then the file was not successfully opened
			perror("\nFile was not opened.\n");
			return (-1);
		}
		
		n = fread(memory, sizeof(char), max, fd);	//if file is opened then read into memory

		if (size > max){	//check for truncate if the size of the byte variable is greater than the max memory
			printf("\nThe file's size is truncated as it exceeds the maximum memory.\n");
		}

		fprintf(stdout, "\n%d Bytes read in decimal and %xH in Hex.\n", n, n);	//display the bytes of the file in decimal and hex

		
		fclose(fd);
}














/* memory modify function
	m - memory modify */
int MemoryModify(){

}

/* quit function
	q - quits the program */
int Quit(){

	exit(0);
}

/* display registers function
	r - displays registers */
int DisplayRegisters(){

}

/* trace function 
	t - execute one instruction */
int Trace(){

}

/* write file function 
	w - write a file */
void WriteFile(void * memory){

	char fileNameW[300];		//buffer to hold filename 
	int writeByte;				//variable to hold number of bytes
	FILE * fp;					//file descriptor for writing to file
	int w;						//variable to hold bytes of file
	
	printf("\nEnter the name of the file to write: ");
	gets_s(fileNameW, sizeof(fileNameW));		

	if (strlen(fileNameW) == 0){	//input string checking if anything valid was entered or if nothing was entered

		perror("\nNo input given, enter a valid file name.\n");
		return(-1);
	}

	fopen_s(&fp, fileNameW, "w");	//open file to write

	/*if (fp == NULL){				//check if buffer is empty

		printf("\nFile was not written.\n");	//if buffer is empty then the file was not successfully opened
		perror(EXIT_FAILURE);
	}*/

		printf("Enter the size of bytes to write to file: ");	
		scanf("%i", &writeByte);				//take user input of size of bytes to write to the file

		if (writeByte < 0){					//check if user enters anything less than 0 for bytes to write
			
			//printf("\nNo bytes added to file.\n");
			perror("\nNo bytes added to file.\n");
			return (-1);
		}

		w = fwrite(memory, sizeof(char), writeByte, fp);	//write the file into memory
		printf("\nFile written with %d bytes.\n", w);

	fclose(fp);
}

/* reset registers function
	z - reset all registers to zero */
void ResetRegisters(){

}

/* help function
	?, h - display list of commands */
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

//----------------------------------------------------------------------------------------------------------------------------------------------------
