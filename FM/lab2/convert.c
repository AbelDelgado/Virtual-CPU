
#include <stdio.h>
int main (void)
{
	short ishort = 0xFFFF;
	unsigned short ushort = 0xFFFF;
	char vchar1 = 0x80;
	char vchar2 = 0x85;
	char vchar3 = 0x35;
	

	printf("The short value is %X Hex (decimal %d)\n",ishort,ishort );
	printf("The unsigned short value is %X Hex (decimal %d)\n",ushort, ushort );
	printf("The char value is %X Hex (decimal %d)\n", vchar1,vchar1 );
	printf("The value of char 85 hex is %X Hex (decimal %d)\n", vchar2, vchar2);
	printf("The value of char 35 hex is %X Hex (decimal %d)\n", vchar3,vchar3 );

}