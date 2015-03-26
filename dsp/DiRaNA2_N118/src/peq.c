#include <stdio.h>
#include <math.h>

#include "saf7741.h"

static int Max_Of_Array(int *array, int size)
{
	int i;
	int max = array[0];

	for (i = 1; i < size; i++)
		if (max < array[i])
			max = array[i];

	return max;
}

static void DB2Val_8BandPeq(int band, float fc, int gain, float Q)
{
	float G, t, D, Gmax, b;
	double a2d, a1d, b0d, b1d, b2d;
	int gc, a2l, a2h, a1l, a1h, b0l, b0h, b1l, b1h, b2l, b2h;

	if (gain > ParametricEqualizerMaxdb || gain < -ParametricEqualizerMaxdb) {
		printf("%s: gain %d-%d is supported\n", __func__,
				-ParametricEqualizerMaxdb, ParametricEqualizerMaxdb);
		return;
	}

	if (Q < 0.1f || Q > 10.0f) {
		printf("%s: Qfactor 0.1-10.0 is supported\n", __func__);
		return;
	}

	switch(band) {
	case 1:
	case 2:
	case 3:
        case 4:
                G = (powf(10, gain / 20.0f) - 1) / 4;
                t = 2 * M_PI * fc / FS;
                if (gain >= 0)
                        D = 1.0f;
                else
                        D = 1 / (4 * G + 1);
		a2d = -0.5 * (1 - D * t / 2 / Q) / (1 + D * t / 2 / Q);
		a1d = (0.5 - a2d) * cos(t);
		b0d = (0.5 + a2d) / 2;

		gc = YMEM2Hex(G);
		b0h = DYMEM2Hex(b0d) >> 12;
		b0l = DYMEM2Hex(b0d) & 0xFFF;
		a1h = DYMEM2Hex(a1d) >> 12;
		a1l = DYMEM2Hex(a1d) & 0xFFF;
		a2h = DYMEM2Hex(a2d) >> 12;
		a2l = DYMEM2Hex(a2d) & 0xFFF;

		break;
	case 5:
	case 6:
	case 7:
	case 8:
                Gmax = 20.0f * log10f(2.0f + Q * FS / M_PI / fc);
                G = powf(10, gain / 20.0f);
                t = 2* M_PI * fc / FS;
                if (G >= 1.0f)
                        b = t / (2 * Q);
                else
                        b = t / (2 * G * Q);

		a2d = -0.5 * (1 - b) / (1 + b);
		a1d = (0.5 - a2d) * cos(t);
		b0d = (G - 1) * (0.25 + 0.5 * a2d) + 0.5;
                b1d = -a1d;
                b2d = -(G - 1) * (0.25 + 0.5 * a2d) - a2d;

		gc = YMEM2Hex(G);
		b0h = DYMEM2Hex(b0d) >> 12;
		b0l = DYMEM2Hex(b0d) & 0xFFF;
                b1h = DYMEM2Hex(b1d) >> 12;
		b1l = DYMEM2Hex(b1d) & 0xFFF;
                b2h = DYMEM2Hex(b2d) >> 12;
		b2l = DYMEM2Hex(b2d) & 0xFFF;
		a1h = DYMEM2Hex(a1d) >> 12;
		a1l = DYMEM2Hex(a1d) & 0xFFF;
		a2h = DYMEM2Hex(a2d) >> 12;
		a2l = DYMEM2Hex(a2d) & 0xFFF;

		break;
	default:
		break;
	}

	if (band <= 4)
		printf("0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x000, 0x000, 0x000, /* band %d %ddB */\n",
                                b0l, b0h,
				a2l, a2h, a1l, a1h, gc, band, gain);
	else
		printf("0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x%03x, /* band %d %ddB max %.2fdB */\n",
                                b0l, b0h, b1l, b1h, b2l, b2h,
				a1l, a1h, a2l, a2h, band, gain, Gmax);
}

static void DB2Val_VolScale(int gain)
{
	float valf;
	int scale;

	/*
	 * Y:Vol_ScalF, Y:Vol_ScalR
	 */
	valf = powf(10.0f, -gain / 20.0f);
	scale = YMEM2Hex(valf);

	printf("0x%03x, /* %ddB */\n", scale, gain);
}

static void DB2Val_EqGx(int gain)
{
	float valf;
	int Gx;

        valf = powf(10.0f, gain / 20.0f);
        Gx = YMEM2Hex(valf);

	printf("0x%03x, /* %ddB */\n", Gx, gain);

}

