#include <stdio.h>
#include <math.h>

#include "tef6638.h"
#include "frac2hex.c"

#define _DEBUG

#ifdef _DEBUG
#define LoudnessBassStep	(0.1f)
#define LoudnessTrebleStep	(0.1f)
#else
#define LoudnessBassStep	(0.5f)
#define LoudnessTrebleStep	(0.5f)
#endif

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

	//printf("Treble first order shelving filter:\n");
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

	//printf("Treble second order peaking filter:\n");
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

	printf("bass_maxf = %f, treble_maxf = %f\n", bass_maxf, treble_maxf);
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

static void loudness_independent_bass_boost(float bass_gain)
{
	float bass_maxf, bass_gainf;
	int bass_value;

	bass_maxf = (powf(10.0f, LoudnessBassMaxdb / 20.0f) - 1.0f) / 8.0f;

	bass_gainf = 1.0f - sqrtf((powf(10.0f, bass_gain / 20.0f) - 1.0f) / (powf(10.0f, LoudnessBassMaxdb / 20.0f) - 1.0f));

	bass_value = YMEM2HEX(bass_gainf);

	printf("0x%03x, /* gbb = %.1fdB */\n", bass_value, bass_gain);
	
}

static void loudness_independent_treble_boost(float treble_gain)
{
	float treble_maxf, treble_gainf;
	int treble_value;

	treble_maxf = (powf(10.0f, LoudnessTrebleMaxdb / 20.0f) - 1.0f) / 4.0f;

	treble_gainf = 1.0f - sqrtf((powf(10.0f, treble_gain / 20.0f) - 1.0f) / (powf(10.0f, LoudnessTrebleMaxdb / 20.0f) - 1.0f));

	treble_value = YMEM2HEX(treble_gainf);

	printf("0x%03x, /* gtb = %.1fdB */\n", treble_value, treble_gain);
	
}

int main(int argc, char *argv[])
{
#ifdef _DEBUG
	float bass_gains[33] = {
		0.0f, 10.0f, 10.0f, 10.0f,
		10.0f, 10.0f, 10.0f, 9.9f,
		9.7f, 9.3f, 8.8f, 8.3f,
		7.7f, 7.0f, 6.0f, 5.0f,
		4.0f, 3.0f, 2.2f, 1.3f,
		0.8f, 0.3f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f,
	};
	float treble_gains[33] = {
		0.0f, 4.0f, 4.0f, 4.0f,
		4.0f, 4.0f, 4.0f, 3.9f,
		3.8f, 3.6f, 3.4f, 3.1f,
		2.8f, 2.5f, 2.1f, 1.7f,
		1.3f, 0.9f, 0.7f, 0.4f,
		0.2f, 0.1f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f,
	};
#else
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
	float treble_gains[33] = {
		0.0f, 0.0f, 0.0f, 0.0f,
		4.0f, 4.0f, 4.0f, 4.0f,
		3.0f, 3.0f, 3.0f, 3.0f,
		3.0f, 3.0f, 2.0f, 2.0f,
		2.0f, 2.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f,
	};
#endif
	float table_treble_fts[31] = {
		100.0f, 200.0f,
		300.0f, 400.0f,
		500.0f, 600.0f,
		700.0f, 800.0f,
		900.0f, 1000.0f,
		1500.0f, 2000.0f,
		2500.0f, 3000.0f,
		3500.0f, 4000.0f,
		4500.0f, 5000.0f,
		5500.0f, 6000.0f,
		6500.0f, 7000.0f,
		7500.0f, 8000.0f,
		8500.0f, 9000.0f,
		9500.0f, 10000.0f,
		15000.0f, 20000.0f,
		22050.0f,
	};
	float table_treble_fcs[31] = {
		100.0f, 200.0f,
		300.0f, 400.0f,
		500.0f, 600.0f,
		700.0f, 800.0f,
		900.0f, 1000.0f,
		1500.0f, 2000.0f,
		2500.0f, 3000.0f,
		3500.0f, 4000.0f,
		4500.0f, 5000.0f,
		5500.0f, 6000.0f,
		6500.0f, 7000.0f,
		7500.0f, 8000.0f,
		8500.0f, 9000.0f,
		9500.0f, 10000.0f,
		15000.0f, 20000.0f,
		22050.0f,
	};
	int i;
	float j;

#ifdef _DEBUG
	bass_2nd_peaking(80.0f, 1.0f);
#else
	bass_2nd_peaking(LoudnessBassFc, 1.0f);
#endif
	treble_2nd_peaking(LoudnessTrebleFc, 1.0f);
	loudness_max();

#ifdef COUPLED_CONTROL
	printf("Loudness Boost for Volume setting from 0 to 32:\n");
	for (i = 0; i < 33; i++) {
		loudness_boost(bass_gains[i]);
	}

	printf("Loudness Boost gain in dB unit:\n");
	for (j = 0.0f; j <= LoudnessBassMaxdb;) {
		loudness_boost(j);
		j += 0.5f;
	}
#else
#if 0
	printf("First order shelving filter coefficients for treble:\n");
	for(i = 0; i < 31; i++)
		treble_1st_shelving(table_treble_fts[i]);	

	printf("Second order band-pass filter coefficients for treble:\n");
	for(i = 0; i < 31; i++)
		treble_2nd_peaking(table_treble_fcs[i], 1.0f);/* Highband center frequency >= 0.022*FS */
#endif
	printf("Loudness Bass Boost for Volume setting from 0 to 32:\n");
	for (i = 0; i < 33; i++) {
		loudness_independent_bass_boost(bass_gains[i]);
	}

	printf("Loudness Treble Boost for Volume setting from 0 to 32:\n");
	for (i = 0; i < 33; i++) {
		loudness_independent_treble_boost(treble_gains[i]);
	}

	printf("Loudness Bass Boost gain in dB unit:\n");
	for (j = 0.0f; j <= LoudnessBassMaxdb;) {
		loudness_independent_bass_boost(j);
		j += LoudnessBassStep;
	}

	printf("Loudness Treble Boost gain in dB unit:\n");
	for (j = 0.0f; j <= LoudnessTrebleMaxdb;) {
		loudness_independent_treble_boost(j);
		j += LoudnessTrebleStep;
	}
#endif

	return 0;
}

