#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "frac2hex.c"

void frac2hex(int type, double frac)
{
	int xHex;

	if (type == 0) {
		xHex = XMEM2HEX(frac);
		printf("frac = %f, XHEX = 0x%X\n", frac, xHex);
		printf("MSB = 0x%X, LSB = 0x%X\n", xHex >> 12, (xHex & 0xFFF) >> 1);
	} else {
		printf("frac = %f, YHEX = 0x%X\n", frac, YMEM2HEX(frac));
	}
}

int main(int argc, char *argv[])
{
	int type;
	double frac;

	if (argc != 3) {
		printf("usage: ./xxx XYtype XYfrac\n");
		printf("XYtype: 0--X, 1--Y\n");
		return -1;
	}

	type = atoi(argv[1]);
	frac = atof(argv[2]);

	frac2hex(type, frac);

	return 0;
}

