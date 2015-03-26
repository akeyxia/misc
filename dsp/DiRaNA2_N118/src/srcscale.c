#include <stdio.h>
#include <math.h>

#include "saf7741.h"

/*
 * Translate the dB gain to YMEM value for Primary Channel Source Scale
 */
void DB2Val_SrcScaleP(float min, float step, float max)
{
	int gain;
	float volume;

	printf("Primary Channel Source Scale gains:\n");
	volume = min;
	while (volume <= max) {
		gain = YMEM2Hex(powf(10.0f, (volume - SrcScalPMaxdb) / 20.0f));
		printf("0x%03x, /* %6.2f dB */\n", gain, volume);

		volume += step;
	}
}

/*
 * Translate the dB gain to YMEM value for Primary Channel Source Scale
 */
void DB2Val_SrcScaleRSA(float min, float step, float max)
{
	int gain;
	float volume;

	printf("RSA Channel Source Scale gains:\n");
	volume = min;
	while (volume <= max) {
		gain = YMEM2Hex(powf(10.0f, volume / 20.0f));
		printf("0x%03x, /* %6.2f dB */\n", gain, volume);

		volume += step;
	}
}

/*
 * Translate the dB gain to YMEM value for Channel Scale
 */
void DB2Val_ChannelScale(float min, float step, float max)
{
	int gain;
	float volume;

	printf("Channel Scale gains:\n");
	volume = min;
	while (volume <= max) {
		gain = YMEM2Hex(powf(10.0f, (volume - ChannelScalMaxdb) / 20.0f));
		printf("0x%03x, /* %6.2f dB */\n", gain, volume);

		volume += step;
	}
}

int main(int argc, char *argv[])
{
	DB2Val_SrcScaleP(-24.0f, 1.0f, SrcScalPMaxdb);
	DB2Val_SrcScaleRSA(-66.0f, 1.0f, SrcScalRSAMaxdb);
	DB2Val_ChannelScale(-12.0f, 1.0f, ChannelScalMaxdb);

	return 0;
}
