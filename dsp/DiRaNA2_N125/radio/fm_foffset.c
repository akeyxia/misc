#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "hex2frac.c"

void fm_freq_offset(int value)
{
	float frac;
	float offset;
	
	frac = XHEX2Frac(value);
	if (frac < 0)
		frac = -(frac + 1);

	offset = 162.5 * frac;

	printf("FM freq offset = %f kHz(%f)\n", offset, frac);
}

int main(int argc, char *argv[])
{
	if (argc == 2)
		fm_freq_offset(strtol(argv[1], NULL, 16));
	else
		printf("usage: ./a.out xxx(foffset hex value)\n");

	return 0;
}

