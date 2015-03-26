#include <stdio.h>
#include <math.h>

#include "saf7741.h"

/*
 * Translate the dB gain to YMEM value for fader and balance
 */
void second_order_lpf(float fc, int gain)
{
        double g, t0, t1;
	double a1d, a2d, b0d, b1d, b2d;
	int a1h, a1l, a2h, a2l, b0h, b0l, b1h, b1l, b2h, b2l;

        g = pow(10.0, gain / 20.0);
        t0 = tan(M_PI * fc / FS);
        t1 = 1.0 + sqrt(2.0) * t0 + pow(t0, 2.0);
        a1d = (1.0 - pow(t0, 2)) / t1;
        a2d = 1.0 / 2.0 * (sqrt(2.0) * t0 - 1.0 - pow(t0, 2.0)) / t1;
        b0d = g * pow(t0, 2.0) / (2.0 * t1);
        b1d = 2.0 * b0d;
        b2d = b0d;

        a1h = DYMEM2Hex(a1d) >> 12;
        a1l = DYMEM2Hex(a1d) & 0xFFF;
        a2h = DYMEM2Hex(a2d) >> 12;
        a2l = DYMEM2Hex(a2d) & 0xFFF;
        b0h = DYMEM2Hex(b0d) >> 12;
        b0l = DYMEM2Hex(b0d) & 0xFFF;
        b1h = DYMEM2Hex(b1d) >> 12;
        b1l = DYMEM2Hex(b1d) & 0xFFF;
        b2h = DYMEM2Hex(b2d) >> 12;
        b2l = DYMEM2Hex(b2d) & 0xFFF;

        printf("0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x%03x, "
                        "0x%03x, /* 2nd order LPF with %.1fHz cut off frequency %ddB gain */\n",
                        a1h, a1l, a2h, a2l, b0h, b0l, b1h, b1l, b2h, b2l, fc, gain);
}

int main(int argc, char *argv[])
{
        second_order_lpf(4000.0f, -12);

	return 0;
}
