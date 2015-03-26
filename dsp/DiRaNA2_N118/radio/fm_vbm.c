#include <stdio.h>
#include <stdlib.h>

#include "mem2hex.c"

int main(int argc, char *argv[])
{
	int fc;
	float frac;

	if (argc != 2) {
		printf("usage: ./xxx fc\n");
		return -1;
	}

	fc = atoi(argv[1]);
	frac = fc/32500.0f;/* BandWidth/325000 ? */
	printf("fc = %d, CoefHEX = 0x%X\n", fc, YMEM2Hex(frac));

	return 0;
}


