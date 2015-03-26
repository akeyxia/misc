#include <stdio.h>
#include <math.h>

#include "frac2hex.c"

#define GraphicEqualizerMaxdb	12
#define ToneControlMaxdb	0

#define FS	44100
#define GeqBand1Fc	80
#define GeqBand2Fc	500
#define GeqBand3Fc	1000
#define GeqBand4Fc	5000
#define GeqBand5Fc	16000

static int Max_Of_Array(int *array, int size)
{
	int i;
	int max = array[0];

	for (i = 1; i < size; i++)
		if (max < array[i])
			max = array[i];

	return max;
}

static void DB2Val_5BandGeq(int band, float fc, int gain, float Q)
{
	float G, t, D;
	double a2d, a1d, b0d;
	float a2f, a1f, b0f;
	int gc, a2l, a2h, a1l, a1h, b0l, b0h;

	if (gain > GraphicEqualizerMaxdb || gain < -GraphicEqualizerMaxdb) {
		printf("%s: gain %d-%d is supported\n", __func__,
				-GraphicEqualizerMaxdb, GraphicEqualizerMaxdb);
		return;
	}

	if (Q < 0.1f || Q > 10.0f) {
		printf("%s: Qfactor 0.1-10.0 is supported\n", __func__);
		return;
	}

	G = (powf(10, gain / 20.0f) - 1) / 4;
	t = 2 * M_PI * fc / FS;
	if (gain >= 0)
		D = 1.0f;
	else
		D = 1 / (4 * G + 1);

	switch(band) {
	case 1:
	case 2:
	case 3:
		a2d = -0.5 * (1 - D * t / 2 / Q) / (1 + D * t / 2 / Q);
		a1d = (0.5 - a2d) * cos(t);
		b0d = (0.5 + a2d) / 2;

		gc = YMEM2HEX(G);
		b0h = DYMEM2HEX(b0d) >> 12;
		b0l = DYMEM2HEX(b0d) & 0xFFF;
		a1h = DYMEM2HEX(a1d) >> 12;
		a1l = DYMEM2HEX(a1d) & 0xFFF;
		a2h = DYMEM2HEX(a2d) >> 12;
		a2l = DYMEM2HEX(a2d) & 0xFFF;

		break;
	case 4:
	case 5:
		a2f = -0.5 * (1 - D * t / 2 / Q) / (1 + D * t / 2 / Q);
		a1f = (0.5 - a2f) * cos(t);
		b0f = (0.5 + a2f) / 2;

		gc = YMEM2HEX(G);
		b0h = 0;
		b0l = YMEM2HEX(b0f);
		a1h = 0;
		a1l = YMEM2HEX(a1f);
		a2h = 0;
		a2l = YMEM2HEX(a2f);

		break;
	default:
		break;
	}

	if (band <= 3)
		printf("0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x%03x, /* band %d %3d dB */\n", b0l, b0h,
				a2l, a2h, a1l, a1h, gc, band, gain);
	else
		printf("0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x000, 0x000, 0x000, /* band %d %3d dB */\n", b0l, a2l,
				a1l, gc, band, gain);
}

static void DB2Val_DesScaleGeq(int gain)
{
	float valf;
	int des_scale;

	/*
	 * Y:Vol_DesScalGEQ
	 */
	valf = powf(10.0f, -gain / 20.0f);
	des_scale = YMEM2HEX(valf);

	printf("0x%03x, /* %ddB */\n", des_scale, gain);
}

static void DB2Val_MaxBoost()
{
	double vald;
	int max_boost;

	/*
	 * X:Vol_OneOverMaxBoost
	 */
	vald = pow(10.0f, -(ToneControlMaxdb + GraphicEqualizerMaxdb) / 20.0f);
	max_boost = XMEM2HEX(vald);

	printf("0x%06x, /* max frequency response */\n", max_boost);
}

