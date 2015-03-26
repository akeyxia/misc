#include <stdio.h>
#include <math.h>

#include "saf7741.h"

/*
 * Translate the dB gain to YMEM value for Speed Compensation
 * Volume
 */
void DB2Val_SCV(float min, float step, float max)
{
	int gain;
	float volume;

	printf("Speed Compensation Volume gains:\n");
	volume = min;
	while (volume <= max) {
		gain = YMEM2Hex(powf(10.0f, volume / 20.0f));
		printf("0x%03x, /* %6.2f dB */\n", gain, volume);

		volume += step;
	}
}

int main(int argc, char *argv[])
{
	DB2Val_SCV(-SCV0dbLevel, 1.0f, SCVMaxdb);

	return 0;
}
