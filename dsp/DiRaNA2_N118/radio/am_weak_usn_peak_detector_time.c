#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "mem2hex.c"

void time(float tc)
{
	double fracAtk;
	double fracDcy;

	fracAtk = (1.0f - expf(-1.0f / (tc * 40625.0f)));
	fracDcy = fracAtk;
	printf("tc = %f s, fracAtk = %f, fracDcy = %f\n", tc, fracAtk, fracDcy);
	printf("%s: tc = %f, AmNfd_yUsnAvAtk = 0x%X, AmNfd_yUsnAvDcy = 0x%X\n",
		__func__, tc, YMEM2Hex(fracAtk), YMEM2Hex(fracDcy));

	return;
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("usage: ./xxx tc\n");
		return -1;
	}

	time(atof(argv[1]));

	return 0;
}
