#include <stdio.h>
#include <stdlib.h>

char hex2minus(unsigned char val)
{
	return (~(val-1) & 0xff);
}

unsigned char minus2hex(char val)
{
	return (~((val+1)*(-1)));
}

int main(int argc, char *argv[])
{
	if (argc == 3)
		printf(" hex2minus = -%d, minus2hex = 0x%x\n",
			hex2minus(strtol(argv[1], NULL, 16)), minus2hex(atoi(argv[2])));
	else
		printf("usage: ./a.out 0xyy -xx\n");

	return 0;
}

