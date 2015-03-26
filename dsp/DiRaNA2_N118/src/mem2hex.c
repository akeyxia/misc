#include <math.h>

#define YMEM_COEFF 2048
#define XMEM_COEFF 8388608

/*
 * HEX number = HEX[min(round(Fraction*2048), 2047)]
 *
 * YMEM support 12bit data
 */
int YMEM2Hex(float frac)
{
	int value = (int)roundf(frac * YMEM_COEFF);

	if (value < YMEM_COEFF - 1)
		return value & 0xFFF;
	else
		return (YMEM_COEFF - 1) & 0xFFF;
}

/*
 * HEX number = HEX[min(round(Fraction*8388608), 8388607]
 *
 * Double precision YMEM support two 12bit data
 */
int DYMEM2Hex(double frac)
{
	int value = (int)round(frac * XMEM_COEFF);
	int low, high;

	if (value < XMEM_COEFF - 1)
		value = value & 0xFFFFFF;
	else
		value = (XMEM_COEFF - 1) & 0xFFFFFF;

	low = value & 0xFFF;
	high = value & 0xFFF000;
	value = high | (low >> 1);

	return value;
}

/*
 * HEX number = HEX[min(round(Fraction*8388608), 8388607]
 *
 * XMEM support 24bit data
 */
int XMEM2Hex(float frac)
{
	int value = (int)roundf(frac * XMEM_COEFF);

	if (value < XMEM_COEFF - 1)
		return value & 0xFFFFFF;
	else
		return (XMEM_COEFF - 1) & 0xFFFFFF;
}
