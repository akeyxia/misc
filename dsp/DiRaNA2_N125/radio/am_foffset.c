#include <stdio.h>
#include <stdlib.h>

#include "hex2frac.c"

void am_freq_offset(int value)
{
	double frac;
	double offset;

	frac = XHEX2Frac(value);
	if (frac < 0)
		frac = -(frac + 1);

	offset = 20.3125 * frac;

	printf("AM freq offset = %f Hz(%f)\n", offset*1000, frac);
}

int main(int argc, char *argv[])
{
	if (argc == 2)
		am_freq_offset(strtol(argv[1], NULL, 16));
	else
		printf("usage: ./a.out xxx(foffset hex value)\n");

	return 0;
}

