#include <stdio.h>

int main()
{
	unsigned short ushort;
	unsigned char uchar;

	ushort = 0x68;
	uchar = ushort;

	printf("The value of %c in hex is %X\n", uchar, ushort);
}
