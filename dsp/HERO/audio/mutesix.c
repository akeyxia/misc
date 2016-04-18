#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "frac2hex.c"

void MuteSixGain(float min, float max, float step)
{
	float value;

	printf("%s :\n", __func__);

	value = min;
	while(value <= max) {
		printf("0x%03X,	/* %6.2f dB */\n",
			//YMEM2HEX(-pow10f(value/20.0)), value);
			YMEM2HEX(pow10f(value/20.0)), value);

		value += step;
	}
}

int main(int argc, char *argv[])
{
	float gain;

	if (argc == 2) {
		gain = atof(argv[1]);
		MuteSixGain(gain, gain, 1.0);
	} else
		MuteSixGain(-66.0, 0.0, 1.0);

	return 0;
}

