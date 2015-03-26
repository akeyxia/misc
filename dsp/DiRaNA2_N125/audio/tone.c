#include <stdio.h>
#include <math.h>

#include "Audio_7741.h"
#include "frac2hex.c"

/*
 * Second order peaking filter for Tone Treble
 */
static void treble_2nd_peaking(float fc, float Q, float gt)
{
	float gtre, D, t, a2t, a1t, b0t, b2t;
	int a1, a2, b0, b1, b2, Gtre;

	if (Q < 0.1f || Q > 10.0f) {
		printf("%s: Qfactor 0.1-10.0 is supported\n", __func__);
		return;
	}

	gtre = (powf(10.0f, gt / 20.0f) - 1.0f) / 16.0f;
	if (gtre >= 0)
	  D = 1.0f;
	else
	  D = 1.0f / (16.0f * gtre + 1.0f);
	t = 2.0f * M_PI * fc / FS;
	a2t = -0.5f * (1.0f - D * t / (2 * Q)) / (1.0f + D * t / (2 * Q));
	a1t = (0.5f - a2t) * cosf(t);
	b0t = 0.25f + 0.5f * a2t;
	b2t = -b0t;

	a1 = YMEM2HEX(a1t);
	a2 = YMEM2HEX(a2t);
	b0 = YMEM2HEX(b0t);
	b1 = 0x000;
	b2 = YMEM2HEX(b2t);
	Gtre = YMEM2HEX(gtre);

	//printf("Tone Treble second order peaking filter:\n");
	/* BMT_a1tP, BMT_a2tP, BMT_b0tP, BMT_b1tP, BMT_b2tP */
	printf("{0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X}, /* %.1fdB, %.1fHz */\n",
				a1, a2, b0, b1, b2, Gtre, gt, fc);
}

/*
 * Second order peaking filter for Tone Bass
 */
static void bass_2nd_peaking(float fc, float Q)
{
	double t, a2b, a1b, b0b, b2b;
	int b0h, b0l, b1h, b1l, b2h, b2l, a1h, a1l, a2h, a2l;

	t = 2.0f * M_PI * fc / FS;
	a2b = -0.5f * (1.0f - t / (2 * Q)) / (1.0f + t / (2 * Q));
	a1b = (0.5f - a2b) * cos(t);
	b0b = (0.5f + a2b) / 2.0f;
	b2b = -b0b;

	a1h = DYMEM2HEX(a1b) >> 12;
	a1l = DYMEM2HEX(a1b) & 0xFFF;
	a2h = DYMEM2HEX(a2b) >> 12;
	a2l = DYMEM2HEX(a2b) & 0xFFF;
	b0h = DYMEM2HEX(b0b) >> 12;
	b0l = DYMEM2HEX(b0b) & 0xFFF;
	b1h = 0x000;
	b1l = 0x000;
	b2h = DYMEM2HEX(b2b) >> 12;
	b2l = DYMEM2HEX(b2b) & 0xFFF;

	//printf("Tone Bass second order peaking filter:\n");
	/* BMT_a1bH, BMT_a1bL, BMT_a2bH, BMT_a2bL, BMT_b0bH, BMT_b0bL, BMT_b1bH, BMT_b1bL, BMT_b2bH, BMT_b2bL */
	printf("0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, /* %.1fHz */\n",
			a1h, a1l, a2h, a2l, b0h, b0l, b1h, b1l, b2h, b2l, fc);
}

/*
 * Second order peaking filter for Tone Mid
 */
static void mid_2nd_peaking(float fc, float Q, float gm)
{
	double D, t, a2m, a1m, b0m;
	float gmid;
	int b0h, b0l, a1h, a1l, a2h, a2l, Gmid;

	gmid = (powf(10.0f, gm / 20.0f) - 1.0f) / 16.0f;
	if (gm >= 0)
	  D = 1;
	else
	  D = 1.0f / (16 * gmid + 1.0f);
	t = 2.0f * M_PI * fc / FS;
	a2m = -0.5f * (1.0f - D * t / (2 * Q)) / (1.0f + D * t / (2 * Q));
	a1m = (0.5f - a2m) * cos(t);
	b0m = 0.25f + 0.5f * a2m;

	a1h = DYMEM2HEX(a1m) >> 12;
	a1l = DYMEM2HEX(a1m) & 0xFFF;
	a2h = DYMEM2HEX(a2m) >> 12;
	a2l = DYMEM2HEX(a2m) & 0xFFF;
	b0h = DYMEM2HEX(b0m) >> 12;
	b0l = DYMEM2HEX(b0m) & 0xFFF;
	//Gmid = DYMEM2HEX(gmid) >> 12;
	Gmid = YMEM2HEX(gmid);

	//printf("Tone Mid second order peaking filter:\n");
	/* BMT_a1mH, BMT_a1mL, BMT_a2mH, BMT_a2mL, BMT_b0mH, BMT_b0mL, BMT_Gmid */
	printf("{0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X}, /* %.1fdB, %.1fHz */\n",
			a1h, a1l, a2h, a2l, b0h, b0l, Gmid, gm, fc);
}

/*
 * Tone Bass boost gain
 */
static void bass_boost(float gb)
{
	float gbas;
	int Gbas;

	if (gb >= 0 )
	  gbas = (powf(10.0f, gb / 20.0f) - 1.0f) / 16.0f;
	else
	  gbas = -(powf(10.0f, -gb / 20.0f) - 1.0f) / 16.0f;
	Gbas = YMEM2HEX(gbas);

	printf("0x%03X, /* %.1fdB */\n", Gbas, gb);
}

int main(int argc, char *argv[])
{
	float gain;

	printf("Example:\n");
	bass_2nd_peaking(80, 1.0f);
	mid_2nd_peaking(800, 1.0f, -6.0f);
	treble_2nd_peaking(5000, 1.0f, 6.0f);

	printf("Tone Bass setting:\n");
		bass_2nd_peaking(ToneBassFc, 1.0f);
	printf("Tone Bass Boost gain in dB unit:\n");
	for (gain = -ToneBassMaxdb; gain <= ToneBassMaxdb; gain += 1.0f) {
		bass_boost(gain);
	}
	
	printf("Tone Mid setting:\n");
	for (gain = -ToneMidMaxdb; gain <= ToneMidMaxdb; gain += 1.0f)
		mid_2nd_peaking(ToneMidFc, 1.0f, gain);
	
	printf("Tone Treble setting:\n");
	for (gain = -ToneTrebleMaxdb; gain <= ToneTrebleMaxdb; gain += 1.0f)
		treble_2nd_peaking(ToneTrebleFc, 1.0f, gain);

	return 0;
}
