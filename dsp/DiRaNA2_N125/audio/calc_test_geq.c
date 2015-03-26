#include <stdio.h>
#include <math.h>

#include "frac2hex.c"

#define FS	44100
#define ParametricEqualizerMaxdb	12


static int Max_Of_Array(int *array, int size)
{
	int i;
	int max = array[0];

	for (i = 1; i < size; i++)
		if (max < array[i])
			max = array[i];

	return max;
}

static void DB2Val_7BandPeq(int band, float fc, int gain, float Q)
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

		gc = YMEM2HEX(G);
		b0h = DYMEM2HEX(b0d) >> 12;
		b0l = DYMEM2HEX(b0d) & 0xFFF;
		a1h = DYMEM2HEX(a1d) >> 12;
		a1l = DYMEM2HEX(a1d) & 0xFFF;
		a2h = DYMEM2HEX(a2d) >> 12;
		a2l = DYMEM2HEX(a2d) & 0xFFF;

		break;
	case 5:
	case 6:
	case 7:
                Gmax = 20.0f * log10f(2.0f + Q * FS / M_PI / fc);
                G = powf(10, gain / 20.0f);
                t = 2.0f * M_PI * fc / FS;
                if (G >= 1.0f)
                        b = t / (2 * Q);
                else
                        b = t / (2 * G * Q);

		a2d = -0.5 * (1 - b) / (1 + b);
		a1d = (0.5 - a2d) * cos(t);
		b0d = (G - 1) * (0.25 + 0.5 * a2d) + 0.5;
                b1d = -a1d;
                b2d = -(G - 1) * (0.25 + 0.5 * a2d) - a2d;

		gc = YMEM2HEX(G);
		b0h = DYMEM2HEX(b0d) >> 12;
		b0l = DYMEM2HEX(b0d) & 0xFFF;
                b1h = DYMEM2HEX(b1d) >> 12;
		b1l = DYMEM2HEX(b1d) & 0xFFF;
                b2h = DYMEM2HEX(b2d) >> 12;
		b2l = DYMEM2HEX(b2d) & 0xFFF;
		a1h = DYMEM2HEX(a1d) >> 12;
		a1l = DYMEM2HEX(a1d) & 0xFFF;
		a2h = DYMEM2HEX(a2d) >> 12;
		a2l = DYMEM2HEX(a2d) & 0xFFF;

		break;
	default:
		break;
	}

	if (band <= 4)
		printf("0x1004, %d, %d, %d, %4.2f, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x000, 0x000, 0x000, /* band %d(%.0f) %ddB */\n",
                                band, (int)fc, gain, Q, b0l, b0h,
				a2l, a2h, a1l, a1h, gc, band, fc, gain);
	else
		printf("0x1004, %d, %d, %d, %4.2f, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, /* band %d(%.0f) %ddB max %.2fdB */\n",
                                band, (int)fc, gain, Q, b0l, b0h, b1l, b1h, b2l, b2h,
				a1l, a1h, a2l, a2h, band, fc, gain, Gmax);
}

static void DB2Val_VolScale(int gain)
{
	float valf;
	int scale;

	/*
	 * Y:Vol_ScalF, Y:Vol_ScalR
	 */
	valf = powf(10.0f, -gain / 20.0f);
	scale = YMEM2HEX(valf);

	printf("0x%03X, /* %ddB */\n", scale, gain);
}

static void DB2Val_EqGx(int gain)
{
	float valf;
	int Gx;

        valf = powf(10.0f, gain / 20.0f);
        Gx = YMEM2HEX(valf);

	printf("0x%03X, /* %ddB */\n", Gx, gain);

}

int main(int argc, char *argv[])
{
	int i, j;

	printf("PeqResp:\n");
	for (i = -12; i <= 12; i++)
		DB2Val_VolScale(i);

        printf("Eq_Gx:\n");
        DB2Val_EqGx(0);

	printf("A1206C GEQ Default FL/FR\n");
	//int peq_default_front[7] = {0, -4, 3, 3, 0, 0, 0};
	int peq_default_front[7] = {0, 12, 0, 0, 0, 0, 0};
	DB2Val_VolScale(Max_Of_Array(peq_default_front, 7));
	DB2Val_7BandPeq(1, 60.0f, peq_default_front[0], 1.0f);
	DB2Val_7BandPeq(2, 100.0f, peq_default_front[1], 1.0f);
	DB2Val_7BandPeq(3, 200.0f, peq_default_front[2], 1.0f);
	DB2Val_7BandPeq(4, 500.0f, peq_default_front[3], 1.0f);
	DB2Val_7BandPeq(5, 1000.0f, peq_default_front[4], 1.0f);
	DB2Val_7BandPeq(6, 5000.0f, peq_default_front[5], 1.0f);
	DB2Val_7BandPeq(7, 5000.0f, peq_default_front[6], 1.0f);

	printf("A1206C GEQ Default RL/RR\n");
	//int peq_default_rear[7] = {2, -4, 3, 3, 0, 0, 0};
	int peq_default_rear[7] = {0, -12, 0, 0, 0, 0, 0};
	DB2Val_VolScale(Max_Of_Array(peq_default_rear, 7));
	DB2Val_7BandPeq(1, 60.0f, peq_default_rear[0], 1.0f);
	DB2Val_7BandPeq(2, 100.0f, peq_default_rear[1], 1.0f);
	DB2Val_7BandPeq(3, 200.0f, peq_default_rear[2], 1.0f);
	DB2Val_7BandPeq(4, 500.0f, peq_default_rear[3], 1.0f);
	DB2Val_7BandPeq(5, 1000.0f, peq_default_rear[4], 1.0f);
	DB2Val_7BandPeq(6, 5000.0f, peq_default_rear[5], 1.0f);
	DB2Val_7BandPeq(7, 5000.0f, peq_default_rear[6], 1.0f);

	printf("A1206C GEQ Flat\n");
	int peq_flat_all[7] = {0, 0, 0, 0, 0, 0, 0};
	DB2Val_VolScale(Max_Of_Array(peq_flat_all, 7));
	DB2Val_7BandPeq(1, 60.0f, peq_flat_all[0], 1.0f);
	DB2Val_7BandPeq(2, 100.0f, peq_flat_all[1], 1.0f);
	DB2Val_7BandPeq(3, 200.0f, peq_flat_all[2], 1.0f);
	DB2Val_7BandPeq(4, 500.0f, peq_flat_all[3], 1.0f);
	DB2Val_7BandPeq(5, 1000.0f, peq_flat_all[4], 1.0f);
	DB2Val_7BandPeq(6, 5000.0f, peq_flat_all[5], 1.0f);
	DB2Val_7BandPeq(7, 5000.0f, peq_flat_all[6], 1.0f);

	return 0;
}
