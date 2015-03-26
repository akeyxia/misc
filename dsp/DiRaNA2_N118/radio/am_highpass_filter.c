#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "mem2hex.c"

void am_highpass_filter(float fc)
{
	double fracA;
	double fracB;

	/*
	* AmSig_yDcA = -0.00004936*Fc + 0.99999290
	* AmSig_yDcB = 2* AmSig_yDcA-1
	*/
	fracA = -0.00004936 * fc + 0.99999290;
	fracB = 2.0f * fracA - 1.0f;
	printf("%s: fc = %f, AmSig_yDcA = 0x%X, AmSig_yDcB = 0x%X\n",
		__func__, fc, YMEM2Hex(fracA), YMEM2Hex(fracB));

	return;
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("usage: ./xxx fc\n");
		return -1;
	}

	am_highpass_filter(atof(argv[1]));

	return 0;
}
