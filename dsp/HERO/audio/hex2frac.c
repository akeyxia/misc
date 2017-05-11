#include <math.h>

#define YMEM_COEF	2048
#define XMEM_COEF	8388608

/* HEX[min(round(Fraction*2048), 2047)] */
float YHEX2Frac(int hex)
{
/*
	int value = (int)roundf(frac * YMEM_COEF);

	if (value >= YMEM_COEF)
		value = YMEM_COEF - 1;

	return value & 0xFFF;
*/
	float frac;

	if (hex <= 0 || hex > YMEM_COEF << 1) {
		printf("The value is out of range(0, 0x%X]\n", YMEM_COEF << 1);
		return 0;
	}

	frac = (float)hex / YMEM_COEF;
	
	if (hex >= YMEM_COEF)
		frac = 1 - frac;

	return frac;
}

/* HEX[min(round(Fraction*8388608), 8388607)] */
double XHEX2Frac(int hex)
{
/*
	int value = (int)round(frac * XMEM_COEF);

	if (value >= XMEM_COEF)
		value = XMEM_COEF - 1;

	return value & 0xFFFFFF;
*/
	double frac;

	if (hex <= 0 || hex > XMEM_COEF << 1) {
		printf("The value is out of range(0, 0x%X]\n", XMEM_COEF << 1);
		return 0;
	}

	frac = (double)hex / XMEM_COEF;

	if (hex >= XMEM_COEF)
		frac = 1 - frac;

	return frac;
}


