#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "radio/hex2frac.c"

void volumeHex2db(int value)
{
	float frac;
	float voldB;
	
	frac = YHEX2Frac(value);
	voldB = 20.0f * log10(frac);

	printf("voldB = %f, frac = %f\n", voldB, frac);
	printf("dbLin = %f\n", -20.0f * log10(value));
}

int main(int argc, char *argv[])
{
	if (argc == 2)
		volumeHex2db(strtol(argv[1], NULL, 16));
	else
		printf("usage: ./a.out xxx(volume hex value)\n");

	return 0;
}

