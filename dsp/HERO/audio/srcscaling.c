#include <stdio.h>
#include <math.h>

#include "frac2hex.c"

/*
* Input: P(Primary 2 channel), P6(Primary 6 channel),
*        S(Secondary), N(Navigation), T(Phone)
* Output coefficient: Y:Vol_SrcScal<n>
*/
void SourceScaling(float min, float max, float step)
{
	float value;

	value = min;
	while (value <= max) {
		printf("0x%03X, /* %6.2f dB */\n",
			YMEM2HEX(pow10f((value - 6.02)/20.0)), value);

		value += step;
	}
}

int main(int argc, char *argv[])
{
	SourceScaling(-60.0, 6.0, 0.5);

	return 0;
}

