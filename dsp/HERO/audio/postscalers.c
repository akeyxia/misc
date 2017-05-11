#include <stdio.h>
#include <math.h>
#include "frac2hex.c"
#include "hex2frac.c"

//#define FixedBoost	48.16
#define FixedBoost	60.0

/*
* Input: n=<F, R, SwL, SwR>, m=<L, R>
* Output coefficient: Y:Vol_UpScal<n>, Y:Vol_ChanGainPF<m>
*/
void PostScales(float comp)
{
	float CompN = comp;

	/* Comp<n>=MainVolMax + GeqMaxResp + PeqMaxResp<n> + ToneBoostMaxP + ChanGainPMax */

	printf("CompN = %.2f, 0x%03X, /* %6.2f dB */\n",
		CompN, YMEM2HEX(pow10f((CompN - FixedBoost)/20.0)), FixedBoost-CompN);

}

void PostScales_Range(float min, float max, float step)
{
	float value;
	float CompN;

	printf("%s: \n", __func__);

	value = min;
	while (value <= max) {
		CompN = value;
		PostScales(CompN);

		value += step;
	}
}

void PostScales_Frac(int comphex)
{
	float CompN, CompFrac;

	CompFrac = YHEX2Frac(comphex);
	CompN = 20.0 * log10f(CompFrac) + FixedBoost;

	printf("CompHex = 0x%X, CompN = %6.2f\n", comphex, CompN);
	
}

int main(int argc, char *argv[])
{
	if (argc == 2)
		PostScales_Frac(strtol(argv[1], NULL, 16));
	else
		PostScales_Range(0.0, 48.0, 1.0);

	return 0;
}