int main(int argc, char *argv[])
{
	int i, j;

	printf("GeqMaxResp:\n");
	DB2Val_MaxBoost();

	printf("GeqResp:\n");
	for (i = -12; i <= 12; i++)
		DB2Val_DesScaleGeq(i);

	printf("b0low, b0high, a1low, a1high, a2low, a2high, gc\n");
        printf("Band %d\n", 1);
        for (i = -12; i <= 12; i++) {
                DB2Val_5BandGeq(1, GeqBand1Fc, i, 1.0f);
        }
        printf("Band %d\n", 2);
        for (i = -12; i <= 12; i++) {
                DB2Val_5BandGeq(2, GeqBand2Fc, i, 1.0f);
        }
        printf("Band %d\n", 3);
        for (i = -12; i <= 12; i++) {
                DB2Val_5BandGeq(3, GeqBand3Fc, i, 1.0f);
        }
        printf("Band %d\n", 4);
        for (i = -12; i <= 12; i++) {
                DB2Val_5BandGeq(4, GeqBand4Fc, i, 1.0f);
        }
        printf("Band %d\n", 5);
        for (i = -12; i <= 12; i++) {
                DB2Val_5BandGeq(5, GeqBand5Fc, i, 1.0f);
        }

	printf("Pop\n");
	int pop[5] = {4, -2, 4, 2, 2};
	DB2Val_DesScaleGeq(Max_Of_Array(pop, 5));
	DB2Val_5BandGeq(1, GeqBand1Fc, pop[0], 1.0f);
	DB2Val_5BandGeq(2, GeqBand2Fc, pop[1], 1.0f);
	DB2Val_5BandGeq(3, GeqBand3Fc, pop[2], 1.0f);
	DB2Val_5BandGeq(4, GeqBand4Fc, pop[3], 1.0f);
	DB2Val_5BandGeq(5, GeqBand5Fc, pop[4], 1.0f);

	printf("Jazz\n");
	int jazz[5] = {2, 0, 4, 0, 2};
	DB2Val_DesScaleGeq(Max_Of_Array(jazz, 5));
	DB2Val_5BandGeq(1, GeqBand1Fc, jazz[0], 1.0f);
	DB2Val_5BandGeq(2, GeqBand2Fc, jazz[1], 1.0f);
	DB2Val_5BandGeq(3, GeqBand3Fc, jazz[2], 1.0f);
	DB2Val_5BandGeq(4, GeqBand4Fc, jazz[3], 1.0f);
	DB2Val_5BandGeq(5, GeqBand5Fc, jazz[4], 1.0f);

	printf("Classic\n");
	int classic[5] = {2, 2, 0, 3, 4};
	DB2Val_DesScaleGeq(Max_Of_Array(classic, 5));
	DB2Val_5BandGeq(1, GeqBand1Fc, classic[0], 1.0f);
	DB2Val_5BandGeq(2, GeqBand2Fc, classic[1], 1.0f);
	DB2Val_5BandGeq(3, GeqBand3Fc, classic[2], 1.0f);
	DB2Val_5BandGeq(4, GeqBand4Fc, classic[3], 1.0f);
	DB2Val_5BandGeq(5, GeqBand5Fc, classic[4], 1.0f);

	printf("Vocal\n");
	int vocal[5] = {-4, 2, 2, 2, -4};
	DB2Val_DesScaleGeq(Max_Of_Array(vocal, 5));
	DB2Val_5BandGeq(1, GeqBand1Fc, vocal[0], 1.0f);
	DB2Val_5BandGeq(2, GeqBand2Fc, vocal[1], 1.0f);
	DB2Val_5BandGeq(3, GeqBand3Fc, vocal[2], 1.0f);
	DB2Val_5BandGeq(4, GeqBand4Fc, vocal[3], 1.0f);
	DB2Val_5BandGeq(5, GeqBand5Fc, vocal[4], 1.0f);

	printf("Rock\n");
	int rock[5] = {6, 0, 2, 0, 6};
	DB2Val_DesScaleGeq(Max_Of_Array(rock, 5));
	DB2Val_5BandGeq(1, GeqBand1Fc, rock[0], 1.0f);
	DB2Val_5BandGeq(2, GeqBand2Fc, rock[1], 1.0f);
	DB2Val_5BandGeq(3, GeqBand3Fc, rock[2], 1.0f);
	DB2Val_5BandGeq(4, GeqBand4Fc, rock[3], 1.0f);
	DB2Val_5BandGeq(5, GeqBand5Fc, rock[4], 1.0f);

	return 0;
}
