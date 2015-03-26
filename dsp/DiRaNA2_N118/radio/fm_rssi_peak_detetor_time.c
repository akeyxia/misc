#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "mem2hex.c"

void am_av_time(float tc)
{
	double fracAtk;
	double fracDcy;

	fracAtk = 1.0f - expf(-1.0f / (tc * 20312.5f));
	fracDcy = fracDcy;
	printf("tc = %f s, frac = %f, e = %f\n", tc, fracAtk, expf(1.0f));
	printf("%s: tc = %f, FmHbc_yRssiAvAtk = 0x%X, FmHbc_yRssiAvDcy = 0x%X\n",
		__func__, tc, YMEM2Hex(fracAtk), YMEM2Hex(fracDcy));

	return;
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("usage: ./xxx tc\n");
		return -1;
	}

	am_av_time(atof(argv[1]));

	return 0;
}
