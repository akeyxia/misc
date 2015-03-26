#include <stdio.h>
#include <math.h>

#include "saf7741.h"

/*
 * Translate the dB gain to YMEM value for fader and balance
 */
void DB2Val_Balance(float min, float step, float max)
{
	int gain;
	float volume;

	volume = min;
	while (volume <= max) {
		gain = YMEM2Hex(-powf(10.0f, volume / 20.0f));
		printf("0x%03x, /* %6.2f dB */\n", gain, volume);

		volume += step;
	}
}

int main(int argc, char *argv[])
{
	DB2Val_Balance(-Balance0dbLevel, 1.0f, BalanceMaxdb);

	return 0;
}