int main(int argc, char *argv[])
{
	int i, j;

	printf("PeqResp:\n");
	for (i = -12; i <= 12; i++)
		DB2Val_VolScale(i);

        printf("Eq_Gx:\n");
        DB2Val_EqGx(0);

	printf("AP13 Flat(Default) FL/FR\n");
	int ap13_flat[8] = {-2, -3, -7, -3, 2, 3, 0, 0, };
	DB2Val_VolScale(Max_Of_Array(ap13_flat, 8));
	DB2Val_8BandPeq(1, 70.0f, ap13_flat[0], 2.0f);
	DB2Val_8BandPeq(2, 125.0f, ap13_flat[1], 1.0f);
	DB2Val_8BandPeq(3, 250.0f, ap13_flat[2], 3.0f);
	DB2Val_8BandPeq(4, 330.0f, ap13_flat[3], 2.0f);
	DB2Val_8BandPeq(5, 2500.0f, ap13_flat[4], 1.0f);
	DB2Val_8BandPeq(6, 12000.0f, ap13_flat[5], 1.0f);
	DB2Val_8BandPeq(7, 100.0f, ap13_flat[6], 1.0f);
	DB2Val_8BandPeq(8, 100.0f, ap13_flat[7], 1.0f);

	printf("AP13 Flat(Default) RL/RR\n");
	int ap13_flat_rear[8] = {2, 0, -5, 2, 1, 0, 0, 0, };
	DB2Val_VolScale(Max_Of_Array(ap13_flat_rear, 8));
	DB2Val_8BandPeq(1, 60.0f, ap13_flat_rear[0], 2.0f);
	DB2Val_8BandPeq(2, 100.0f, ap13_flat_rear[1], 2.0f);
	DB2Val_8BandPeq(3, 330.0f, ap13_flat_rear[2], 2.0f);
	DB2Val_8BandPeq(4, 700.0f, ap13_flat_rear[3], 2.0f);
	DB2Val_8BandPeq(5, 1000.0f, ap13_flat_rear[4], 1.0f);
	DB2Val_8BandPeq(6, 800.0f, ap13_flat_rear[5], 1.0f);
	DB2Val_8BandPeq(7, 100.0f, ap13_flat_rear[6], 1.0f);
	DB2Val_8BandPeq(8, 100.0f, ap13_flat_rear[7], 1.0f);

	printf("AS21 Flat(Default) FL/FR\n");
	int as21_flat[8] = {-3, -5, 2, 0, 0, 0, 0, 0, };
	DB2Val_VolScale(Max_Of_Array(as21_flat, 8));
	DB2Val_8BandPeq(1, 125.0f, as21_flat[0], 1.5f);
	DB2Val_8BandPeq(2, 330.0f, as21_flat[1], 2.0f);
	DB2Val_8BandPeq(3, 700.0f, as21_flat[2], 2.0f);
	DB2Val_8BandPeq(4, 1000.0f, as21_flat[3], 1.0f);
	DB2Val_8BandPeq(5, 800.0f, as21_flat[4], 1.0f);
	DB2Val_8BandPeq(6, 100.0f, as21_flat[5], 1.0f);
	DB2Val_8BandPeq(7, 100.0f, as21_flat[6], 1.0f);
	DB2Val_8BandPeq(8, 100.0f, as21_flat[7], 1.0f);

	printf("AS21 Flat(Default) RL/RR\n");
	int as21_flat_rear[8] = {2, -3, -5, 2, 0, 0, 0, 0, };
	DB2Val_VolScale(Max_Of_Array(as21_flat_rear, 8));
	DB2Val_8BandPeq(1, 60.0f, as21_flat_rear[0], 2.0f);
	DB2Val_8BandPeq(2, 125.0f, as21_flat_rear[1], 1.5f);
	DB2Val_8BandPeq(3, 330.0f, as21_flat_rear[2], 2.0f);
	DB2Val_8BandPeq(4, 700.0f, as21_flat_rear[3], 2.0f);
	DB2Val_8BandPeq(5, 1000.0f, as21_flat_rear[4], 1.0f);
	DB2Val_8BandPeq(6, 800.0f, as21_flat_rear[5], 1.0f);
	DB2Val_8BandPeq(7, 100.0f, as21_flat_rear[6], 1.0f);
	DB2Val_8BandPeq(8, 100.0f, as21_flat_rear[7], 1.0f);

	return 0;
}
