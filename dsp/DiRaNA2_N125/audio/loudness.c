#include <stdio.h>
#include <math.h>

#include "Audio_7741.h"
#include "frac2hex.c"

/*
 * First order shelving filter for treble
 */
static void treble_1st_shelving(float fc)
{
	float a1f, b0f, b1f;
	int a1, a2, b0, b1, b2;

	a1f = -0.5f * tanf(M_PI * (fc / FS - 0.25f));
	b0f = 0.25f + 0.5f * a1f;
	b1f = -b0f;

	a1 = YMEM2HEX(a1f);
	a2 = YMEM2HEX(0.0f);
	b0 = YMEM2HEX(b0f);
	b1 = YMEM2HEX(b1f);
	b2 = YMEM2HEX(0.0f);

	printf("Treble first order shelving filter:\n");
	printf("0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x%03x, /* %.1fHz */\n", a1, a2, b0,
			b1, b2, fc);
}

/*
 * Second order peaking filter for treble
 */
static void treble_2nd_peaking(float fc, float Q)
{
	float t0f, a1f, a2f, bf;
	int a1, a2, b0, b1, b2;

	if (Q < 0.1f || Q > 10.0f) {
		printf("%s: Qfactor 0.1-10.0 is supported\n", __func__);
		return;
	}

	t0f = 2.0f * M_PI * fc / FS;
	a2f = -0.5f * (1.0f - t0f / (2 * Q)) / (1.0f + t0f * (2 * Q));
	bf = (0.5f + a2f) / 2;
	a1f = (0.5f - a2f) * cosf(t0f);

	a1 = YMEM2HEX(a1f);
	a2 = YMEM2HEX(a2f);
	b0 = YMEM2HEX(bf);
	b1 = YMEM2HEX(0.0f);
	b2 = YMEM2HEX(-bf);

	printf("Treble second order peaking filter:\n");
	printf("0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x%03x, /* %.1fHz */\n", a1, a2, b0,
			b1, b2, fc);
}

/*
 * First order shelving filter for bass
 */
static void bass_1st_shelving(float fc)
{
	double t0d, a1d, b0d;
	int a1h, a1l, a2h, a2l, b0h, b0l, b2h, b2l;

	t0d = cosf(2.0f * M_PI * fc / FS);
	a1d = 1.0f - (t0d + sqrtf(powf(t0d - 2.0f, 2) - 1.0f)) / 2.0f;
	b0d = 0.5f - a1d;

	a1h = DYMEM2HEX(a1d) >> 12;
	a1l = DYMEM2HEX(a1d) & 0xFFF;
	a2h = YMEM2HEX(0.0f);
	a2l = YMEM2HEX(0.0f);
	b0h = DYMEM2HEX(b0d) >> 12;
	b0l = DYMEM2HEX(b0d) & 0xFFF;
	b2h = YMEM2HEX(0.0f);
	b2l = YMEM2HEX(0.0f);

	printf("Bass first order shelving filter:\n");
	printf("0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x%03x, /* %.1fHz */\n",
			a1l, a1h, a2l, a2h, b0l, b0h, b2l, b2h, fc);
}

/*
 * Second order shelving filter for bass
 */
static void bass_2nd_shelving()
{
	printf("%s: please refer P85 for preset tables\n", __func__);
}

/*
 * Second order peaking filter for bass
 */
static void bass_2nd_peaking(float fc, float Q)
{
	double t0d, a2d, a1d, b0d;
	int b0h, b0l, b2h, b2l, a1h, a1l, a2h, a2l;

	t0d = 2.0f * M_PI * fc / FS;
	a2d = -0.5f * (1.0f - t0d / (2 * Q)) / (1.0f + t0d / (2 * Q));
	a1d = (0.5f - a2d) * cos(t0d);
	b0d = (0.5f + a2d) / 2.0f;

	a1h = DYMEM2HEX(a1d) >> 12;
	a1l = DYMEM2HEX(a1d) & 0xFFF;
	a2h = DYMEM2HEX(a2d) >> 12;
	a2l = DYMEM2HEX(a2d) & 0xFFF;
	b0h = DYMEM2HEX(b0d) >> 12;
	b0l = DYMEM2HEX(b0d) & 0xFFF;
	b2h = DYMEM2HEX(-b0d) >> 12;
	b2l = DYMEM2HEX(-b0d) & 0xFFF;

	printf("Bass second order peaking filter:\n");
	printf("0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x%03x, /* %.1fHz */\n",
			a1l, a1h, a2l, a2h, b0l, b0h, b2l, b2h, fc);
}

/*
 * Bass and Treble max gain
 */
static void loudness_max()
{
	float bass_maxf, treble_maxf;
	int bass_max, treble_max;

	bass_maxf = (powf(10.0f, LoudnessBassMaxdb / 20.0f) - 1.0f) / 8.0f;
	treble_maxf = (powf(10.0f, LoudnessTrebleMaxdb / 20.0f) - 1.0f) / 4.0f;

	bass_max = YMEM2HEX(bass_maxf);
	treble_max = YMEM2HEX(treble_maxf);

	printf("Y:Loudf_MaxBstB = 0x%03x /* %.1fdB */\n", bass_max, LoudnessBassMaxdb);
	printf("Y:Loudf_MaxBstT = 0x%03x /* %.1fdB */\n", treble_max, LoudnessTrebleMaxdb);
}

/*
 * Bass boost gain
 *
 * The Treble boost gain is relational with Bass boost gain, so only one
 * value is enough.
 */
static void loudness_boost(float bass_gain)
{
	float bass_maxf, treble_maxf, bass_gainf, treble_gain;
	int bass_max, treble_max, bass_value;

	bass_maxf = (powf(10.0f, LoudnessBassMaxdb / 20.0f) - 1.0f) / 8.0f;
	treble_maxf = (powf(10.0f, LoudnessTrebleMaxdb / 20.0f) - 1.0f) / 4.0f;
	bass_max = YMEM2HEX(bass_maxf);
	treble_max = YMEM2HEX(treble_maxf);

	bass_gainf = 1.0f - sqrtf((powf(10.0f, bass_gain / 20.0f) - 1.0f) / (powf(10.0f, LoudnessBassMaxdb / 20.0f) - 1.0f));

	treble_gain = 20.0f * logf(treble_max / (2.0f * bass_max) * (powf(10.0f, bass_gain / 20.0f) - 1.0f) + 1.0f)
		/ logf(10.0f);

	bass_value = YMEM2HEX(bass_gainf);

	printf("0x%03x, /* gbb = %.1fdB, gtb = %.1fdB */\n", bass_value, bass_gain, treble_gain);
}

int main(int argc, char *argv[])
{
	float bass_gains[33] = {
		0.0f, 0.0f, 0.0f, 0.0f,
		10.0f, 10.0f, 10.0f, 10.0f,
		9.5f, 9.5f, 9.0f, 9.0f,
		8.5f, 8.0f, 7.0f, 7.0f,
		6.0f, 6.0f, 6.0f, 4.0f,
		4.0f, 3.0f, 2.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f,
	};
	int i;
	float j;

	bass_2nd_peaking(LoudnessBassFc, 1.0f);
	treble_2nd_peaking(LoudnessTrebleFc, 1.0f);
	loudness_max();

	printf("Loudness Boost for Volume setting from 0 to 32:\n");
	for (i = 0; i < 33; i++) {
		loudness_boost(bass_gains[i]);
	}

	printf("Loudness Boost gain in dB unit:\n");
	for (j = 0.0f; j <= LoudnessBassMaxdb;) {
		loudness_boost(j);
		j += 0.5f;
	}
}
