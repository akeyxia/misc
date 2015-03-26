#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "saf7741.h"

/*
 * Translate the dB gain to YMEM value for Speed Compensation
 * Volume
 */
void mute_ta_tr(double time)
{
	int T;

	printf("Calculation Primary overall, Navigation, Phone, and Rear Seat Audio Mute rise and fall times\n");
	T = XMEM2Hex(0.8f * 2.0f / time / 48.0f);
	printf("0x%06x, /* %f ms, sample: 48000 */\n", T, time);
	T = XMEM2Hex(0.8f * 2.0f / time / 44.1f);
	printf("0x%06x, /* %f ms, sample: 44100 */\n", T, time);

	printf("Calculation Front, Rear, Subwoofer and Additional Independent Channel Mutes rise and fall times\n");
	T = XMEM2Hex(0.8f * 4.0f / time / 48.0f);
	printf("0x%06x, /* %f ms, sample: 48000 */\n", T, time);
	T = XMEM2Hex(0.8f * 4.0f / time / 44.1f);
	printf("0x%06x, /* %f ms, sample: 44100 */\n", T, time);
}

int main(int argc, char *argv[])
{
	double time;

	if (argc != 2) {
		printf("usage: ./xxx xx(ms)\n");
		return -1;
	}

	time = atof(argv[1]);
	mute_ta_tr(time);

	return 0;
}
