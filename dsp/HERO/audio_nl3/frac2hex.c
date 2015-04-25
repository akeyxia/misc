#include <math.h>

#define YMEM_COEF	2048
#define XMEM_COEF	8388608

/* HEX[min(round(Fraction*2048), 2047)] */
int YMEM2HEX(float frac)
{
	int value = (int)roundf(frac * YMEM_COEF);

	if (value >= YMEM_COEF)
		value = YMEM_COEF - 1;

	return value & 0xFFF;
}

/* HEX[min(round(Fraction*8388608), 8388607)] */
int XMEM2HEX(double frac)
{
	int value = (int)round(frac * XMEM_COEF);

	if (value >= XMEM_COEF)
		value = XMEM_COEF - 1;

	return value & 0xFFFFFF;
}

int DYMEM2HEX(double frac)
{
	int value = XMEM2HEX(frac);
	int low, high;

	low = value & 0xFFF;
	high = value & 0xFFF000;
	value = high | (low >> 1);

	return value;
}

