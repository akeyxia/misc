#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "mem2hex.c"

void peak(float tc)
{
	double fracAtk;
	double fracDcy;

	fracAtk = 16.0f * (1.0f - expf(-1.0f / (tc * 20312.5f)));
	fracDcy = 2048.0f * (1.0f - expf(-1.0f / (tc * 20312.5f)));
	printf("tc = %f s, fracAtk = %f, fracDcy = %f\n", tc, fracAtk, fracDcy);
	printf("%s: tc = %f, FmHbc_yRssiAggMax = 0x%X, FmHbc_yRssiDcyTmp = 0x%X\n",
		__func__, tc, YMEM2Hex(fracAtk), YMEM2Hex(fracDcy));

	return;
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("usage: ./xxx tc\n");
		return -1;
	}

	peak(atof(argv[1]));

	return 0;
}
