#define MEMSIZE 16384 //THIS IS 16KB OR W/E I WANT IT TO BE
#define MAXLEN 256 //THIS IS SO NO ONE WRITES A HUGE FILENAME
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


void menuFunction();
char *trimwhitespace(char *str);



unsigned char memory [MEMSIZE]; //NO BYTE TYPE IN C, THIS IS CLOSEST TO IT

int main ()
{

    int input;	
    char option;
    char buf[BUFSIZ];	
    char bufferHolder [BUFSIZ];
	

    char buf2[BUFSIZ];	

    int addressInMemory;
    int lengthOfMessage;
	

	int getOffset(int * finalAddress)
	{
		unsigned char inputAddress[MAXLEN];
		printf("Enter offset address:");
		fgets(inputAddress, MAXLEN, stdin);
		sscanf(inputAddress, "%X", finalAddress);
	}

	int getLength(int * finalLengthOfMessage)
	{
		unsigned char inputLength[MAXLEN];
		printf("Enter length of Message:");
		fgets(inputLength, MAXLEN, stdin);
		sscanf(inputLength, "%X", finalLengthOfMessage);
	}

	void memoryDump(void *pointerToMemory, unsigned addressInMemory, unsigned lengthOfMessage)
	{		
		unsigned int startPoint, index = addressInMemory; //index keeps going up, but startPoint is set to 0 at every for loop
		unsigned char * addressToMemory = pointerToMemory;//location of memory
		unsigned int totalLengthOfMessage = addressInMemory + lengthOfMessage; //from starting address(offset) to the end of length (length of message)

		printf("Address: 0x%04X\n",addressInMemory);
		printf("Length:  0x%04X\n",lengthOfMessage);

		while (index < (totalLengthOfMessage))		
		{
			unsigned int indexStartingPoint = index; //Used so that it can start back at starting point for character prints
			printf("%04x\t", index);

			for (startPoint = 0; startPoint < 16; startPoint++, index++) //Print out 16 bytes only
			{
				if ( index < (totalLengthOfMessage) ) //The end of where it should be, so it stops part way if address is something like 12
				{
					printf(" %02X",addressToMemory[index]);
				}
				else break;
			}
	
			index = indexStartingPoint;
			printf("\n    \t");


			for (startPoint = 0; startPoint < 16; startPoint++, index++)
			{	
				if ( index < (totalLengthOfMessage) ) 
				{
					if(!isspace(addressToMemory[index]) && isprint(addressToMemory[index]))
					{
						printf(" %c ",addressToMemory[index]);
					}
					else printf(" . ");
				}
				else break;
			}

			printf("\n");
		}
	}


	




    //WRITING TO A FILE FUNCTION 
    int WriteFile(void *memory)
    {	
	unsigned char fname[MAXLEN];
	unsigned char numberOfBytes[MAXLEN];
	int writeByte;	
	int writeToFile;			

	/* get file name */
	printf("enter file name: ");
	fgets(fname, MAXLEN, stdin);
	fname[strlen(fname) - 1] = '\0';   // remove line feed character
	printf("***%s*** -> opening file ...\n", fname);

	FILE *f = fopen(fname, "w");
	if (f == NULL)
	{
	    printf("Error opening file: Invalid Name!\n");
	    fclose(f);
	    return 1;
	}
//	printf("Succuess! Opening %s",fname);
	printf("Enter number of bytes: ");
	fgets (numberOfBytes, MAXLEN,stdin);
	int finalByte = atoi(numberOfBytes);

	if (finalByte < 0)
	{
	    printf("\nNot a valid size!!\n");
	    fclose(f);
	    return 1;
	}
	if (finalByte > MEMSIZE)
	{
		printf("Size too big. Closing file:\n");
		fclose(f);
		return 1;
	}
	
	writeToFile = fwrite (memory,sizeof(char), finalByte, f);
	printf("Writing a file with %d bytes\n", writeToFile);
	fclose(f);	
	
    }




    //LOAD A FILE INTO MEMORY!!!
    int LoadFile (void * memory, unsigned int max)
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
	if((fptr = fopen(fname, "rb")) == NULL) //rb is for Reading in BINARY mode!!!
	{
	    perror("OOPS!");
	    return -1;
	}else{// Load file into memory
	    bytecnt = fread(memory, 1, MEMSIZE, fptr);
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

     


    //REMOVES ALL WHITESPACES
    char *trimwhitespace(char *str)
    {
	char *end;

	// Trim leading space
	while(isspace(*str)) str++;

	if(*str == 0)  // All spaces?
	    return str;

	// Trim trailing space
	end = str + strlen(str) - 1;
	while(end > str && isspace(*end)) end--;

	// Write new null terminator
	*(end+1) = 0;

	return str;
    }



	int memoryModify(void *pointerToMemory, unsigned addressInMemory)
	{
		unsigned char inputForNewValue[MAXLEN];	
		unsigned int startPoint, index = addressInMemory;
		unsigned char * addressToMemory = pointerToMemory;//location of memory
		printf("Address: 0x%04X\n",addressInMemory);			
		printf("Value:   %c\n",addressToMemory[index]);

		printf("Enter new value: ");
		fgets (buf2, sizeof (buf2), stdin);
		printf("You entered: ***%s***",trimwhitespace(buf2));
		//trimwhitespace(inputForNewValue);
		bufferHolder[1] =* trimwhitespace(buf2);
		option = (char)(bufferHolder[1]);
		printf("Final choice: ***%c***\n",option);
		if (option == '.')
		{
			return 1;
		}
		addressToMemory[index] = option;
		printf("New Value:   %c\n",addressToMemory[index]);
	}


    void displayFunctions()
    {
	printf("These are the commands you can enter:\n");
	printf("d - dump memory\n");
	printf("g - run the entire program\n");
	printf("l - load a file into memory\n"); 
	printf("m - memory modify\n");
	printf("q - quit\n");
	printf("r - display registers\n");	
	printf("t - trace\n");
	printf("w - write file\n");
	printf("z - reset all registers to zero\n");
	printf("h,? - display list of commads\n");	
    }





    void menuFunction()
    {
	while(1)
	{
	    fflush(stdout);
	    printf(">:");

	    if (fgets (buf, sizeof(buf), stdin) != NULL)
	    {

		//		printf("You entered: %s\n", trimwhitespace(buf));
		bufferHolder[1] =* trimwhitespace(buf);
		

	
		option = (char)(bufferHolder[1]);
		option =  (tolower(option));
//		printf("Final choice: ***%c***\n",option);
		

	    }	


	    switch (option)
	    {
		case 'd':
		    printf("Memory Dump\n");	
		    getOffset(&addressInMemory);
		    getLength(&lengthOfMessage);  	   		  		    		
		    memoryDump(&memory,addressInMemory,lengthOfMessage);
		    break;
		case 'q':
		    exit(0);	
		case 'h':
		    displayFunctions();
		    break;			
		case 'g':
		    printf("Runs the entire program\n");
		    break;
		//Loads a file into memory
		case 'l':	
		    printf("Load goes here\n");
		    LoadFile(memory, sizeof(memory));
		    break;
		case 'm':
		    printf("Memory Modify\n");
		    getOffset(&addressInMemory);
	  	    memoryModify(&memory, addressInMemory);
		    break;
		case 'r':
		    printf("display registers\n");
		    break;
		case 't':
		    printf("Trace stuff\n");
		    break;
		//Create a file with size specified 
		case 'w':
		    printf("Write to a file\n");
		    WriteFile(memory);
		    break;
		case 'z':
		    printf("Registers are 0\n");
		    break;	
		case '?':
		    displayFunctions();
		    break;				
	    }	
	}
    }
	
    printf("Name: Abel Deglado\n");
    printf("Enter h for help\n");
    menuFunction();
    //	displayFunctions();
}

