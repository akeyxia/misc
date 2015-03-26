#include <stdio.h>
#include <math.h>

#include "frac2hex.c"

#define FS	44100
#define ParametricEqualizerMaxdb	12

static void DB2Val_7BandPeq(int band ,int fc, float gain, float Q)
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
                t = 2.0f * M_PI * fc / FS;
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
		printf("0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x000, 0x000, 0x000, /* band %d(%d) %.1fdB %.1f*/\n",
                                b0l, b0h,
				a2l, a2h, a1l, a1h, gc, band, fc, gain, Q);
	else
		printf("0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, /* band %d(%d) %.1fdB max %.2fdB %.1f*/\n",
                                b0l, b0h, b1l, b1h, b2l, b2h,
				a1l, a1h, a2l, a2h, band, fc, gain, Gmax, Q);
}

int main(int argc, char *argv[])
{
	printf("A1206C GEQ Default FL/FR\n");
	float peq_default_front[7] = {0, 0, -4, 3, 3, 0, 0};
	DB2Val_7BandPeq(1, 60, peq_default_front[0], 1.0f);
	DB2Val_7BandPeq(2, 100, peq_default_front[1], 1.0f);
	DB2Val_7BandPeq(3, 200, peq_default_front[2], 2.0f);
	DB2Val_7BandPeq(4, 500, peq_default_front[3], 2.0f);
	DB2Val_7BandPeq(5, 1000, peq_default_front[4], 1.0f);
	DB2Val_7BandPeq(6, 3200, peq_default_front[5], 1.0f);
	DB2Val_7BandPeq(7, 6400, peq_default_front[6], 1.0f);

	printf("A1206C GEQ Default RL/RR\n");
	float peq_default_rear[7] = {2, 0, -4, 3, 3, 0, 0};
	DB2Val_7BandPeq(1, 60, peq_default_rear[0], 2.0f);
	DB2Val_7BandPeq(2, 100, peq_default_rear[1], 1.0f);
	DB2Val_7BandPeq(3, 200, peq_default_rear[2], 2.0f);
	DB2Val_7BandPeq(4, 500, peq_default_rear[3], 2.0f);
	DB2Val_7BandPeq(5, 1000, peq_default_rear[4], 1.0f);
	DB2Val_7BandPeq(6, 3200, peq_default_rear[5], 1.0f);
	DB2Val_7BandPeq(7, 6400, peq_default_rear[6], 1.0f);

	printf("A1206C GEQ Flat\n");
	float peq_flat_all[7] = {0, 0, 0, 0, 0, 0, 0};
	DB2Val_7BandPeq(1, 60, peq_flat_all[0], 1.0f);
	DB2Val_7BandPeq(2, 100, peq_flat_all[1], 1.0f);
	DB2Val_7BandPeq(3, 200, peq_flat_all[2], 1.0f);
	DB2Val_7BandPeq(4, 500, peq_flat_all[3], 1.0f);
	DB2Val_7BandPeq(5, 1000, peq_flat_all[4], 1.0f);
	DB2Val_7BandPeq(6, 3200, peq_flat_all[5], 1.0f);
	DB2Val_7BandPeq(7, 6400, peq_flat_all[6], 1.0f);

	return 0;
}
